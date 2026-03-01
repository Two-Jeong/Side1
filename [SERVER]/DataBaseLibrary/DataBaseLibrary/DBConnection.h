#pragma once
#include <mysql/mysql.h>
#include <string>
#include <memory>
#include <vector>
#include <chrono>
#include <functional>
#include "DBConfig.h"
#include "DBException.h"
#include "QueryResult.h"

namespace DB {

class DBConnection {
public:
    explicit DBConnection(const DBConfig& config);
    ~DBConnection();
    
    // Move-only semantics
    DBConnection(const DBConnection&) = delete;
    DBConnection& operator=(const DBConnection&) = delete;
    DBConnection(DBConnection&& other) noexcept;
    DBConnection& operator=(DBConnection&& other) noexcept;
    
    // Connection management
    void connect();
    void disconnect();
    bool is_connected() const;
    bool is_valid() const;
    void ping() const;
    void reset();
    
    // Query execution
    QueryResult execute_query(const std::string& query);
    uint64_t execute_update(const std::string& query);
    uint64_t execute_insert(const std::string& query);
    void execute(const std::string& query);
    
    // Prepared statements
    class PreparedStatement {
    public:
        PreparedStatement(MYSQL* connection, const std::string& query);
        ~PreparedStatement();
        
        // Move-only semantics
        PreparedStatement(const PreparedStatement&) = delete;
        PreparedStatement& operator=(const PreparedStatement&) = delete;
        PreparedStatement(PreparedStatement&& other) noexcept;
        PreparedStatement& operator=(PreparedStatement&& other) noexcept;
        
        // Parameter binding
        void bind_string(int index, const std::string& value);
        void bind_int(int index, int value);
        void bind_long(int index, long long value);
        void bind_double(int index, double value);
        void bind_null(int index);
        void bind_datetime(int index, const std::chrono::system_clock::time_point& time);
        
        // Execution
        QueryResult execute_query();
        uint64_t execute_update();
        void execute();
        
        void clear_bindings();
        
    private:
        MYSQL_STMT* stmt_;
        std::vector<MYSQL_BIND> binds_;
        std::vector<std::string> string_buffers_;  // Keep strings alive
        std::vector<long long> numeric_buffers_;   // Keep numeric values alive
        std::vector<my_bool> null_indicators_;
        bool prepared_;
        
        void ensure_bind_capacity(int index);
        void reset_statement();
    };
    
    std::unique_ptr<PreparedStatement> prepare(const std::string& query);
    
    // Transaction support
    class Transaction {
    public:
        explicit Transaction(DBConnection& connection);
        ~Transaction();
        
        void commit();
        void rollback();
        bool is_active() const { return active_; }
        
    private:
        DBConnection& connection_;
        bool active_;
        bool committed_;
    };
    
    std::unique_ptr<Transaction> begin_transaction();
    void commit();
    void rollback();
    void set_autocommit(bool enable);
    
    // Utility methods
    std::string escape_string(const std::string& str);
    uint64_t get_last_insert_id() const;
    uint64_t get_affected_rows() const;
    std::string get_last_error() const;
    int get_last_error_code() const;
    
    // Connection info
    const DBConfig& get_config() const { return config_; }
    std::string get_server_info() const;
    std::string get_client_info() const;
    std::chrono::system_clock::time_point get_last_activity_time() const { return last_activity_; }
    
private:
    MYSQL* mysql_;
    DBConfig config_;
    bool connected_;
    std::chrono::system_clock::time_point last_activity_;
    
    void setup_connection_options();
    void check_connection() const;
    void update_activity_time() const;
    std::string format_mysql_error(const std::string& operation) const;
};

// Helper class for automatic resource cleanup
class ConnectionGuard {
public:
    explicit ConnectionGuard(DBConnection& connection) 
        : connection_(connection) {
        connection_.connect();
    }
    
    ~ConnectionGuard() {
        try {
            if (connection_.is_connected()) {
                connection_.disconnect();
            }
        } catch (...) {
            // Ignore errors during cleanup
        }
    }
    
private:
    DBConnection& connection_;
};

}  // namespace DB