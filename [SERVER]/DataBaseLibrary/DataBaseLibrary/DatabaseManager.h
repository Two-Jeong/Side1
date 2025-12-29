#pragma once
#include "DBConnectionPool.h"
#include "DBConnection.h"
#include <memory>
#include <functional>
#include <future>

namespace DB {

/**
 * @brief High-level database manager that provides easy-to-use interface
 * 
 * This class combines connection pooling, error handling, and convenient
 * methods for common database operations.
 */
class DatabaseManager {
public:
    explicit DatabaseManager(const DBConfig& config);
    ~DatabaseManager();
    
    // Initialize and shutdown
    void initialize();
    void shutdown();
    bool is_initialized() const { return initialized_; }
    
    // Simple query execution
    QueryResult execute_query(const std::string& query);
    uint64_t execute_update(const std::string& query);
    uint64_t execute_insert(const std::string& query);
    void execute(const std::string& query);
    
    // Transaction support
    template<typename Func>
    auto execute_transaction(Func func) -> decltype(func(std::declval<DBConnection*>()));
    
    // Prepared statement helpers
    template<typename Func>
    auto execute_prepared(const std::string& query, Func func) -> decltype(func(std::declval<DBConnection::PreparedStatement*>()));
    
    // Async operations (disabled for now to avoid template issues)
    // template<typename Func>
    // auto execute_async(Func func) -> std::future<decltype(func(std::declval<DBConnection*>()))>;
    
    // Utility methods
    bool table_exists(const std::string& table_name);
    std::vector<std::string> get_table_list();
    void create_database_if_not_exists(const std::string& database_name);
    
    // Pool management
    DBConnectionPool::Statistics get_pool_statistics() const;
    bool is_pool_healthy() const;
    void validate_pool() const;
    
    // Configuration
    const DBConfig& get_config() const { return config_; }
    
private:
    DBConfig config_;
    std::unique_ptr<DBConnectionPool> pool_;
    std::atomic<bool> initialized_;
    
    void ensure_initialized() const;
};

// Template implementations
template<typename Func>
auto DatabaseManager::execute_transaction(Func func) -> decltype(func(std::declval<DBConnection*>())) {
    ensure_initialized();
    
    auto connection = pool_->acquire_connection();
    if (!connection) {
        throw ConnectionException("Failed to acquire connection for transaction");
    }
    
    auto transaction = connection->get()->begin_transaction();
    
    try {
        auto result = func(connection->get());
        transaction->commit();
        return result;
    } catch (...) {
        if (transaction->is_active()) {
            transaction->rollback();
        }
        throw;
    }
}

template<typename Func>
auto DatabaseManager::execute_prepared(const std::string& query, Func func) -> decltype(func(std::declval<DBConnection::PreparedStatement*>())) {
    ensure_initialized();
    
    auto connection = pool_->acquire_connection();
    if (!connection) {
        throw ConnectionException("Failed to acquire connection for prepared statement");
    }
    
    auto stmt = connection->get()->prepare(query);
    return func(stmt.get());
}

// Async implementation disabled for now
/*
template<typename Func>
auto DatabaseManager::execute_async(Func func) -> std::future<decltype(func(std::declval<DBConnection*>()))> {
    ensure_initialized();
    return execute_async_with_connection(*pool_, func);
}
*/

// Convenience macros for common operations
#define DB_EXECUTE_QUERY(manager, query) \
    (manager).execute_query(query)

#define DB_EXECUTE_UPDATE(manager, query) \
    (manager).execute_update(query)

#define DB_EXECUTE_TRANSACTION(manager, func) \
    (manager).execute_transaction([&](DB::DBConnection* conn) { return func(conn); })

#define DB_EXECUTE_PREPARED(manager, query, bind_func) \
    (manager).execute_prepared(query, [&](DB::DBConnection::PreparedStatement* stmt) { return bind_func(stmt); })

}  // namespace DB