#ifndef CHATSERVICE_H
#define CHATSERVICE_H
#include"groupModel.hpp"
#include"UserModel.hpp"
#include "friendModel.hpp"
#include"offlinemessagemodel.hpp"
#include <mymuduo/TcpServer.h>
#include<functional>
#include<unordered_map>
#include<string>
#include<mutex>
#include"redis.hpp"
#include"UserModel.hpp"
using namespace std;
#include"json.hpp"
using json = nlohmann::json;

// 获取消息对应的处理器
using MsgHander = std::function<void(const TcpConnectionPtr& conn, json& js, Timestamp)>;

// 聊天服务器业务类
class ChatService {
public:
    // 获取单例对象的接口函数
    static ChatService* instance();
    ~ChatService() = default;
    // 注册
    void login(const TcpConnectionPtr& conn, json& js, Timestamp time);
    // 登录
    void reg(const TcpConnectionPtr& conn, json& js, Timestamp time);

    // 一对一聊天
    void oneChat(const TcpConnectionPtr& conn, json& js, Timestamp time);

    // 添加好友
    void addFriend(const TcpConnectionPtr& conn, json& js, Timestamp time);

    // 获取消息对应的处理方法
    MsgHander getHandler(int msgid);

    // 处理客户端异常退出
    void clientCloseException(const TcpConnectionPtr &conn);
    
    // 创建群组
    void createGroup(const TcpConnectionPtr& conn, json& js, Timestamp time);

    // 加入群组
    void addGroup(const TcpConnectionPtr& conn, json& js, Timestamp time);

    // 群组聊天
    void groupChat(const TcpConnectionPtr& conn, json& js, Timestamp time);

    // 登出操作
    void loginout(const TcpConnectionPtr &conn, json &js, Timestamp time);

    // 服务器异常，业务重置方法
    void reset();

    // 从redis消息队列中获取订阅的消息
    void handleRedisSubscribeMessage(int userid, string msg);

private:
    ChatService();

    // 存储消息id和其对应的业务处理方法
    unordered_map<int, MsgHander> _msgHandlerMap;

    // 存储在线用户的通信连接
    unordered_map<int, TcpConnectionPtr> _userConnMap;

    // 互斥锁，保证_userConnMap的线程安全
    mutex _connMutex;

    /*
        数据库操作类对象
    */
    // 数据操作类对象
    UserModel _userModel;

    // 离线消息操作类对象
    OfflineMsgModel _offlineMsgModel;

    // 好友操作类对象
    FriendModel _friendModel;

    // 群组操作类对象
    GroupModel _groupModel;

    // redis操作类对象
    Redis _redis;

    

};
#endif