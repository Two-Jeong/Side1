#pragma once
#include <stdexcept>
#include <string>

namespace DB {

class DBException : public std::runtime_error {
public:
    explicit DBException(const std::string& message) 
        : std::runtime_error(message) {}
    
    explicit DBException(const std::string& message, int error_code)
        : std::runtime_error(message), error_code_(error_code) {}
    
    int get_error_code() const noexcept { return error_code_; }

private:
    int error_code_ = 0;
};

class ConnectionException : public DBException {
public:
    explicit ConnectionException(const std::string& message)
        : DBException("Connection Error: " + message) {}
};

class QueryException : public DBException {
public:
    explicit QueryException(const std::string& message)
        : DBException("Query Error: " + message) {}
    
    explicit QueryException(const std::string& message, int error_code)
        : DBException("Query Error: " + message, error_code) {}
};

class TransactionException : public DBException {
public:
    explicit TransactionException(const std::string& message)
        : DBException("Transaction Error: " + message) {}
};

}  // namespace DB