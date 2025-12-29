#pragma once

/**
 * @file Database.h
 * @brief Main include header for the Database Library
 * 
 * This header provides a convenient way to include all necessary components
 * of the database library. Simply include this file to access all functionality.
 * 
 * @author DatabaseLibrary Team
 * @version 1.0
 * @date 2024
 * 
 * @example Basic Usage:
 * @code
 * #include "Database.h"
 * 
 * int main() {
 *     DB::DBConfig config;
 *     config.host = "localhost";
 *     config.user = "root";
 *     config.password = "password";
 *     config.database = "mydb";
 *     
 *     DB::DatabaseManager db(config);
 *     db.initialize();
 *     
 *     auto result = db.execute_query("SELECT * FROM users");
 *     for (const auto& row : result) {
 *         std::cout << row.get<std::string>("username") << std::endl;
 *     }
 *     
 *     return 0;
 * }
 * @endcode
 */

// Core includes
#include "DBConfig.h"
#include "DBException.h"
#include "QueryResult.h"
#include "DBConnection.h"
#include "DBConnectionPool.h"
#include "DatabaseManager.h"

/**
 * @namespace DB
 * @brief Main namespace for all database library components
 * 
 * All classes, functions, and types in this library are contained within
 * the DB namespace to avoid naming conflicts.
 */
namespace DB {

/**
 * @brief Library version information
 */
constexpr int LIBRARY_VERSION_MAJOR = 1;
constexpr int LIBRARY_VERSION_MINOR = 0;
constexpr int LIBRARY_VERSION_PATCH = 0;

/**
 * @brief Get library version as string
 * @return Version string in format "major.minor.patch"
 */
inline std::string get_library_version() {
    return std::to_string(LIBRARY_VERSION_MAJOR) + "." +
           std::to_string(LIBRARY_VERSION_MINOR) + "." +
           std::to_string(LIBRARY_VERSION_PATCH);
}

/**
 * @brief Initialize MySQL library globally
 * 
 * Call this once at the beginning of your program if you're using
 * multiple threads. This is optional but recommended for thread safety.
 * 
 * @return true if initialization succeeded, false otherwise
 */
inline bool initialize_mysql_library() {
    return mysql_library_init(0, nullptr, nullptr) == 0;
}

/**
 * @brief Cleanup MySQL library globally
 * 
 * Call this once at the end of your program to cleanup MySQL library
 * resources. This is optional but good practice.
 */
inline void cleanup_mysql_library() {
    mysql_library_end();
}

/**
 * @brief RAII helper for MySQL library initialization
 * 
 * Use this class to automatically initialize and cleanup the MySQL library.
 * Create an instance at the beginning of main() and it will handle the rest.
 */
class MySQLLibraryManager {
public:
    MySQLLibraryManager() {
        initialized_ = initialize_mysql_library();
    }
    
    ~MySQLLibraryManager() {
        if (initialized_) {
            cleanup_mysql_library();
        }
    }
    
    bool is_initialized() const { return initialized_; }
    
private:
    bool initialized_;
};

// Type aliases for convenience
using Config = DBConfig;
using Connection = DBConnection;
using ConnectionPool = DBConnectionPool;
using Manager = DatabaseManager;
using Result = QueryResult;
using Row = QueryResult::Row;
using Transaction = DBConnection::Transaction;
using PreparedStatement = DBConnection::PreparedStatement;

// Exception type aliases
using Exception = DBException;
using ConnectionException = DB::ConnectionException;
using QueryException = DB::QueryException;
using TransactionException = DB::TransactionException;

} // namespace DB

/**
 * @brief Quick start guide
 * 
 * ## Basic Usage
 * 
 * 1. **Include the library**
 *    ```cpp
 *    #include "Database.h"
 *    ```
 * 
 * 2. **Configure your database**
 *    ```cpp
 *    DB::Config config;
 *    config.host = "localhost";
 *    config.user = "your_user";
 *    config.password = "your_password";
 *    config.database = "your_database";
 *    ```
 * 
 * 3. **Create and initialize manager**
 *    ```cpp
 *    DB::Manager db(config);
 *    db.initialize();
 *    ```
 * 
 * 4. **Execute queries**
 *    ```cpp
 *    // Simple query
 *    auto result = db.execute_query("SELECT * FROM users");
 *    
 *    // Insert data
 *    auto user_id = db.execute_insert("INSERT INTO users (name) VALUES ('John')");
 *    
 *    // Update data
 *    auto affected = db.execute_update("UPDATE users SET name='Jane' WHERE id=1");
 *    ```
 * 
 * 5. **Process results**
 *    ```cpp
 *    for (const auto& row : result) {
 *        std::cout << "ID: " << row.get<int>("id") << std::endl;
 *        std::cout << "Name: " << row.get<std::string>("name") << std::endl;
 *    }
 *    ```
 * 
 * ## Advanced Features
 * 
 * ### Prepared Statements
 * ```cpp
 * DB_EXECUTE_PREPARED(db, "SELECT * FROM users WHERE age > ?", [](auto* stmt) {
 *     stmt->bind_int(0, 18);
 *     return stmt->execute_query();
 * });
 * ```
 * 
 * ### Transactions
 * ```cpp
 * DB_EXECUTE_TRANSACTION(db, [](auto* conn) {
 *     conn->execute("INSERT INTO users (name) VALUES ('User1')");
 *     conn->execute("INSERT INTO users (name) VALUES ('User2')");
 *     return true; // commit
 * });
 * ```
 * 
 * ### Connection Pool Configuration
 * ```cpp
 * config.pool_min_size = 5;    // Minimum connections
 * config.pool_max_size = 20;   // Maximum connections
 * config.pool_idle_timeout = std::chrono::seconds(300); // 5 minutes
 * ```
 * 
 * ### Error Handling
 * ```cpp
 * try {
 *     auto result = db.execute_query("SELECT * FROM users");
 * } catch (const DB::QueryException& e) {
 *     std::cerr << "Query error: " << e.what() << std::endl;
 * } catch (const DB::ConnectionException& e) {
 *     std::cerr << "Connection error: " << e.what() << std::endl;
 * }
 * ```
 * 
 * ## Best Practices
 * 
 * 1. **Use connection pooling** - Always use DatabaseManager instead of direct connections
 * 2. **Handle exceptions** - Wrap database operations in try-catch blocks
 * 3. **Use prepared statements** - For repeated queries or user input
 * 4. **Use transactions** - For operations that must succeed or fail together
 * 5. **Monitor pool health** - Check pool statistics periodically
 * 6. **Configure timeouts** - Set appropriate connection and query timeouts
 * 7. **Use RAII** - Let smart pointers and RAII classes handle resource cleanup
 */