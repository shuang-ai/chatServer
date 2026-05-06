#include "db.h"
#include <string>

using namespace std;

// 数据库配置信息（放在 cpp 文件中，避免重复定义）
static string server = "127.0.0.1";
static string user = "root";
static string password = "123456";
static string dbname = "chat";

// 构造函数
MySQL::MySQL()
    : _conn(nullptr)
    , _connected(false)
{
    _conn = mysql_init(nullptr);
    if (_conn == nullptr) {
        LOG_ERROR("mysql_init failed!");
    }
}

// 析构函数
MySQL::~MySQL()
{
    if (_conn != nullptr) {
        mysql_close(_conn);
        _conn = nullptr;
    }
}

// 连接数据库
bool MySQL::connect()
{
    if (_conn == nullptr) {
        LOG_ERROR("MySQL object not initialized");
        return false;
    }
    
    MYSQL* p = mysql_real_connect(_conn, 
                                   server.c_str(), 
                                   user.c_str(),
                                   password.c_str(), 
                                   dbname.c_str(), 
                                   3306, 
                                   nullptr, 
                                   0);
    
    if (p != nullptr) {
        // 设置中文编码
        if (mysql_query(_conn, "set names gbk") != 0) {
            LOG_ERROR("Failed to set charset: %s", mysql_error(_conn));
        }
        _connected = true;
        LOG_INFO("MySQL connected successfully to database: %s", dbname.c_str());
        return true;
    } else {
        LOG_ERROR("MySQL connection failed: %s", mysql_error(_conn));
        _connected = false;
        return false;
    }
}

// 更新操作（增、删、改）
bool MySQL::update(const string& sql)
{
    if (_conn == nullptr || !_connected) {
        LOG_ERROR("MySQL not connected");
        return false;
    }
    
    if (mysql_query(_conn, sql.c_str()) != 0) {
        LOG_ERROR("Update failed: %s\nError: %s",
                  sql.c_str(),
                  mysql_error(_conn));
        return false;
    }
    
    LOG_DEBUG("Update success: %s", sql.c_str());
    return true;
}

// 查询操作
MYSQL_RES* MySQL::query(const string& sql)
{
    if (_conn == nullptr || !_connected) {
        LOG_ERROR("MySQL not connected");
        return nullptr;
    }
    
    if (mysql_query(_conn, sql.c_str()) != 0) {
        LOG_ERROR("Query failed: %s\nError: %s",
                  sql.c_str(),
                  mysql_error(_conn));
        return nullptr;
    }
    
    MYSQL_RES* result = mysql_use_result(_conn);
    if (result == nullptr) {
        LOG_ERROR("mysql_use_result failed: %s", mysql_error(_conn));
    }
    
    return result;
}

// 获取连接状态
bool MySQL::isConnected() const
{
    return _connected && (_conn != nullptr);
}

MYSQL* MySQL::getConnection(){
    return _conn;
}