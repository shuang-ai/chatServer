#include "chatservice.hpp"
#include "public.hpp"
#include <mymuduo/Logger.h>
using namespace std;
using namespace placeholders;
// using namespace muduo;
// 回调操作
ChatService::ChatService()
{
    // 登录
    _msgHandlerMap[LOGIN_MSG] = std::bind(&ChatService::login, this, _1, _2, _3);
    // 注册
    _msgHandlerMap[REG_MSG] = std::bind(&ChatService::reg, this, _1, _2, _3);
    // 一对一聊天
    _msgHandlerMap[ONE_CHAT_MSG] = std::bind(&ChatService::oneChat, this, _1, _2, _3);
    // 增加好友
    _msgHandlerMap[ADD_FRIEND_MSG] = std::bind(&ChatService::addFriend, this, _1, _2, _3);
    // 创建群组
    _msgHandlerMap[CREATE_GROUP_MSG] = std::bind(&ChatService::createGroup, this, _1, _2, _3);
    // 加入一个存在的群组
    _msgHandlerMap[ADD_GROUP_MSG] = std::bind(&ChatService::addGroup, this, _1, _2, _3);
    // 群里聊天
    _msgHandlerMap[GROUP_CHAT_MSG] = std::bind(&ChatService::groupChat, this, _1, _2, _3);
    // 登出
    _msgHandlerMap[LOGINOUT_MSG] = std::bind(&ChatService::loginout, this, _1, _2, _3);

    if (_redis.connect())
    {
        // 设置上报消息的回调
        _redis.init_notify_handler(std::bind(&ChatService::handleRedisSubscribeMessage, this, _1, _2));
    }
}
// 登录
void ChatService::login(const TcpConnectionPtr &conn, json &js, Timestamp time){
    // 修改：使用用户名登录，而不是ID
    // 期望的 JSON 格式：
    // {
    //     "msgid": 1,
    //     "name": "zhangsan",      // ← 用户名
    //     "password": "123456"
    // }
    
    // 1. 检查必要字段是否存在
    if (!js.contains("id") || !js.contains("password"))
    {
        json response;
        response["msgid"] = LOGIN_MSG_ACK;
        response["errno"] = 400;
        response["errmsg"] = "Missing required fields: name and password";
        conn->send(response.dump());
        return;
    }
    
    // 2. 获取用户名和密码
    int id = js["id"].get<int>();
    string pwd = js["password"].get<string>();
    
    // 3. 根据用户id查询用户
    User user = _userModel.query(id);
    
    // 4. 验证用户是否存在
    if (user.getId() == -1)
    {
        // 用户不存在
        json response;
        response["msgid"] = LOGIN_MSG_ACK;
        response["errno"] = 1;
        response["errmsg"] = "用户不存在";
        conn->send(response.dump());
        return;
    }
    
    // 5. 验证密码
    if (user.getPassword() != pwd)
    {
        // 密码错误
        json response;
        response["msgid"] = LOGIN_MSG_ACK;
        response["errno"] = 1;
        response["errmsg"] = "用户名或者密码错误";
        conn->send(response.dump());
        return;
    }
    
    // 6. 检查用户是否已经登录
    if (user.getState() == "online")
    {
        json response;
        response["msgid"] = LOGIN_MSG_ACK;
        response["errno"] = 2;
        response["errmsg"] = "该用户已登录,请重新输入账号";
        conn->send(response.dump());
        return;
    }
    
    // 7. 登录成功
    {
        lock_guard<mutex> lock(_connMutex);
        // 记录用户连接信息
        _userConnMap.insert({user.getId(), conn});
    }
    
    // 8. 订阅 Redis 消息（用于跨服务器通信）
    _redis.subscribe(user.getId());
    
    // 9. 更新用户状态为在线
    user.setState("online");
    _userModel.updateState(user);
    
    // 10. 构造登录成功响应
    json response;
    response["msgid"] = LOGIN_MSG_ACK;
    response["errno"] = 0;
    response["id"] = user.getId();
    response["name"] = user.getName();
    
    // 11. 获取离线消息
    vector<string> vec = _offlineMsgModel.query(user.getId());
    if (!vec.empty())
    {
        response["offlinemsg"] = vec;
        _offlineMsgModel.remove(user.getId());
    }
    
    // 12. 获取好友列表
    vector<User> userVec = _friendModel.query(user.getId());
    if (!userVec.empty())
    {
        vector<string> vec2;
        for (auto &friendUser : userVec)
        {
            json tmpjs;
            tmpjs["id"] = friendUser.getId();
            tmpjs["name"] = friendUser.getName();
            tmpjs["state"] = friendUser.getState();
            vec2.push_back(tmpjs.dump());
        }
        response["friends"] = vec2;
    }
    
    // 13. 获取群组列表
    vector<Group> groupVec = _groupModel.queryGroups(user.getId());
    if (!groupVec.empty())
    {
        vector<string> vec3;
        for (auto &group : groupVec)
        {
            json tmpjs;
            tmpjs["id"] = group.getId();
            tmpjs["groupname"] = group.getName();
            tmpjs["groupdesc"] = group.getDesc();
            
            vector<string> userVec2;
            for (auto &groupUser : group.getUsers())
            {
                json tmpjs2;
                tmpjs2["id"] = groupUser.getId();
                tmpjs2["name"] = groupUser.getName();
                tmpjs2["state"] = groupUser.getState();
                tmpjs2["role"] = groupUser.getRole();
                userVec2.push_back(tmpjs2.dump());
            }
            tmpjs["users"] = userVec2;
            vec3.push_back(tmpjs.dump());
        }
        response["groups"] = vec3;
    }
    
    // 14. 发送响应
    conn->send(response.dump());

}


// 注册
void ChatService::reg(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    // cout<<"reg"<<endl;
    string name = js["name"];
    string password = js["password"];
    User user;
    user.setName(name);
    user.setPassword(password);
    bool state = _userModel.insert(user);
    if (state)
    {
        // 注册成功
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 0;
        response["id"] = user.getId();
        // 所以必须用 .dump() 转成字符串才能发送
        conn->send(response.dump());
    }
    else
    {
        // 注册失败
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 1;
        conn->send(response.dump());
    }
}

MsgHander ChatService::getHandler(int msgid)
{
    auto it = _msgHandlerMap.find(msgid);
    if (it == _msgHandlerMap.end())
    {
        // 默认
        return [this, msgid](const TcpConnectionPtr &conn, json &js, Timestamp time)
        {
            LOG_ERROR("msgid:can not find handler!");
        };
    }
    else
    {
        return _msgHandlerMap[msgid];
    }
}

ChatService *ChatService::instance()
{
    static ChatService service;
    return &service;
}

void ChatService::clientCloseException(const TcpConnectionPtr &conn)
{
    // user 局部变量	❌ 只有当前线程能看到	❌ 不需要加锁
    User user;
    {
        lock_guard<mutex> lock(_connMutex);
        for (auto it = _userConnMap.begin(); it != _userConnMap.end(); ++it)
        {
            if (it->second == conn)
            {
                // 删除用户连接信息
                user.setId(it->first);
                _userConnMap.erase(it);
                break;
            }
        }
    }

    if (user.getId() != -1)
    {
        // 更新用户状态信息,改为下线
        user.setState("offline");
        // 更新表的状态
        _userModel.updateState(user);
    }
}

void ChatService::oneChat(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    // msgid
    // id:1
    // from:"zhangsan"
    // to:3
    // msg:"xxx"
    int toId = js["toid"].get<int>();
    {
        lock_guard<mutex> lock(_connMutex);
        auto it = _userConnMap.find(toId);
        if (it != _userConnMap.end())
        {
            // toId 在线
            // it->second 是目标用户的连接，所以数据发给目标用户
            it->second->send(js.dump());
            return;
        }
    }

    // 查询todi是否在线
    User user = _userModel.query(toId);
    if (user.getState() == "online")
    {
        // toId 在线，但是不在同一台服务器上
        _redis.publish(toId, js.dump());
        return;
    }

    // 如果toid离线，则存储离线消息
    _offlineMsgModel.insert(toId, js.dump());
}

void ChatService::reset()
{
    // 重置用户状态
    _userModel.resetState();
}

// 添加好友,msgid ,id ,friendid
void ChatService::addFriend(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    int friendid = js["friendid"].get<int>();
    _friendModel.insert(userid, friendid);
}

void ChatService::createGroup(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    string name = js["groupname"];
    string desc = js["groupdesc"];
    Group group(-1, name, desc);

    if (_groupModel.createGroup(group))
    {
        _groupModel.addGroup(userid, group.getId(), "creator");
    }
}

// 加入群组
void ChatService::addGroup(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    _groupModel.addGroup(userid, groupid, "normal");
}

void ChatService::groupChat(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    vector<int> useridVec = _groupModel.queryGroupUsers(userid, groupid);
    for (int id : useridVec)
    {
        // 注意！！！map不是线程安全的，所以应该加锁
        {
            lock_guard<mutex> lock(_connMutex);
            // 找到这个用户的连接id
            auto it = _userConnMap.find(id);
            // 注意在线就直接发送，不在线就发离线群消息
            if (it != _userConnMap.end())
            {
                // 转发给群组其它用户
                it->second->send(js.dump());
            }
            else
            {
                // 查询id是否在线
                User user = _userModel.query(id);
                if (user.getState() == "online")
                {
                    // 说明此时在不同服务器上
                    _redis.publish(id, js.dump());
                }
                else
                {
                    // 存储离线群消息
                    _offlineMsgModel.insert(id, js.dump());
                }
            }
        }
    }
}

void ChatService::loginout(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    // 擦除掉在连接表中的连接
    {
        lock_guard<mutex> lock(_connMutex);
        auto it = _userConnMap.find(userid);
        if (it != _userConnMap.end())
        {
            _userConnMap.erase(it);
        }
    }

    // 用户注销，在redis中取消订阅
    _redis.unsubscribe(userid);

    // 更新用户的状态信息
    User user(userid, "", "", "offline");
    _userModel.updateState(user);
    // conn->shutdown();
}

// 从redis消息队列中获取订阅的消息
void ChatService::handleRedisSubscribeMessage(int userid, string msg)
{
    lock_guard<mutex> lock(_connMutex);
    auto it = _userConnMap.find(userid);
    if (it != _userConnMap.end())
    {
        it->second->send(msg);
        return;
    }

    // 存储该用户的离线消息
    _offlineMsgModel.insert(userid, msg);
}