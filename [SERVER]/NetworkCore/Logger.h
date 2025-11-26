#pragma once

#define LOG_MAX_COUNT 500


enum class e_log_level
{
    LV_DEBUG = 1,
    LV_WARNING = 2,
    LV_ERROR = 3,
    LV_STOP = 4,
    
};

struct file_log_info
{
    e_log_level log_level;
    std::tm* request_time = nullptr;
    std::string log_context;
    std::string log_request_fuction_name;
    std::string log_request_line;
};

class Logger
{
public:
    Logger();
    ~Logger();

public:
    Logger& get_logger()
    {
        static Logger logger;
        return logger;
    }

    
    static std::string get_log_level_str(e_log_level level);

public:
    void init();
    
    bool start(std::string file_path, std::string file_name);
    void request_log(e_log_level level, std::string&& context, std::string request_function_name, std::string request_line);
    void stop();

private:
    void logger_thread_work();
    
    bool open_new_file();
    bool write(file_log_info& info);
private:
    std::ofstream* m_current_file;
    std::thread m_logger_thread;
    bool m_is_running;

    int m_file_counter;
    int m_log_counter;
    
    std::string m_init_file_name;
    std::string m_init_file_path;
    std::string m_current_file_name;
    std::tm* m_current_file_time;

    concurrency::concurrent_queue<file_log_info> m_log_datas;
};
