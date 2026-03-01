#pragma once
#include <mysql/mysql.h>
#include <string>
#include <vector>
#include <memory>
#include <optional>
#include <any>
#include <unordered_map>
#include <chrono>
#include <sstream>
#include <iomanip>
#include "DBException.h"

namespace DB {

class QueryResult {
public:
    class Row {
    public:
        Row(MYSQL_ROW row, unsigned long* lengths, const std::vector<std::string>& field_names);
        
        // Type-safe getters with automatic conversion
        template<typename T>
        T get(const std::string& field_name) const;
        
        template<typename T>
        T get(size_t index) const;
        
        template<typename T>
        std::optional<T> get_optional(const std::string& field_name) const;
        
        template<typename T>
        std::optional<T> get_optional(size_t index) const;
        
        bool is_null(const std::string& field_name) const;
        bool is_null(size_t index) const;
        
        size_t field_count() const { return values_.size(); }
        const std::vector<std::string>& get_field_names() const { return field_names_; }
        
    private:
        std::vector<std::string> values_;
        std::vector<std::string> field_names_;
        std::unordered_map<std::string, size_t> field_index_map_;
        
        size_t get_field_index(const std::string& field_name) const;
        template<typename T>
        T convert_value(const std::string& value) const;
    };
    
    class Iterator {
    public:
        explicit Iterator(QueryResult* result, bool is_end = false);
        
        Row operator*() const;
        Iterator& operator++();
        bool operator!=(const Iterator& other) const;
        bool operator==(const Iterator& other) const;
        
    private:
        QueryResult* result_;
        MYSQL_ROW current_row_;
        bool is_end_;
        unsigned long* lengths_;
    };

public:
    explicit QueryResult(MYSQL_RES* result);
    ~QueryResult();
    
    // Move-only semantics
    QueryResult(const QueryResult&) = delete;
    QueryResult& operator=(const QueryResult&) = delete;
    QueryResult(QueryResult&& other) noexcept;
    QueryResult& operator=(QueryResult&& other) noexcept;
    
    // Iterator interface
    Iterator begin();
    Iterator end();
    
    // Direct access methods
    std::vector<Row> fetch_all();
    std::optional<Row> fetch_one();
    
    // Metadata
    size_t row_count() const { return row_count_; }
    size_t field_count() const { return field_count_; }
    const std::vector<std::string>& get_field_names() const { return field_names_; }
    
    bool empty() const { return row_count_ == 0; }
    bool is_valid() const { return result_ != nullptr; }
    
private:
    MYSQL_RES* result_;
    size_t row_count_;
    size_t field_count_;
    std::vector<std::string> field_names_;
    
    void extract_metadata();
};

// Template implementations
template<typename T>
T QueryResult::Row::get(const std::string& field_name) const {
    return get<T>(get_field_index(field_name));
}

template<typename T>
T QueryResult::Row::get(size_t index) const {
    if (index >= values_.size()) {
        throw DBException("Field index out of range: " + std::to_string(index));
    }
    
    if (values_[index].empty()) {
        throw DBException("Field value is NULL at index: " + std::to_string(index));
    }
    
    return convert_value<T>(values_[index]);
}

template<typename T>
std::optional<T> QueryResult::Row::get_optional(const std::string& field_name) const {
    return get_optional<T>(get_field_index(field_name));
}

template<typename T>
std::optional<T> QueryResult::Row::get_optional(size_t index) const {
    if (index >= values_.size() || values_[index].empty()) {
        return std::nullopt;
    }
    
    try {
        return convert_value<T>(values_[index]);
    } catch (...) {
        return std::nullopt;
    }
}

// Specializations for common types
template<>
inline std::string QueryResult::Row::convert_value<std::string>(const std::string& value) const {
    return value;
}

template<>
inline int QueryResult::Row::convert_value<int>(const std::string& value) const {
    return std::stoi(value);
}

template<>
inline long QueryResult::Row::convert_value<long>(const std::string& value) const {
    return std::stol(value);
}

template<>
inline long long QueryResult::Row::convert_value<long long>(const std::string& value) const {
    return std::stoll(value);
}

template<>
inline float QueryResult::Row::convert_value<float>(const std::string& value) const {
    return std::stof(value);
}

template<>
inline double QueryResult::Row::convert_value<double>(const std::string& value) const {
    return std::stod(value);
}

template<>
inline bool QueryResult::Row::convert_value<bool>(const std::string& value) const {
    return value == "1" || value == "true" || value == "TRUE";
}

template<>
inline std::chrono::system_clock::time_point QueryResult::Row::convert_value<std::chrono::system_clock::time_point>(const std::string& value) const {
    // Parse MySQL datetime format: YYYY-MM-DD HH:MM:SS
    std::tm tm = {};
    std::istringstream ss(value);
    ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
    if (ss.fail()) {
        throw DBException("Invalid datetime format: " + value);
    }
    return std::chrono::system_clock::from_time_t(std::mktime(&tm));
}

}  // namespace DB