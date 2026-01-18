#pragma once
#include <functional>
#include <memory>
#include <string>

namespace DB {

template<typename SuccessCallbackType>
class AsyncDBContext {
public:
    // 성공 시 호출될 콜백 (타입은 사용자 정의)
     std::optional<SuccessCallbackType>  success_callback;
    
    // 실패 시 호출될 콜백 (표준화)
    std::function<void(const std::string&)> error_callback;
    
    // 사용자 정의 데이터 (필요시 상속해서 확장 가능)
    void* user_data = nullptr;

public:
    AsyncDBContext() = default;
    virtual ~AsyncDBContext() = default;
    
    /**
     * @brief DB 작업 성공 결과 전달
     * @tparam ResultType 결과 타입 (auto로 추론)
     * @param result DB 작업 결과
     */
    template<typename ResultType>
    void deliver_success(ResultType& result) {
        if (success_callback) {
            success_callback.value()(result);
        }
    }
    
    /**
     * @brief DB 작업 성공 전달 (결과 없는 경우)
     */
    void deliver_success() {
        if (success_callback) {
            success_callback();
        }
    }
    
    /**
     * @brief DB 작업 실패 전달
     * @param error_message 에러 메시지
     */
    void deliver_error(const std::string& error_message) {
        if (error_callback) {
            error_callback(error_message);
        }
    }
    
    /**
     * @brief 사용자 데이터 설정
     * @tparam T 데이터 타입
     * @param data 저장할 데이터
     */
    template<typename T>
    void set_user_data(T* data) {
        user_data = static_cast<void*>(data);
    }
    
    /**
     * @brief 사용자 데이터 가져오기
     * @tparam T 데이터 타입
     * @return 저장된 데이터 (nullptr일 수 있음)
     */
    template<typename T>
    T* get_user_data() const {
        return static_cast<T*>(user_data);
    }
};

using VoidAsyncDBContext = AsyncDBContext<std::function<void()>>;
using QueryAsyncDBContext = AsyncDBContext<std::function<void(const QueryResult&)>>;
using InsertAsyncDBContext = AsyncDBContext<std::function<void(uint64_t)>>; // inserted id
using UpdateAsyncDBContext = AsyncDBContext<std::function<void(uint64_t)>>; // affected rows

template<typename SuccessCallback>
auto create_async_context(SuccessCallback success_cb, std::function<void(const std::string&)> error_cb = nullptr) {
    auto context = std::make_shared<AsyncDBContext<SuccessCallback>>();
    context->success_callback.emplace(std::move(success_cb));
    context->error_callback = std::move(error_cb);
    return context;
}

// 편의 매크로
#define DB_CREATE_CONTEXT(success_callback, error_callback) \
    DB::create_async_context(success_callback, error_callback)

} 