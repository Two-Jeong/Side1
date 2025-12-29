#include "QueryResult.h"
#include <cstring>

namespace DB {

// QueryResult::Row implementation
QueryResult::Row::Row(MYSQL_ROW row, unsigned long* lengths, const std::vector<std::string>& field_names)
    : field_names_(field_names) {
    
    values_.reserve(field_names_.size());
    for (size_t i = 0; i < field_names_.size(); ++i) {
        if (row[i] == nullptr) {
            values_.emplace_back("");  // NULL value represented as empty string
        } else {
            values_.emplace_back(row[i], lengths[i]);
        }
        field_index_map_[field_names_[i]] = i;
    }
}

bool QueryResult::Row::is_null(const std::string& field_name) const {
    return is_null(get_field_index(field_name));
}

bool QueryResult::Row::is_null(size_t index) const {
    if (index >= values_.size()) {
        throw DBException("Field index out of range: " + std::to_string(index));
    }
    return values_[index].empty();
}

size_t QueryResult::Row::get_field_index(const std::string& field_name) const {
    auto it = field_index_map_.find(field_name);
    if (it == field_index_map_.end()) {
        throw DBException("Field not found: " + field_name);
    }
    return it->second;
}

// QueryResult::Iterator implementation
QueryResult::Iterator::Iterator(QueryResult* result, bool is_end)
    : result_(result), current_row_(nullptr), is_end_(is_end), lengths_(nullptr) {
    if (!is_end_ && result_ && result_->is_valid()) {
        current_row_ = mysql_fetch_row(result_->result_);
        if (current_row_) {
            lengths_ = mysql_fetch_lengths(result_->result_);
        } else {
            is_end_ = true;
        }
    } else {
        is_end_ = true;
    }
}

QueryResult::Row QueryResult::Iterator::operator*() const {
    if (is_end_ || !current_row_) {
        throw DBException("Iterator is at end or invalid");
    }
    return Row(current_row_, lengths_, result_->field_names_);
}

QueryResult::Iterator& QueryResult::Iterator::operator++() {
    if (!is_end_ && result_ && result_->is_valid()) {
        current_row_ = mysql_fetch_row(result_->result_);
        if (current_row_) {
            lengths_ = mysql_fetch_lengths(result_->result_);
        } else {
            is_end_ = true;
            current_row_ = nullptr;
            lengths_ = nullptr;
        }
    } else {
        is_end_ = true;
    }
    return *this;
}

bool QueryResult::Iterator::operator!=(const Iterator& other) const {
    return is_end_ != other.is_end_ || result_ != other.result_;
}

bool QueryResult::Iterator::operator==(const Iterator& other) const {
    return !(*this != other);
}

// QueryResult implementation
QueryResult::QueryResult(MYSQL_RES* result)
    : result_(result), row_count_(0), field_count_(0) {
    if (result_) {
        row_count_ = mysql_num_rows(result_);
        field_count_ = mysql_num_fields(result_);
        extract_metadata();
    }
}

QueryResult::~QueryResult() {
    if (result_) {
        mysql_free_result(result_);
    }
}

QueryResult::QueryResult(QueryResult&& other) noexcept
    : result_(other.result_), row_count_(other.row_count_), 
      field_count_(other.field_count_), field_names_(std::move(other.field_names_)) {
    other.result_ = nullptr;
    other.row_count_ = 0;
    other.field_count_ = 0;
}

QueryResult& QueryResult::operator=(QueryResult&& other) noexcept {
    if (this != &other) {
        if (result_) {
            mysql_free_result(result_);
        }
        
        result_ = other.result_;
        row_count_ = other.row_count_;
        field_count_ = other.field_count_;
        field_names_ = std::move(other.field_names_);
        
        other.result_ = nullptr;
        other.row_count_ = 0;
        other.field_count_ = 0;
    }
    return *this;
}

QueryResult::Iterator QueryResult::begin() {
    if (result_) {
        mysql_data_seek(result_, 0);  // Reset to beginning
    }
    return Iterator(this, false);
}

QueryResult::Iterator QueryResult::end() {
    return Iterator(this, true);
}

std::vector<QueryResult::Row> QueryResult::fetch_all() {
    std::vector<Row> rows;
    if (!is_valid()) {
        return rows;
    }
    
    mysql_data_seek(result_, 0);  // Reset to beginning
    rows.reserve(row_count_);
    
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result_)) != nullptr) {
        unsigned long* lengths = mysql_fetch_lengths(result_);
        rows.emplace_back(row, lengths, field_names_);
    }
    
    return rows;
}

std::optional<QueryResult::Row> QueryResult::fetch_one() {
    if (!is_valid()) {
        return std::nullopt;
    }
    
    MYSQL_ROW row = mysql_fetch_row(result_);
    if (!row) {
        return std::nullopt;
    }
    
    unsigned long* lengths = mysql_fetch_lengths(result_);
    return Row(row, lengths, field_names_);
}

void QueryResult::extract_metadata() {
    if (!result_) {
        return;
    }
    
    field_names_.reserve(field_count_);
    
    MYSQL_FIELD* fields = mysql_fetch_fields(result_);
    for (size_t i = 0; i < field_count_; ++i) {
        field_names_.emplace_back(fields[i].name);
    }
}

}  // namespace DB