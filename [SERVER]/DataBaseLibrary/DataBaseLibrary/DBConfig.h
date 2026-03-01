#pragma once
#include <string>
#include <chrono>
#include <nlohmann/json.hpp>
#include <fstream>
#include <stdexcept>

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
    
    // JSON 파일에서 설정 로드
    static DBConfig from_json_file(const std::string& file_path) {
        std::ifstream file(file_path);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open config file: " + file_path);
        }
        
        nlohmann::json j;
        try {
            file >> j;
        } catch (const nlohmann::json::exception& e) {
            throw std::runtime_error("Failed to parse JSON config: " + std::string(e.what()));
        }
        
        return from_json(j);
    }
    
    // JSON 객체에서 설정 로드
    static DBConfig from_json(const nlohmann::json& j) {
        DBConfig config;
        
        try {
            // 필수 필드
            config.host = j.value("host", "localhost");
            config.user = j.value("user", "root");
            config.password = j.value("password", "");
            config.database = j.value("database", "");
            config.port = j.value("port", 3306);
            
            // 연결 옵션
            config.connection_timeout = j.value("connection_timeout", 10);
            config.read_timeout = j.value("read_timeout", 30);
            config.write_timeout = j.value("write_timeout", 30);
            config.auto_reconnect = j.value("auto_reconnect", true);
            config.charset = j.value("charset", "utf8mb4");
            
            
            // 커넥션 풀 설정
            config.pool_min_size = j.value("pool_min_size", 1);
            config.pool_max_size = j.value("pool_max_size", 10);
            
            // 시간 설정 (초 단위로 받아서 std::chrono::seconds로 변환)
            int idle_timeout_sec = j.value("pool_idle_timeout_sec", 300);
            int validation_interval_sec = j.value("pool_validation_interval_sec", 60);
            config.pool_idle_timeout = std::chrono::seconds(idle_timeout_sec);
            config.pool_validation_interval = std::chrono::seconds(validation_interval_sec);
            
        } catch (const nlohmann::json::exception& e) {
            throw std::runtime_error("Error parsing JSON config: " + std::string(e.what()));
        }
        
        return config;
    }
    
};

}  // namespace DB