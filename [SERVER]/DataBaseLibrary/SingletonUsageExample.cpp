#include "DataBaseLibrary/DatabaseManager.h"
#include <iostream>

namespace DB {

// Example usage of singleton DatabaseManager
class DatabaseExample {
public:
    static void demonstrate_singleton_usage() {
        try {
            // 방법 1: JSON 파일에서 설정 로드 (추천)
            DB_INITIALIZE_FROM_JSON("config/database_config_dev.json");
            
            // 또는 방법 2: 직접 Config 객체 생성
            // DBConfig config;
            // config.host = "localhost";
            // config.user = "root";
            // config.password = "password123";
            // config.database = "game_db";
            // config.port = 3306;
            // config.pool_min_size = 5;
            // config.pool_max_size = 20;
            // DB_INITIALIZE(config);
            
            // 3. 이후 어디서든 싱글톤 패턴으로 사용
            demonstrate_basic_operations();
            demonstrate_transactions();
            demonstrate_prepared_statements();
            
            // 4. 프로그램 종료시 정리
            DB_SHUTDOWN();
            
        } catch (const std::exception& e) {
            std::cerr << "Database error: " << e.what() << std::endl;
        }
    }
    
private:
    static void demonstrate_basic_operations() {
        std::cout << "=== Basic Operations ===" << std::endl;
        
        // 간단한 쿼리 실행 - 매크로 사용
        auto result = DB_EXECUTE_QUERY("SELECT * FROM users LIMIT 5");
        std::cout << "Query result count: " << result.size() << std::endl;
        
        // 업데이트 쿼리
        uint64_t affected_rows = DB_EXECUTE_UPDATE("UPDATE users SET last_login = NOW() WHERE user_id = 1");
        std::cout << "Updated rows: " << affected_rows << std::endl;
        
        // 직접 인스턴스 접근도 가능
        auto& db = DB_INSTANCE();
        bool table_exists = db.table_exists("users");
        std::cout << "Users table exists: " << (table_exists ? "Yes" : "No") << std::endl;
    }
    
    static void demonstrate_transactions() {
        std::cout << "\n=== Transaction Example ===" << std::endl;
        
        // 트랜잭션 실행 - 매크로 사용
        auto result = DB_EXECUTE_TRANSACTION([](DBConnection* conn) {
            // 복잡한 비즈니스 로직
            conn->execute("INSERT INTO users (username, email) VALUES ('test_user', 'test@example.com')");
            auto user_id = conn->get_last_insert_id();
            
            conn->execute("INSERT INTO user_profiles (user_id, created_at) VALUES (" + 
                         std::to_string(user_id) + ", NOW())");
            
            return user_id;
        });
        
        std::cout << "Transaction completed, new user ID: " << result << std::endl;
    }
    
    static void demonstrate_prepared_statements() {
        std::cout << "\n=== Prepared Statement Example ===" << std::endl;
        
        // Prepared Statement 사용
        auto result = DB_EXECUTE_PREPARED(
            "SELECT * FROM users WHERE username = ? AND email = ?",
            [](DBConnection::PreparedStatement* stmt) {
                stmt->bind_string(1, "test_user");
                stmt->bind_string(2, "test@example.com");
                return stmt->execute_query();
            }
        );
        
        std::cout << "Prepared query result count: " << result.size() << std::endl;
    }
    
public:
    // 다른 클래스에서 사용하는 예시
    static void usage_in_different_classes() {
        // LoginServer에서 사용
        class LoginManager {
        public:
            bool authenticate_user(const std::string& username, const std::string& password) {
                try {
                    auto result = DB_EXECUTE_PREPARED(
                        "SELECT user_id, password_hash FROM users WHERE username = ?",
                        [&](DBConnection::PreparedStatement* stmt) {
                            stmt->bind_string(1, username);
                            return stmt->execute_query();
                        }
                    );
                    
                    if (result.size() == 1) {
                        auto row = result.fetch_one();
                        // 실제로는 password hashing 검증 로직이 들어감
                        return true;
                    }
                    return false;
                } catch (const DBException& e) {
                    std::cerr << "Login authentication failed: " << e.what() << std::endl;
                    return false;
                }
            }
        };
        
        // GameServer에서 사용
        class GameDataManager {
        public:
            void save_player_progress(int player_id, int level, int experience) {
                DB_EXECUTE_TRANSACTION([=](DBConnection* conn) {
                    auto stmt = conn->prepare("UPDATE player_data SET level = ?, experience = ?, last_updated = NOW() WHERE player_id = ?");
                    stmt->bind_int(1, level);
                    stmt->bind_int(2, experience);
                    stmt->bind_int(3, player_id);
                    stmt->execute_update();
                    return true;
                });
            }
        };
    }
};

}

int main() {
    DB::DatabaseExample::demonstrate_singleton_usage();
    return 0;
}