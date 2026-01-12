#include "DatabaseManager.h"
#include <iostream>
#include <string>
#include <chrono>

// Simple demonstration and testing of the singleton database library
int main() {
    try {
        std::cout << "=== Singleton Database Library Test ===" << std::endl;
	
		DB::DBConfig config;
		config.host = "localhost";
		config.user = "root";
		config.password = "eunj1109^^";  // Change this to your actual password
		config.database = "test";
		config.port = 3306;
		config.pool_min_size = 2;
		config.pool_max_size = 10;
		
		std::cout << "Connecting to: " << config.to_string() << std::endl;
		DB_INITIALIZE(config);
        
        // Get reference to singleton instance
        auto& db_manager = DB_INSTANCE();
        
        // Create database if not exists
        std::cout << "Creating database if not exists..." << std::endl;
        try {
            db_manager.create_database_if_not_exists(config.database);
        } catch (const DB::DBException& e) {
            std::cout << "Note: Could not create database (may already exist): " << e.what() << std::endl;
        }
        
        // Test basic query execution using singleton macros
        std::cout << "\n=== Basic Query Tests (Using Singleton Macros) ===" << std::endl;
        
        // Create test table
        std::cout << "Creating test table..." << std::endl;
        DB_INSTANCE().execute(R"(
            CREATE TABLE IF NOT EXISTS users (
                id INT AUTO_INCREMENT PRIMARY KEY,
                username VARCHAR(50) NOT NULL UNIQUE,
                email VARCHAR(100) NOT NULL,
                created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                is_active BOOLEAN DEFAULT TRUE
            )
        )");
        
        // Check if table exists
        std::cout << "Table exists: " << (DB_INSTANCE().table_exists("users") ? "YES" : "NO") << std::endl;
        
        // Insert test data using macro
        std::cout << "Inserting test data..." << std::endl;
        auto insert_id = DB_EXECUTE_UPDATE(
            "INSERT INTO users (username, email) VALUES ('testuser1', 'test1@example.com')"
        );
        std::cout << "Inserted user with affected rows: " << insert_id << std::endl;
        
        // Test prepared statement using singleton macro
        std::cout << "\n=== Prepared Statement Test (Singleton) ===" << std::endl;
        auto prep_result = DB_EXECUTE_PREPARED(
            "INSERT INTO users (username, email, is_active) VALUES (?, ?, ?)",
            [](DB::DBConnection::PreparedStatement* stmt) {
                stmt->bind_string(1, "testuser2");
                stmt->bind_string(2, "test2@example.com");
                stmt->bind_int(3, 1);
                return stmt->execute_update();
            }
        );
        std::cout << "Prepared statement affected rows: " << prep_result << std::endl;
        
        // Test query with results using singleton macro
        std::cout << "\n=== Query Results Test (Singleton) ===" << std::endl;
        auto query_result = DB_EXECUTE_QUERY("SELECT * FROM users ORDER BY id");
        
        std::cout << "Query returned " << query_result.row_count() << " rows:" << std::endl;
        std::cout << "Fields: ";
        for (const auto& field : query_result.get_field_names()) {
            std::cout << field << " ";
        }
        std::cout << std::endl;
        
        for (const auto& row : query_result) {
            std::cout << "  ID: " << row.get<int>("id") 
                      << ", Username: " << row.get<std::string>("username")
                      << ", Email: " << row.get<std::string>("email")
                      << ", Active: " << row.get<bool>("is_active") << std::endl;
        }
        
        // Test transaction using singleton macro
        std::cout << "\n=== Transaction Test (Singleton) ===" << std::endl;
        try {
            auto transaction_result = DB_EXECUTE_TRANSACTION([](DB::DBConnection* conn) {
                // Insert multiple users in a transaction
                conn->execute("INSERT INTO users (username, email) VALUES ('tx_user1', 'tx1@example.com')");
                conn->execute("INSERT INTO users (username, email) VALUES ('tx_user2', 'tx2@example.com')");
                return true;  // Return success
            });
            std::cout << "Transaction completed successfully" << std::endl;
        } catch (const DB::DBException& e) {
            std::cout << "Transaction failed: " << e.what() << std::endl;
        }
        
        // Test error handling
        std::cout << "\n=== Error Handling Test ===" << std::endl;
        try {
            DB_EXECUTE_QUERY("SELECT * FROM non_existent_table");
        } catch (const DB::QueryException& e) {
            std::cout << "Expected error caught: " << e.what() << std::endl;
        }
        
        // Test connection pool statistics using singleton
        std::cout << "\n=== Connection Pool Statistics ===" << std::endl;
        auto stats = DB_INSTANCE().get_pool_statistics();
        std::cout << "Total connections: " << stats.total_connections << std::endl;
        std::cout << "Active connections: " << stats.active_connections << std::endl;
        std::cout << "Idle connections: " << stats.idle_connections << std::endl;
        std::cout << "Total acquired: " << stats.total_acquired << std::endl;
        std::cout << "Pool healthy: " << (DB_INSTANCE().is_pool_healthy() ? "YES" : "NO") << std::endl;
        
        // Performance test using singleton macros
        std::cout << "\n=== Performance Test (Singleton Macros) ===" << std::endl;
        auto start_time = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < 100; ++i) {
            auto test_result = DB_EXECUTE_QUERY("SELECT COUNT(*) as user_count FROM users");
            auto count_row = test_result.fetch_one();
            if (i == 0 && count_row) {
                std::cout << "Current user count: " << count_row->get<int>("user_count") << std::endl;
            }
        }
        
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        std::cout << "100 queries completed in " << duration.count() << "ms" << std::endl;
        std::cout << "Average: " << (duration.count() / 100.0) << "ms per query" << std::endl;
        
        // Clean up test data using singleton macro
        std::cout << "\n=== Cleanup ===" << std::endl;
        auto deleted_rows = DB_EXECUTE_UPDATE("DELETE FROM users WHERE username LIKE 'test%' OR username LIKE 'tx_%'");
        std::cout << "Cleaned up " << deleted_rows << " test records" << std::endl;
        
        // Get table list using singleton
        std::cout << "Tables in database:" << std::endl;
        auto tables = DB_INSTANCE().get_table_list();
        for (const auto& table : tables) {
            std::cout << "  " << table << std::endl;
        }
        
        // Demonstrate configuration access
        std::cout << "\n=== Configuration Info ===" << std::endl;
        std::cout << "Connected to: " << config.to_string() << std::endl;
        std::cout << "Pool size: " << config.pool_min_size << " - " << config.pool_max_size << std::endl;
        std::cout << "SSL enabled: " << (config.use_ssl ? "YES" : "NO") << std::endl;
        
        // Clean shutdown of singleton
        std::cout << "\n=== Shutting Down ===" << std::endl;
        DB_SHUTDOWN();
        std::cout << "✓ Database manager shutdown completed" << std::endl;
        
        std::cout << "\n=== Singleton Test Completed Successfully ===" << std::endl;
        
    } catch (const DB::DBException& e) {
        std::cerr << "Database error: " << e.what() << std::endl;
        std::cerr << "Error code: " << e.get_error_code() << std::endl;
        
        // Ensure cleanup even on error
        try {
            DB_SHUTDOWN();
        } catch (...) {
            // Ignore shutdown errors
        }
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Standard exception: " << e.what() << std::endl;
        
        // Ensure cleanup even on error
        try {
            DB_SHUTDOWN();
        } catch (...) {
            // Ignore shutdown errors
        }
        return 1;
    } catch (...) {
        std::cerr << "Unknown exception occurred" << std::endl;
        
        // Ensure cleanup even on error
        try {
            DB_SHUTDOWN();
        } catch (...) {
            // Ignore shutdown errors
        }
        return 1;
    }
    
    return 0;
}