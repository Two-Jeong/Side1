#pragma once

struct iTask
{
    bool is_repeat = false; // 패킷 처리 task는 무조건 false => 실행 후 패킷을 delete함.
    std::chrono::steady_clock::time_point execute_time;
    long long delay_time = 0;
    std::function<void()> func;
    std::function<void()> post_processing_func;
};