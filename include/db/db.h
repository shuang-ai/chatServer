#ifndef DB_H
#define DB_H

#include <mysql/mysql.h>
#include <string>
#include <muduo/base/Logging.h>

// 数据库操作类
class MySQL
{
public:
    // 初始化数据库连接
    MySQL();
    
    // 释放数据库连接资源
    ~MySQL();
    
    // 连接数据库
    bool connect();
    
    // 更新操作（增、删、改）
    bool update(const std::string& sql);
    
    // 查询操作
    MYSQL_RES* query(const std::string& sql);
    
    // 获取连接状态
    bool isConnected() const;

    MYSQL* getConnection();

private:
    MYSQL* _conn;
    bool _connected;  // 连接状态标志
};

#endif // DB_H