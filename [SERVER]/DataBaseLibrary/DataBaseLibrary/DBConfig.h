#pragma once
#include <string>
#include <chrono>

namespace DB {

struct DBConfig {
    std::string host = "localhost";
    std::string user = "root";
    std::string password = "";
    std::string database = "";
    unsigned int port = 3306;
    
    // Connection options
    unsigned int connection_timeout = 10;  // seconds
    unsigned int read_timeout = 30;        // seconds
    unsigned int write_timeout = 30;       // seconds
    bool auto_reconnect = true;
    std::string charset = "utf8mb4";
    
    // SSL options
    bool use_ssl = false;
    std::string ssl_ca_path = "";
    std::string ssl_cert_path = "";
    std::string ssl_key_path = "";
    
    // Connection pool settings
    size_t pool_min_size = 1;
    size_t pool_max_size = 10;
    std::chrono::seconds pool_idle_timeout{300};  // 5 minutes
    std::chrono::seconds pool_validation_interval{60};  // 1 minute
    
    // Validation
    bool is_valid() const {
        return !host.empty() && !user.empty() && !database.empty() && port > 0;
    }
    
    std::string to_string() const {
        return "mysql://" + user + "@" + host + ":" + std::to_string(port) + "/" + database;
    }
};

}  // namespace DB