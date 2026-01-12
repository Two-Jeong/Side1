#include "DataBaseLibrary/DatabaseManager.h"
#include <iostream>
#include <string>

namespace DB {

class JsonConfigExample {
public:
    static void demonstrate_json_config() {
        std::cout << "=== JSON Config 사용 예시 ===" << std::endl;
        
        // 1. JSON 파일에서 설정 로드하여 초기화
        try {
            std::cout << "1. Development 환경 설정 로드..." << std::endl;
            DB_INITIALIZE_FROM_JSON("config/database_config_dev.json");
            
            // 현재 설정 확인
            auto& db = DB_INSTANCE();
            auto config = db.get_config();
            std::cout << "   연결된 DB: " << config.to_string() << std::endl;
            std::cout << "   풀 크기: " << config.pool_min_size << "~" << config.pool_max_size << std::endl;
            
            // DB 작업 수행
            test_database_operations();
            
            DB_SHUTDOWN();
            
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }
    
    static void demonstrate_environment_configs() {
        std::cout << "\n=== 환경별 설정 예시 ===" << std::endl;
        
        // 환경에 따른 설정 파일 선택
        std::string environment = get_environment();  // 실제로는 환경변수나 커맨드라인 인자에서 가져옴
        std::string config_file;
        
        if (environment == "development") {
            config_file = "config/database_config_dev.json";
        } else if (environment == "production") {
            config_file = "config/database_config_prod.json";
        } else {
            config_file = "config/database_config.json";  // 기본값
        }
        
        std::cout << "환경: " << environment << std::endl;
        std::cout << "설정 파일: " << config_file << std::endl;
        
        try {
            // 환경에 맞는 설정으로 초기화
            DB_INITIALIZE_FROM_JSON(config_file);
            
            auto& db = DB_INSTANCE();
            auto config = db.get_config();
            std::cout << "초기화 완료: " << config.to_string() << std::endl;
            
            DB_SHUTDOWN();
            
        } catch (const std::exception& e) {
            std::cerr << "초기화 실패: " << e.what() << std::endl;
        }
    }
    
    
private:
    static void test_database_operations() {
        std::cout << "   기본 DB 작업 테스트..." << std::endl;
        
        try {
            // 간단한 테스트 쿼리 (실제 DB 연결 없이는 실행되지 않음)
            // auto result = DB_EXECUTE_QUERY("SELECT 1 as test");
            // std::cout << "   쿼리 테스트 성공" << std::endl;
            
            auto& db = DB_INSTANCE();
            auto stats = db.get_pool_statistics();
            std::cout << "   연결 풀 상태 확인 완료" << std::endl;
            
        } catch (const std::exception& e) {
            std::cout << "   테스트 실패 (예상됨): " << e.what() << std::endl;
        }
    }
    
    static std::string get_environment() {
        // 실제로는 환경변수나 커맨드라인 인자에서 가져옴
        // const char* env = std::getenv("APP_ENVIRONMENT");
        // return env ? env : "development";
        return "development";  // 예시를 위한 하드코딩
    }
};

}

int main() {
    std::cout << "DataBase JSON Config 예시 프로그램" << std::endl;
    std::cout << "=====================================" << std::endl;
    
    DB::JsonConfigExample::demonstrate_json_config();
    DB::JsonConfigExample::demonstrate_environment_configs();
    
    return 0;
}