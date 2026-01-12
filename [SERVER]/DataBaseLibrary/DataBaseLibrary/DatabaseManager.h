#pragma once
#include "DBConnectionPool.h"
#include "DBConnection.h"
#include <memory>
#include <functional>
#include <future>
#include <mutex>

namespace DB {


class DatabaseManager {
public:
    // Singleton access
    static DatabaseManager& get_instance();
    static void initialize_instance(const DBConfig& config);
    static void initialize_instance_from_json(const std::string& config_file_path);
    static void shutdown_instance();
    
    // Delete copy constructor and assignment operator
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;
    
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
    // Private constructor for singleton
    explicit DatabaseManager(const DBConfig& config);
public:
    ~DatabaseManager();
    
private:
    // Static members
    static std::unique_ptr<DatabaseManager> instance_;
    static std::mutex instance_mutex_;
    
    // Instance members
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

// Convenience macros for singleton pattern
#define DB_INSTANCE() \
    DB::DatabaseManager::get_instance()

#define DB_EXECUTE_QUERY(query) \
    DB::DatabaseManager::get_instance().execute_query(query)

#define DB_EXECUTE_UPDATE(query) \
    DB::DatabaseManager::get_instance().execute_update(query)

#define DB_EXECUTE_TRANSACTION(func) \
    DB::DatabaseManager::get_instance().execute_transaction([&](DB::DBConnection* conn) { return func(conn); })

#define DB_EXECUTE_PREPARED(query, bind_func) \
    DB::DatabaseManager::get_instance().execute_prepared(query, [&](DB::DBConnection::PreparedStatement* stmt) { return bind_func(stmt); })

#define DB_INITIALIZE(config) \
    DB::DatabaseManager::get_instance().initialize_instance(config)

#define DB_INITIALIZE_FROM_JSON(file_path) \
    DB::DatabaseManager::get_instance().initialize_instance_from_json(file_path)

#define DB_SHUTDOWN() \
    DB::DatabaseManager::get_instance().shutdown_instance()

}  // namespace DB