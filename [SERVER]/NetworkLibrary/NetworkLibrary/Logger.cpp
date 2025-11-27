#include "pch.h"
#include "Logger.h"

#include <string>

Logger::Logger()
{
    start(__FILE__, "log");
}

std::string Logger::get_log_level_str(e_log_level level)
{
    switch (level)
    {
    case e_log_level::LV_DEBUG:
        return "[DEBUG]";
    case e_log_level::LV_WARNING:
        return "[WARNING]";
    case e_log_level::LV_ERROR:
        return "[ERROR]";
    case e_log_level::LV_STOP:
        return "[STOP]";
    }

return "[NULL]";
}

void Logger::init()
{
   m_file_counter = 0; 
}

bool Logger::start(std::string file_path, std::string file_name)
{
    m_is_running = true;
    m_init_file_path = file_path;
    m_init_file_name = file_name;
    
    m_current_file = xnew std::ofstream;
    open_new_file();

    m_logger_thread = std::thread(&Logger::logger_thread_work, this);

    return true;
}

void Logger::request_log(e_log_level level, std::string&& context, std::string request_function_name, std::string request_line)
{
    file_log_info info;
    info.log_level = level;
    info.log_context = std::move(context);
    auto now = std::chrono::system_clock::now();
    std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
    if (0 != localtime_s(info.request_time, &now_time_t))
    {
        //TODO: stop
        return;
    }

    info.log_request_fuction_name = request_function_name;
    info.log_request_line = request_line;
    
    m_log_datas.push(info);
}

void Logger::stop()
{
    m_is_running = false;
}

void Logger::logger_thread_work()
{
   while (m_is_running)
   {
       file_log_info log;

       if (false == m_log_datas.try_pop(log))
       {
           std::this_thread::sleep_for(std::chrono::milliseconds(100));
           continue;
       }

       if (false == write(log))
           m_is_running = false;
   } 
}

bool Logger::open_new_file()
{
    if (nullptr == m_current_file)
    {
        //TODO: xstop
        return false;
    }
    auto now = std::chrono::system_clock::now();
    std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
    localtime_s(m_current_file_time, &now_time_t);

    m_current_file_name = m_init_file_name + "_" + std::to_string(m_current_file_time->tm_year + 1900) + std::to_string(m_current_file_time->tm_mon + 1) + std::to_string(m_current_file_time->tm_mday) + "_" + std::to_string(m_file_counter);
    m_current_file->open(m_init_file_path + "\\" + m_current_file_name);

    if (false == m_current_file->is_open())
    {
        //TODO: xstop 
        return false;
    }

    return true;
}

bool Logger::write(file_log_info& info)
{
    if (m_log_counter >= LOG_MAX_COUNT)
    {
        open_new_file();
        m_log_counter = 0;
        m_file_counter++;
    }
    
    std::string log_level_str = get_log_level_str(info.log_level);
    std::string log_write_time = "[ " + std::to_string(info.request_time->tm_year + 1900) + std::to_string(info.request_time->tm_mon + 1) + std::to_string(info.request_time->tm_mday) + "_" + std::to_string(info.request_time->tm_hour) + std::to_string(info.request_time->tm_min) + std::to_string(info.request_time->tm_sec) + "]";
    
    std::string total_context = log_write_time +" " + info.log_context + " " + info.log_context + " " + info.log_request_fuction_name + "(" + info.log_request_line + ")\n";  
    m_current_file->write(total_context.c_str(), total_context.length());

    if (false == m_current_file->good())
    {
        //TODO stop
        return false;
    }

    m_log_counter++;
    
    return true;
}
