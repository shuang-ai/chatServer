#include "chatserver.hpp"
#include <functional>
#include"json.hpp"
#include "chatservice.hpp"
#include <mymuduo/Logger.h>
#include<iostream>
#define ERROR_MSG 999
using json=nlohmann::json;


using namespace std;

ChatServer::ChatServer(EventLoop *loop,
                       const InetAddress &listenAddr,
                       const string &nameArg) : server_(loop, listenAddr, nameArg), loop_(loop)
{
    server_.setConnectionCallback([this](const TcpConnectionPtr &conn)
                                  { this->onConnection(conn); });

    server_.setMessageCallback([this](const TcpConnectionPtr &conn,
                                      Buffer *buffer,
                                      Timestamp time)
                               { this->onMessage(conn, buffer, time); });

    server_.setThreadNum(4);
}

void ChatServer::start()
{
    server_.start();
}

void ChatServer::onConnection(const TcpConnectionPtr &conn)
{
    if(!conn->connected()){
        ChatService::instance()->clientCloseException(conn);
        conn->shutdown();
    }
    else{
        std::cout<<"connection success"<<std::endl;
    }

}
/**
 * @brief 当 TCP 连接上有消息到达时的回调函数
 * @param conn  TCP 连接对象（表示与客户端的连接）
 * @param buffer 接收到的数据缓冲区
 * @param time 消息到达的时间戳
 * 
 * 这个函数在 muduo 网络库的 EventLoop 线程中被调用，
 * 负责将网络层接收到的数据转发给业务层处理。
 */
void ChatServer::onMessage(const TcpConnectionPtr &conn,
                           Buffer* buffer,
                           Timestamp time)
{
    // 1. 从缓冲区中取出所有数据（作为完整的消息）
    // retrieveAllAsString() 会返回 buffer 中的所有数据，并清空 buffer
    string buf = buffer->retrieveAllAsString();
    
    // 2. 添加日志，记录收到的原始消息（便于调试）
    // 注意：生产环境可能需要脱敏处理
    LOG_INFO("Received message from %s: %s",
             conn->peerAddress().toIpPort().c_str(),
             buf);
    
    // 3. 使用 try-catch 捕获 JSON 处理过程中可能抛出的所有异常
    //    防止程序因异常而崩溃
    try {
        // 3.1 解析 JSON 字符串
        // json::parse() 是 nlohmann/json 库提供的函数
        // 如果 buf 不是合法的 JSON 格式，会抛出 parse_error 异常
        json js = json::parse(buf);
        
        // 3.2 检查 msgid 字段是否存在
        // contains() 检查 JSON 对象是否包含指定的键
        if (!js.contains("msgid"))
        {
            LOG_ERROR("Invalid message: missing msgid field, raw: %s", buf);
            
            // 发送错误响应给客户端
            json response;
            response["msgid"] = ERROR_MSG;
            response["errno"] = 400;
            response["errmsg"] = "Missing msgid field";
            conn->send(response.dump());
            return;  // 消息格式错误，直接返回，不继续处理
        }
        
        // 3.3 检查 msgid 字段的类型是否为数字
        // is_number() 检查该字段的值是否是数字类型
        if (!js["msgid"].is_number())
        {
            LOG_ERROR("Invalid message: msgid is not a number, type: %s",
                      js["msgid"].type_name());
            
            // 发送错误响应给客户端
            json response;
            response["msgid"] = ERROR_MSG;
            response["errno"] = 400;
            response["errmsg"] = "msgid must be number";
            conn->send(response.dump());
            return;  // msgid 类型错误，直接返回
        }
        
        // 3.4 获取消息类型 ID
        // get<int>() 将 JSON 值转换为 int 类型
        // 如果类型不匹配（比如是字符串），会抛出 type_error 异常
        int msgid = js["msgid"].get<int>();
        
        // 3.5 记录消息类型
        LOG_DEBUG("Received msgid: %d", msgid);
        
        // 3.6 根据消息 ID 获取对应的业务处理器
        // ChatService 是单例模式，负责管理所有消息处理器
        // getHandler() 返回一个函数对象（std::function）
        auto msgHandler = ChatService::instance()->getHandler(msgid);
        
        // 3.7 检查处理器是否存在
        // 如果找不到对应的处理器，getHandler 可能返回 nullptr 或默认处理器
        if (!msgHandler)
        {
            LOG_ERROR("No handler found for msgid: %d", msgid);
            
            // 发送错误响应给客户端
            json response;
            response["msgid"] = ERROR_MSG;
            response["errno"] = 404;
            response["errmsg"] = "Handler not found";
            conn->send(response.dump());
            return;  // 没有注册该消息类型的处理器，无法处理
        }
        
        // 3.8 调用业务处理器执行具体的业务逻辑
        // 将连接对象、JSON 数据和消息时间传递给处理器
        // 处理器会完成具体的业务处理（如登录、聊天、添加好友等）
        msgHandler(conn, js, time);
    }
    // 4. 捕获 JSON 解析错误（JSON 格式不正确）
    //    parse_error 在 json::parse() 解析失败时抛出
    catch (const json::parse_error& e)
    {
        LOG_ERROR("JSON parse error: %s", e.what());
        LOG_ERROR("Raw message: %s", buf);
        
        // 发送错误响应给客户端，告知消息格式错误
        json response;
        response["msgid"] = ERROR_MSG;
        response["errno"] = 400;
        response["errmsg"] = "Invalid JSON format: " + std::string(e.what());
        conn->send(response.dump());
    }
    // 5. 捕获 JSON 类型错误（字段类型不匹配）
    //    type_error 在 get<int>() 等类型转换失败时抛出
    catch (const json::type_error& e)
    {
        LOG_ERROR("JSON type error: %s", e.what());
        LOG_ERROR("Message: %s", buf);
        
        // 发送错误响应给客户端
        json response;
        response["msgid"] = ERROR_MSG;
        response["errno"] = 400;
        response["errmsg"] = "Type error: " + std::string(e.what());
        conn->send(response.dump());
    }
    // 6. 捕获 JSON 越界错误（访问不存在的字段）
    //    out_of_range 在使用 at() 访问不存在的键时抛出
    catch (const json::out_of_range& e)
    {
        LOG_ERROR("JSON out of range: %s", e.what());
        LOG_ERROR("Message: %s", buf);
        
        // 发送错误响应给客户端
        json response;
        response["msgid"] = ERROR_MSG;
        response["errno"] = 400;
        response["errmsg"] = "Missing required field: " + std::string(e.what());
        conn->send(response.dump());
    }
    // 7. 捕获其他标准异常（兜底处理）
    //    确保所有异常都被捕获，程序不会崩溃
    catch (const std::exception& e)
    {
        LOG_ERROR("Unexpected exception: %s", e.what());
        LOG_ERROR("Message: %s", buf);
        
        // 发送通用错误响应
        json response;
        response["msgid"] = ERROR_MSG;
        response["errno"] = 500;
        response["errmsg"] = "Internal server error: " + std::string(e.what());
        conn->send(response.dump());
    }
    // 8. 捕获所有其他未知异常
    catch (...)
    {
        LOG_ERROR("Unknown exception caught!");
        LOG_ERROR("Message: %s", buf);
        
        // 发送通用错误响应
        json response;
        response["msgid"] = ERROR_MSG;
        response["errno"] = 500;
        response["errmsg"] = "Unknown error";
        conn->send(response.dump());
    }
}