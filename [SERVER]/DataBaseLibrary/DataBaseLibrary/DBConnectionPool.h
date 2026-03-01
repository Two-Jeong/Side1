#pragma once
#include <memory>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <thread>
#include <chrono>
#include <unordered_set>
#include <future>
#include "DBConnection.h"
#include "DBConfig.h"

namespace DB {

class DBConnectionPool {
public:
    explicit DBConnectionPool(const DBConfig& config);
    ~DBConnectionPool();
    
    // Pool management
    void initialize();
    void shutdown();
    
    // Connection acquisition/release
    class PooledConnection {
    public:
        PooledConnection(std::unique_ptr<DBConnection> connection, DBConnectionPool* pool);
        ~PooledConnection();
        
        // Move-only semantics
        PooledConnection(const PooledConnection&) = delete;
        PooledConnection& operator=(const PooledConnection&) = delete;
        PooledConnection(PooledConnection&& other) noexcept;
        PooledConnection& operator=(PooledConnection&& other) noexcept;
        
        DBConnection* get() const { return connection_.get(); }
        DBConnection* operator->() const { return connection_.get(); }
        DBConnection& operator*() const { return *connection_; }
        
        void release();
        
    private:
        std::unique_ptr<DBConnection> connection_;
        DBConnectionPool* pool_;
        bool released_;
    };
    
    std::unique_ptr<PooledConnection> acquire_connection(
        std::chrono::milliseconds timeout = std::chrono::milliseconds(5000));
    
    // Pool statistics
    struct Statistics {
        size_t total_connections;
        size_t active_connections;
        size_t idle_connections;
        size_t pending_requests;
        uint64_t total_acquired;
        uint64_t total_created;
        uint64_t total_destroyed;
        std::chrono::system_clock::time_point last_cleanup;
    };
    
    Statistics get_statistics() const;
    
    // Health check
    bool is_healthy() const;
    void validate_connections();
    
private:
    DBConfig config_;
    std::atomic<bool> initialized_;
    std::atomic<bool> shutdown_requested_;
    
    mutable std::mutex pool_mutex_;
    std::condition_variable pool_condition_;
    
    std::queue<std::unique_ptr<DBConnection>> idle_connections_;
    std::unordered_set<DBConnection*> active_connections_;
    std::atomic<size_t> total_connections_;
    std::atomic<size_t> pending_requests_;
    
    // Statistics
    std::atomic<uint64_t> total_acquired_;
    std::atomic<uint64_t> total_created_;
    std::atomic<uint64_t> total_destroyed_;
    
    // Cleanup thread
    std::thread cleanup_thread_;
    std::atomic<bool> cleanup_thread_running_;
    
    // Helper methods
    std::unique_ptr<DBConnection> create_connection();
    void return_connection(std::unique_ptr<DBConnection> connection);
    void cleanup_idle_connections();
    void cleanup_thread_worker();
    bool is_connection_valid(DBConnection* connection);
    void ensure_minimum_connections();
    
    friend class PooledConnection;
};

// RAII helper for database operations
template<typename Func>
auto execute_with_connection(DBConnectionPool& pool, Func func) -> decltype(func(std::declval<DBConnection*>())) {
    auto connection = pool.acquire_connection();
    if (!connection) {
        throw ConnectionException("Failed to acquire connection from pool");
    }
    
    try {
        return func(connection->get());
    } catch (const DBException& e) {
        // Check if connection is still valid after error
        if (!connection->get()->is_valid()) {
            connection->get()->reset();
        }
        throw;
    }
}

// Simple async helper - remove for now to avoid template issues

}  // namespace DB