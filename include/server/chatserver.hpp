#ifndef CHATSERVER_HPP
#define CHATSERVER_HPP
#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
using namespace muduo;
using namespace muduo::net;

class ChatServer
{
public:
    ChatServer(EventLoop *loop,
               const InetAddress &listenAddr,
               const string &nameArg);

    void start();

private:
    void onConnection(const TcpConnectionPtr &conn);
    void onMessage(const TcpConnectionPtr &conn,
                   Buffer *,
                   Timestamp);
    TcpServer server_;
    EventLoop *loop_; // 指向事件循环的指针
};

#endif