#pragma once
#include <nlohmann/json.hpp>
#include <fstream>

struct LoginServerConfig {

    std::string login_server_ip;
    int login_server_port;
    std::string game_server_ip;
    int game_server_port;
    
    static LoginServerConfig from_json_file(const std::string& file_path) {
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
    static LoginServerConfig from_json(const nlohmann::json& j) {
        LoginServerConfig config;
        
        try {
            config.login_server_ip = j.value("login_server_ip", "0.0.0.0");
            config.login_server_port = j.value("login_server_port", 25000);
            config.game_server_ip = j.value("game_server_ip", "0.0.0.0");
            config.game_server_port = j.value("game_server_port", 50000);
            
        } catch (const nlohmann::json::exception& e) {
            throw std::runtime_error("Error parsing JSON config: " + std::string(e.what()));
        }
        
        return config;
    }
    
};
