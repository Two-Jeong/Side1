#include "DatabaseManager.h"
#include "DBConnectionPool.h"
#include <sstream>

namespace DB {

DatabaseManager::DatabaseManager(const DBConfig& config)
    : config_(config), initialized_(false) {
    pool_ = std::make_unique<DBConnectionPool>(config_);
}

DatabaseManager::~DatabaseManager() {
    shutdown();
}

void DatabaseManager::initialize() {
    if (initialized_.load()) {
        return;
    }
    
    try {
        pool_->initialize();
        initialized_.store(true);
    } catch (const DBException& e) {
        throw ConnectionException("Failed to initialize database manager: " + std::string(e.what()));
    }
}

void DatabaseManager::shutdown() {
    if (initialized_.load()) {
        pool_->shutdown();
        initialized_.store(false);
    }
}

QueryResult DatabaseManager::execute_query(const std::string& query) {
    ensure_initialized();
    
    return execute_with_connection(*pool_, [&](DBConnection* conn) {
        return conn->execute_query(query);
    });
}

uint64_t DatabaseManager::execute_update(const std::string& query) {
    ensure_initialized();
    
    return execute_with_connection(*pool_, [&](DBConnection* conn) {
        return conn->execute_update(query);
    });
}

uint64_t DatabaseManager::execute_insert(const std::string& query) {
    ensure_initialized();
    
    return execute_with_connection(*pool_, [&](DBConnection* conn) {
        return conn->execute_insert(query);
    });
}

void DatabaseManager::execute(const std::string& query) {
    ensure_initialized();
    
    execute_with_connection(*pool_, [&](DBConnection* conn) {
        conn->execute(query);
        return 0;  // Dummy return for lambda compatibility
    });
}

bool DatabaseManager::table_exists(const std::string& table_name) {
    ensure_initialized();
    
    std::string query = "SELECT COUNT(*) as count FROM information_schema.tables "
                       "WHERE table_schema = '" + config_.database + "' "
                       "AND table_name = '" + table_name + "'";
    
    try {
        auto result = execute_query(query);
        auto row = result.fetch_one();
        if (row) {
            return row->get<int>("count") > 0;
        }
    } catch (const DBException&) {
        // If query fails, assume table doesn't exist
        return false;
    }
    
    return false;
}

std::vector<std::string> DatabaseManager::get_table_list() {
    ensure_initialized();
    
    std::string query = "SELECT table_name FROM information_schema.tables "
                       "WHERE table_schema = '" + config_.database + "' "
                       "ORDER BY table_name";
    
    std::vector<std::string> tables;
    
    try {
        auto result = execute_query(query);
        for (const auto& row : result) {
            tables.push_back(row.get<std::string>("table_name"));
        }
    } catch (const DBException& e) {
        throw QueryException("Failed to get table list: " + std::string(e.what()));
    }
    
    return tables;
}

void DatabaseManager::create_database_if_not_exists(const std::string& database_name) {
    ensure_initialized();
    
    // Connect without specifying database
    DBConfig temp_config = config_;
    temp_config.database = "";
    
    DBConnection temp_conn(temp_config);
    temp_conn.connect();
    
    std::string query = "CREATE DATABASE IF NOT EXISTS `" + database_name + "` "
                       "CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci";
    
    try {
        temp_conn.execute(query);
    } catch (const DBException& e) {
        throw QueryException("Failed to create database '" + database_name + "': " + std::string(e.what()));
    }
}

DBConnectionPool::Statistics DatabaseManager::get_pool_statistics() const {
    ensure_initialized();
    return pool_->get_statistics();
}

bool DatabaseManager::is_pool_healthy() const {
    if (!initialized_.load()) {
        return false;
    }
    return pool_->is_healthy();
}

void DatabaseManager::validate_pool() const {
    ensure_initialized();
    pool_->validate_connections();
}

void DatabaseManager::ensure_initialized() const {
    if (!initialized_.load()) {
        throw ConnectionException("Database manager is not initialized");
    }
}

}  // namespace DB