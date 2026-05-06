#include"UserModel.hpp"
#include"db.h"
bool UserModel::insert(User &user){
    char sql[1024] = {0};
    sprintf(sql,"insert into user(name,password,state) values('%s','%s','%s')",
        user.getName().c_str(),user.getPassword().c_str(),user.getState().c_str());

        MySQL mysql;
        if(mysql.connect()){
            if(mysql.update(sql)){
                // mysql_insert_id 是 MySQL C API 中的一个函数，
                // 用于获取上一次 INSERT 操作自动生成的 ID（通常是 AUTO_INCREMENT 字段的值）。
                user.setId(mysql_insert_id(mysql.getConnection()));
                return true;
            }
        }

    return false;
}

User UserModel::query(int id){
    // 构建SQL查询语句，从user表中根据id查询用户信息
    char sql[1024] = {0};
    sprintf(sql,"select * from user where id = %d",id);

    MySQL mysql;  // 创建MySQL数据库连接对象
    if(mysql.connect()){  // 尝试连接数据库，成功则进入
        // 执行SQL查询，返回结果集
        MYSQL_RES *result = mysql.query(sql);
        if(result != nullptr){  // 查询成功，结果集不为空
            // 从结果集中获取一行数据
            MYSQL_ROW row = mysql_fetch_row(result);
            if(row != nullptr){  // 成功获取到一行数据
                User user;  // 创建User对象
                // TODO: 将row中的数据填充到user对象中
                // 例如: user.setId(atoi(row[0]));
                //      user.setName(row[1]);
                //      user.setPassword(row[2]);
                //      ...
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setPassword(row[2]);  
                user.setState(row[3]);
                // 释放由 mysql_store_result() 或 mysql_use_result() 分配的结果集内存。
                // mysql_free_result 释放的是 MYSQL_RES *result 指向的结果集内存，不是连接资源！
                // 只要是返回 MYSQL_RES* 的函数，并且返回值不为 NULL，
                // 就必须用 mysql_free_result() 释放。
                mysql_free_result(result);
                return user;  // 返回查询到的用户对象
            }
        }
    }
    // 查询失败或未找到用户，返回空的User对象
    return User();  // 或抛出异常
}

bool UserModel::updateState(User user){
    char sql[1024] = {0};
    sprintf(sql,"update user set state='%s' where id=%d",
        user.getState().c_str(),user.getId());

    MySQL mysql;
    if(mysql.connect()){
        if(mysql.update(sql)){
            return true;
        }
    }
    return false;
}

void UserModel::resetState(){
    char sql[1024] = "update user set state='offline' where state='online'";
    MySQL mysql;
    if(mysql.connect()){
        mysql.update(sql);
    }
}

// ========== 新增：根据用户名查询用户信息 ==========
User UserModel::queryByName(const string& name)
{
    // 构建SQL查询语句，从user表中根据name查询用户信息
    char sql[1024] = {0};
    sprintf(sql, "select * from user where name = '%s'", name.c_str());

    MySQL mysql;
    if (mysql.connect())
    {
        MYSQL_RES *result = mysql.query(sql);
        if (result != nullptr)
        {
            MYSQL_ROW row = mysql_fetch_row(result);
            if (row != nullptr)
            {
                User user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setPassword(row[2]);
                user.setState(row[3]);
                mysql_free_result(result);
                return user;
            }
            mysql_free_result(result);
        }
    }
    
    // 查询失败或未找到用户，返回空的User对象（id = -1）
    return User();
}