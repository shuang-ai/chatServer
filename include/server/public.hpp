#ifndef PUBLIC_HPP
#define PUBLIC_HPP

#include "server/chatserver.hpp"
#include "server/chatservice.hpp"
enum EnMsgType{
    // 登录消息
    LOGIN_MSG=1,
    // 登录响应消息
    LOGIN_MSG_ACK,
    // 注册消息
    REG_MSG,
    // 注册响应消息
    REG_MSG_ACK,
    // 一对一聊天消息
    ONE_CHAT_MSG,
    // 添加好友消息
    ADD_FRIEND_MSG,

    //创建群组消息
    CREATE_GROUP_MSG,
    // 加入群组消息
    ADD_GROUP_MSG,
    // 群组聊天消息
    GROUP_CHAT_MSG,
    // 登出消息
    LOGINOUT_MSG,
    
};

#endif