#include "DBConnection.h"
#include <sstream>
#include <iomanip>

namespace DB {

DBConnection::DBConnection(const DBConfig& config)
    : mysql_(nullptr), config_(config), connected_(false), 
      last_activity_(std::chrono::system_clock::now()) {
    mysql_ = mysql_init(nullptr);
    if (!mysql_) {
        throw ConnectionException("Failed to initialize MySQL connection");
    }
    
    setup_connection_options();
}

DBConnection::~DBConnection() {
    try {
        disconnect();
    } catch (...) {
        // Ignore errors during destruction
    }
    
    if (mysql_) {
        mysql_close(mysql_);
        mysql_ = nullptr;
    }
}

DBConnection::DBConnection(DBConnection&& other) noexcept
    : mysql_(other.mysql_), config_(std::move(other.config_)), 
      connected_(other.connected_), last_activity_(other.last_activity_) {
    other.mysql_ = nullptr;
    other.connected_ = false;
}

DBConnection& DBConnection::operator=(DBConnection&& other) noexcept {
    if (this != &other) {
        disconnect();
        if (mysql_) {
            mysql_close(mysql_);
        }
        
        mysql_ = other.mysql_;
        config_ = std::move(other.config_);
        connected_ = other.connected_;
        last_activity_ = other.last_activity_;
        
        other.mysql_ = nullptr;
        other.connected_ = false;
    }
    return *this;
}

void DBConnection::setup_connection_options() {
    if (!mysql_) return;
    
    // Set connection timeout
    unsigned int timeout = config_.connection_timeout;
    mysql_options(mysql_, MYSQL_OPT_CONNECT_TIMEOUT, &timeout);
    
    // Set read/write timeouts
    mysql_options(mysql_, MYSQL_OPT_READ_TIMEOUT, &config_.read_timeout);
    mysql_options(mysql_, MYSQL_OPT_WRITE_TIMEOUT, &config_.write_timeout);
    
    // Auto-reconnect
    my_bool reconnect = config_.auto_reconnect ? 1 : 0;
    mysql_options(mysql_, MYSQL_OPT_RECONNECT, &reconnect);
    
    // Character set
    mysql_options(mysql_, MYSQL_SET_CHARSET_NAME, config_.charset.c_str());
    
    // SSL options
    if (config_.use_ssl) {
        mysql_ssl_set(mysql_, 
            config_.ssl_key_path.empty() ? nullptr : config_.ssl_key_path.c_str(),
            config_.ssl_cert_path.empty() ? nullptr : config_.ssl_cert_path.c_str(),
            config_.ssl_ca_path.empty() ? nullptr : config_.ssl_ca_path.c_str(),
            nullptr, nullptr);
    }
}

void DBConnection::connect() {
    if (connected_) {
        return;
    }
    
    if (!config_.is_valid()) {
        throw ConnectionException("Invalid database configuration");
    }
    
    MYSQL* result = mysql_real_connect(
        mysql_,
        config_.host.c_str(),
        config_.user.c_str(),
        config_.password.empty() ? nullptr : config_.password.c_str(),
        config_.database.c_str(),
        config_.port,
        nullptr,
        0
    );
    
    if (!result) {
        throw ConnectionException(format_mysql_error("connect"));
    }
    
    connected_ = true;
    update_activity_time();
}

void DBConnection::disconnect() {
    if (connected_ && mysql_) {
        mysql_close(mysql_);
        mysql_ = mysql_init(nullptr);
        setup_connection_options();
        connected_ = false;
    }
}

bool DBConnection::is_connected() const {
    return connected_ && mysql_ != nullptr;
}

bool DBConnection::is_valid() const {
    if (!is_connected()) {
        return false;
    }
    
    try {
        ping();
        return true;
    } catch (...) {
        return false;
    }
}

void DBConnection::ping() const {
    check_connection();
    
    if (mysql_ping(mysql_) != 0) {
        const_cast<DBConnection*>(this)->connected_ = false;
        throw ConnectionException(format_mysql_error("ping"));
    }
    
    const_cast<DBConnection*>(this)->update_activity_time();
}

void DBConnection::reset() {
    disconnect();
    connect();
}

QueryResult DBConnection::execute_query(const std::string& query) {
    check_connection();
    update_activity_time();
    
    if (mysql_real_query(mysql_, query.c_str(), static_cast<unsigned long>(query.length())) != 0) {
        throw QueryException(format_mysql_error("execute_query"));
    }
    
    MYSQL_RES* result = mysql_store_result(mysql_);
    if (!result && mysql_field_count(mysql_) > 0) {
        throw QueryException(format_mysql_error("store_result"));
    }
    
    return QueryResult(result);
}

uint64_t DBConnection::execute_update(const std::string& query) {
    check_connection();
    update_activity_time();
    
    if (mysql_real_query(mysql_, query.c_str(), static_cast<unsigned long>(query.length())) != 0) {
        throw QueryException(format_mysql_error("execute_update"));
    }
    
    return mysql_affected_rows(mysql_);
}

uint64_t DBConnection::execute_insert(const std::string& query) {
    uint64_t affected = execute_update(query);
    return get_last_insert_id();
}

void DBConnection::execute(const std::string& query) {
    check_connection();
    update_activity_time();
    
    if (mysql_real_query(mysql_, query.c_str(), static_cast<unsigned long>(query.length())) != 0) {
        throw QueryException(format_mysql_error("execute"));
    }
    
    // Free any result set that might have been generated
    MYSQL_RES* result = mysql_store_result(mysql_);
    if (result) {
        mysql_free_result(result);
    }
}

std::unique_ptr<DBConnection::PreparedStatement> DBConnection::prepare(const std::string& query) {
    check_connection();
    return std::make_unique<PreparedStatement>(mysql_, query);
}

std::unique_ptr<DBConnection::Transaction> DBConnection::begin_transaction() {
    return std::make_unique<Transaction>(*this);
}

void DBConnection::commit() {
    check_connection();
    
    if (mysql_commit(mysql_) != 0) {
        throw TransactionException(format_mysql_error("commit"));
    }
    
    update_activity_time();
}

void DBConnection::rollback() {
    check_connection();
    
    if (mysql_rollback(mysql_) != 0) {
        throw TransactionException(format_mysql_error("rollback"));
    }
    
    update_activity_time();
}

void DBConnection::set_autocommit(bool enable) {
    check_connection();
    
    if (mysql_autocommit(mysql_, enable ? 1 : 0) != 0) {
        throw DBException(format_mysql_error("set_autocommit"));
    }
    
    update_activity_time();
}

std::string DBConnection::escape_string(const std::string& str) {
    check_connection();
    
    std::string escaped;
    escaped.resize(str.length() * 2 + 1);
    
    unsigned long escaped_length = mysql_real_escape_string(
        mysql_, 
        &escaped[0], 
        str.c_str(), 
        static_cast<unsigned long>(str.length())
    );
    
    escaped.resize(escaped_length);
    return escaped;
}

uint64_t DBConnection::get_last_insert_id() const {
    return mysql_insert_id(mysql_);
}

uint64_t DBConnection::get_affected_rows() const {
    return mysql_affected_rows(mysql_);
}

std::string DBConnection::get_last_error() const {
    return mysql_error(mysql_);
}

int DBConnection::get_last_error_code() const {
    return mysql_errno(mysql_);
}

std::string DBConnection::get_server_info() const {
    check_connection();
    return mysql_get_server_info(mysql_);
}

std::string DBConnection::get_client_info() const {
    return mysql_get_client_info();
}

void DBConnection::check_connection() const {
    if (!is_connected()) {
        throw ConnectionException("Database connection is not established");
    }
}

void DBConnection::update_activity_time() const {
    const_cast<DBConnection*>(this)->last_activity_ = std::chrono::system_clock::now();
}

std::string DBConnection::format_mysql_error(const std::string& operation) const {
    std::ostringstream oss;
    oss << operation << " failed: [" << mysql_errno(mysql_) << "] " << mysql_error(mysql_);
    return oss.str();
}

// PreparedStatement implementation
DBConnection::PreparedStatement::PreparedStatement(MYSQL* connection, const std::string& query)
    : stmt_(mysql_stmt_init(connection)), prepared_(false) {
    if (!stmt_) {
        throw QueryException("Failed to initialize prepared statement");
    }
    
    if (mysql_stmt_prepare(stmt_, query.c_str(), static_cast<unsigned long>(query.length())) != 0) {
        std::string error = "Failed to prepare statement: ";
        error += mysql_stmt_error(stmt_);
        mysql_stmt_close(stmt_);
        throw QueryException(error);
    }
    
    prepared_ = true;
    
    // Initialize bind arrays
    unsigned long param_count = mysql_stmt_param_count(stmt_);
    if (param_count > 0) {
        binds_.resize(param_count);
        string_buffers_.resize(param_count);
        numeric_buffers_.resize(param_count);
        null_indicators_.resize(param_count);
        
        // Initialize bind structures
        memset(binds_.data(), 0, binds_.size() * sizeof(MYSQL_BIND));
    }
}

DBConnection::PreparedStatement::~PreparedStatement() {
    if (stmt_) {
        mysql_stmt_close(stmt_);
    }
}

DBConnection::PreparedStatement::PreparedStatement(PreparedStatement&& other) noexcept
    : stmt_(other.stmt_), binds_(std::move(other.binds_)),
      string_buffers_(std::move(other.string_buffers_)),
      numeric_buffers_(std::move(other.numeric_buffers_)),
      null_indicators_(std::move(other.null_indicators_)),
      prepared_(other.prepared_) {
    other.stmt_ = nullptr;
    other.prepared_ = false;
}

DBConnection::PreparedStatement& DBConnection::PreparedStatement::operator=(PreparedStatement&& other) noexcept {
    if (this != &other) {
        if (stmt_) {
            mysql_stmt_close(stmt_);
        }
        
        stmt_ = other.stmt_;
        binds_ = std::move(other.binds_);
        string_buffers_ = std::move(other.string_buffers_);
        numeric_buffers_ = std::move(other.numeric_buffers_);
        null_indicators_ = std::move(other.null_indicators_);
        prepared_ = other.prepared_;
        
        other.stmt_ = nullptr;
        other.prepared_ = false;
    }
    return *this;
}

void DBConnection::PreparedStatement::bind_string(int index, const std::string& value) {
    ensure_bind_capacity(index);
    
    string_buffers_[index] = value;
    binds_[index].buffer_type = MYSQL_TYPE_STRING;
    binds_[index].buffer = const_cast<char*>(string_buffers_[index].c_str());
    binds_[index].buffer_length = static_cast<unsigned long>(string_buffers_[index].length());
    binds_[index].is_null = &null_indicators_[index];
    null_indicators_[index] = 0;
}

void DBConnection::PreparedStatement::bind_int(int index, int value) {
    ensure_bind_capacity(index);
    
    numeric_buffers_[index] = value;
    binds_[index].buffer_type = MYSQL_TYPE_LONG;
    binds_[index].buffer = &numeric_buffers_[index];
    binds_[index].is_null = &null_indicators_[index];
    null_indicators_[index] = 0;
}

void DBConnection::PreparedStatement::bind_long(int index, long long value) {
    ensure_bind_capacity(index);
    
    numeric_buffers_[index] = value;
    binds_[index].buffer_type = MYSQL_TYPE_LONGLONG;
    binds_[index].buffer = &numeric_buffers_[index];
    binds_[index].is_null = &null_indicators_[index];
    null_indicators_[index] = 0;
}

void DBConnection::PreparedStatement::bind_double(int index, double value) {
    ensure_bind_capacity(index);
    
    // Store double in a separate location since numeric_buffers_ is for long long
    static thread_local std::vector<double> double_buffers;
    if (double_buffers.size() <= static_cast<size_t>(index)) {
        double_buffers.resize(index + 1);
    }
    
    double_buffers[index] = value;
    binds_[index].buffer_type = MYSQL_TYPE_DOUBLE;
    binds_[index].buffer = &double_buffers[index];
    binds_[index].is_null = &null_indicators_[index];
    null_indicators_[index] = 0;
}

void DBConnection::PreparedStatement::bind_null(int index) {
    ensure_bind_capacity(index);
    
    binds_[index].buffer_type = MYSQL_TYPE_NULL;
    binds_[index].buffer = nullptr;
    binds_[index].is_null = &null_indicators_[index];
    null_indicators_[index] = 1;
}

void DBConnection::PreparedStatement::bind_datetime(int index, const std::chrono::system_clock::time_point& time) {
    ensure_bind_capacity(index);
    
    auto time_t_val = std::chrono::system_clock::to_time_t(time);
    std::tm tm_val;
    localtime_s(&tm_val, &time_t_val);
    
    // Format as MySQL datetime string
    char buffer[32];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &tm_val);
    
    bind_string(index, std::string(buffer));
    binds_[index].buffer_type = MYSQL_TYPE_DATETIME;
}

QueryResult DBConnection::PreparedStatement::execute_query() {
    if (!prepared_) {
        throw QueryException("Statement is not prepared");
    }
    
    if (!binds_.empty()) {
        if (mysql_stmt_bind_param(stmt_, binds_.data()) != 0) {
            throw QueryException("Failed to bind parameters: " + std::string(mysql_stmt_error(stmt_)));
        }
    }
    
    if (mysql_stmt_execute(stmt_) != 0) {
        throw QueryException("Failed to execute prepared statement: " + std::string(mysql_stmt_error(stmt_)));
    }
    
    MYSQL_RES* result = mysql_stmt_result_metadata(stmt_);
    if (result) {
        mysql_free_result(result);
        // For queries that return result sets, we would need more complex handling
        // This is a simplified version for demonstration
    }
    
    return QueryResult(result);
}

uint64_t DBConnection::PreparedStatement::execute_update() {
    if (!prepared_) {
        throw QueryException("Statement is not prepared");
    }
    
    if (!binds_.empty()) {
        if (mysql_stmt_bind_param(stmt_, binds_.data()) != 0) {
            throw QueryException("Failed to bind parameters: " + std::string(mysql_stmt_error(stmt_)));
        }
    }
    
    if (mysql_stmt_execute(stmt_) != 0) {
        throw QueryException("Failed to execute prepared statement: " + std::string(mysql_stmt_error(stmt_)));
    }
    
    return mysql_stmt_affected_rows(stmt_);
}

void DBConnection::PreparedStatement::execute() {
    execute_update();
}

void DBConnection::PreparedStatement::clear_bindings() {
    if (!binds_.empty()) {
        memset(binds_.data(), 0, binds_.size() * sizeof(MYSQL_BIND));
    }
    
    std::fill(string_buffers_.begin(), string_buffers_.end(), std::string());
    std::fill(numeric_buffers_.begin(), numeric_buffers_.end(), 0);
    std::fill(null_indicators_.begin(), null_indicators_.end(), 0);
}

void DBConnection::PreparedStatement::ensure_bind_capacity(int index) {
    if (static_cast<size_t>(index) >= binds_.size()) {
        throw QueryException("Parameter index out of range: " + std::to_string(index));
    }
}

void DBConnection::PreparedStatement::reset_statement() {
    if (mysql_stmt_reset(stmt_) != 0) {
        throw QueryException("Failed to reset statement: " + std::string(mysql_stmt_error(stmt_)));
    }
}

// Transaction implementation
DBConnection::Transaction::Transaction(DBConnection& connection)
    : connection_(connection), active_(true), committed_(false) {
    connection_.set_autocommit(false);
}

DBConnection::Transaction::~Transaction() {
    if (active_ && !committed_) {
        try {
            rollback();
        } catch (...) {
            // Ignore rollback errors during destruction
        }
    }
    
    try {
        connection_.set_autocommit(true);
    } catch (...) {
        // Ignore autocommit reset errors
    }
}

void DBConnection::Transaction::commit() {
    if (!active_) {
        throw TransactionException("Transaction is not active");
    }
    
    connection_.commit();
    committed_ = true;
    active_ = false;
}

void DBConnection::Transaction::rollback() {
    if (!active_) {
        throw TransactionException("Transaction is not active");
    }
    
    connection_.rollback();
    active_ = false;
}

}  // namespace DB