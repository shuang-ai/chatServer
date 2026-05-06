#ifndef USERMODEL_HPP
#define USERMODEL_HPP
#include "db.h"
#include"user.hpp"
class UserModel
{
public:
    // 注册新用户
    bool insert(User &user);

    // 根据用户号码查询用户信息
    User query(int id);

     // ========== 新增：根据用户名查询用户信息 ==========
    User queryByName(const std::string& name);

    // 更新用户状态信息
    bool updateState(User user);

    void resetState();
private:
};

#endif