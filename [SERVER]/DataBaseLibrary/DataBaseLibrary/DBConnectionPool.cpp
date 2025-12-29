#include "DBConnectionPool.h"
#include <algorithm>
#include <thread>

namespace DB {

DBConnectionPool::DBConnectionPool(const DBConfig& config)
    : config_(config), initialized_(false), shutdown_requested_(false),
      total_connections_(0), pending_requests_(0),
      total_acquired_(0), total_created_(0), total_destroyed_(0),
      cleanup_thread_running_(false) {
}

DBConnectionPool::~DBConnectionPool() {
    shutdown();
}

void DBConnectionPool::initialize() {
    std::lock_guard<std::mutex> lock(pool_mutex_);
    
    if (initialized_.load()) {
        return;
    }
    
    if (!config_.is_valid()) {
        throw ConnectionException("Invalid database configuration for connection pool");
    }
    
    // Create minimum connections
    for (size_t i = 0; i < config_.pool_min_size; ++i) {
        try {
            auto connection = create_connection();
            idle_connections_.push(std::move(connection));
            total_connections_.fetch_add(1);
        } catch (const DBException& e) {
            // Clean up any connections created so far
            while (!idle_connections_.empty()) {
                idle_connections_.pop();
                total_connections_.fetch_sub(1);
            }
            throw ConnectionException("Failed to initialize connection pool: " + std::string(e.what()));
        }
    }
    
    // Start cleanup thread
    cleanup_thread_running_.store(true);
    cleanup_thread_ = std::thread(&DBConnectionPool::cleanup_thread_worker, this);
    
    initialized_.store(true);
}

void DBConnectionPool::shutdown() {
    if (!initialized_.load()) {
        return;
    }
    
    shutdown_requested_.store(true);
    
    // Stop cleanup thread
    if (cleanup_thread_running_.load()) {
        cleanup_thread_running_.store(false);
        pool_condition_.notify_all();
        
        if (cleanup_thread_.joinable()) {
            cleanup_thread_.join();
        }
    }
    
    std::lock_guard<std::mutex> lock(pool_mutex_);
    
    // Close all idle connections
    while (!idle_connections_.empty()) {
        idle_connections_.pop();
        total_connections_.fetch_sub(1);
        total_destroyed_.fetch_add(1);
    }
    
    // Note: Active connections will be cleaned up when returned to the pool
    
    initialized_.store(false);
}

std::unique_ptr<DBConnectionPool::PooledConnection> DBConnectionPool::acquire_connection(std::chrono::milliseconds timeout) {
    if (!initialized_.load() || shutdown_requested_.load()) {
        throw ConnectionException("Connection pool is not available");
    }
    
    std::unique_lock<std::mutex> lock(pool_mutex_);
    pending_requests_.fetch_add(1);
    
    auto cleanup_pending = [this]() {
        pending_requests_.fetch_sub(1);
    };
    
    std::unique_ptr<DBConnection> connection;
    
    // Wait for available connection or timeout
    auto deadline = std::chrono::steady_clock::now() + timeout;
    
    while (idle_connections_.empty() && 
           total_connections_.load() >= config_.pool_max_size &&
           !shutdown_requested_.load()) {
        
        if (pool_condition_.wait_until(lock, deadline) == std::cv_status::timeout) {
            cleanup_pending();
            throw ConnectionException("Timeout waiting for database connection");
        }
    }
    
    if (shutdown_requested_.load()) {
        cleanup_pending();
        throw ConnectionException("Connection pool is shutting down");
    }
    
    // Try to get an idle connection
    if (!idle_connections_.empty()) {
        connection = std::move(idle_connections_.front());
        idle_connections_.pop();
        
        // Validate connection
        if (!is_connection_valid(connection.get())) {
            total_connections_.fetch_sub(1);
            total_destroyed_.fetch_add(1);
            connection.reset();
        }
    }
    
    // Create new connection if needed and allowed
    if (!connection && total_connections_.load() < config_.pool_max_size) {
        try {
            connection = create_connection();
            total_connections_.fetch_add(1);
        } catch (const DBException& e) {
            cleanup_pending();
            throw ConnectionException("Failed to create new connection: " + std::string(e.what()));
        }
    }
    
    if (!connection) {
        cleanup_pending();
        throw ConnectionException("Unable to acquire database connection");
    }
    
    // Track as active connection
    active_connections_.insert(connection.get());
    
    cleanup_pending();
    total_acquired_.fetch_add(1);
    
    return std::make_unique<PooledConnection>(std::move(connection), this);
}

DBConnectionPool::Statistics DBConnectionPool::get_statistics() const {
    std::lock_guard<std::mutex> lock(pool_mutex_);
    
    Statistics stats;
    stats.total_connections = total_connections_.load();
    stats.active_connections = active_connections_.size();
    stats.idle_connections = idle_connections_.size();
    stats.pending_requests = pending_requests_.load();
    stats.total_acquired = total_acquired_.load();
    stats.total_created = total_created_.load();
    stats.total_destroyed = total_destroyed_.load();
    stats.last_cleanup = std::chrono::system_clock::now();
    
    return stats;
}

bool DBConnectionPool::is_healthy() const {
    if (!initialized_.load() || shutdown_requested_.load()) {
        return false;
    }
    
    auto stats = get_statistics();
    
    // Check if we have minimum connections available
    if (stats.total_connections < config_.pool_min_size) {
        return false;
    }
    
    // Check for excessive pending requests
    if (stats.pending_requests > config_.pool_max_size) {
        return false;
    }
    
    return true;
}

void DBConnectionPool::validate_connections() {
    std::lock_guard<std::mutex> lock(pool_mutex_);
    cleanup_idle_connections();
    ensure_minimum_connections();
}

std::unique_ptr<DBConnection> DBConnectionPool::create_connection() {
    auto connection = std::make_unique<DBConnection>(config_);
    connection->connect();
    total_created_.fetch_add(1);
    return connection;
}

void DBConnectionPool::return_connection(std::unique_ptr<DBConnection> connection) {
    if (!connection || shutdown_requested_.load()) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(pool_mutex_);
    
    // Remove from active connections
    active_connections_.erase(connection.get());
    
    // Validate connection before returning to pool
    if (is_connection_valid(connection.get()) && 
        idle_connections_.size() < config_.pool_max_size) {
        idle_connections_.push(std::move(connection));
    } else {
        // Connection is invalid or pool is full, destroy it
        total_connections_.fetch_sub(1);
        total_destroyed_.fetch_add(1);
    }
    
    pool_condition_.notify_one();
}

void DBConnectionPool::cleanup_idle_connections() {
    auto now = std::chrono::system_clock::now();
    std::queue<std::unique_ptr<DBConnection>> valid_connections;
    
    while (!idle_connections_.empty()) {
        auto connection = std::move(idle_connections_.front());
        idle_connections_.pop();
        
        // Check if connection has been idle too long
        auto idle_time = now - connection->get_last_activity_time();
        if (idle_time > config_.pool_idle_timeout) {
            total_connections_.fetch_sub(1);
            total_destroyed_.fetch_add(1);
            continue;
        }
        
        // Validate connection
        if (is_connection_valid(connection.get())) {
            valid_connections.push(std::move(connection));
        } else {
            total_connections_.fetch_sub(1);
            total_destroyed_.fetch_add(1);
        }
    }
    
    idle_connections_ = std::move(valid_connections);
}

void DBConnectionPool::cleanup_thread_worker() {
    while (cleanup_thread_running_.load()) {
        std::unique_lock<std::mutex> lock(pool_mutex_);
        
        // Wait for cleanup interval or shutdown
        pool_condition_.wait_for(lock, config_.pool_validation_interval, 
            [this] { return !cleanup_thread_running_.load(); });
        
        if (!cleanup_thread_running_.load()) {
            break;
        }
        
        // Perform cleanup
        cleanup_idle_connections();
        ensure_minimum_connections();
        
        lock.unlock();
        
        // Brief pause to avoid consuming too much CPU
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

bool DBConnectionPool::is_connection_valid(DBConnection* connection) {
    if (!connection) {
        return false;
    }
    
    try {
        return connection->is_valid();
    } catch (...) {
        return false;
    }
}

void DBConnectionPool::ensure_minimum_connections() {
    while (idle_connections_.size() + active_connections_.size() < config_.pool_min_size &&
           total_connections_.load() < config_.pool_max_size &&
           !shutdown_requested_.load()) {
        try {
            auto connection = create_connection();
            idle_connections_.push(std::move(connection));
            total_connections_.fetch_add(1);
        } catch (...) {
            // Failed to create connection, break to avoid infinite loop
            break;
        }
    }
}

// PooledConnection implementation
DBConnectionPool::PooledConnection::PooledConnection(std::unique_ptr<DBConnection> connection, DBConnectionPool* pool)
    : connection_(std::move(connection)), pool_(pool), released_(false) {
}

DBConnectionPool::PooledConnection::~PooledConnection() {
    if (!released_) {
        release();
    }
}

DBConnectionPool::PooledConnection::PooledConnection(PooledConnection&& other) noexcept
    : connection_(std::move(other.connection_)), pool_(other.pool_), released_(other.released_) {
    other.pool_ = nullptr;
    other.released_ = true;
}

DBConnectionPool::PooledConnection& DBConnectionPool::PooledConnection::operator=(PooledConnection&& other) noexcept {
    if (this != &other) {
        if (!released_) {
            release();
        }
        
        connection_ = std::move(other.connection_);
        pool_ = other.pool_;
        released_ = other.released_;
        
        other.pool_ = nullptr;
        other.released_ = true;
    }
    return *this;
}

void DBConnectionPool::PooledConnection::release() {
    if (!released_ && pool_ && connection_) {
        pool_->return_connection(std::move(connection_));
        released_ = true;
    }
}

}  // namespace DB