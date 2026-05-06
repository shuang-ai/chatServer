#include<iostream>
#include <mymuduo/TcpServer.h>
#include <mymuduo/Logger.h>
#include<iostream>
using namespace std;
/*
1.组合TcpServer对象
2.创建EventLoop事件循环对象的指针
3.明确TcpServer构造函数需要什么参数，输出ChatServer的构造函数
4.在当前服务器类的构造函数中，注册处理链接的回调函数(setConnectionCallback)和处理读写事件的回调函数(setMessageCallback)
5.设置合适的服务端线程数量，muduo库会自己分配I/O线程和worker线程
*/

class ChatServer{
public:
      ChatServer(EventLoop* loop,
            const InetAddress& listenAddr,
            const string& nameArg):server_(loop,listenAddr,nameArg),loop_(loop){
                // 给服务器注册用户连接和创建的断开回调
                // server_.setConnectionCallback(std::bind(&ChatServer::onConnection,this,placeholders::_1));
                server_.setConnectionCallback([this](const TcpConnectionPtr& conn){
                    this->onConnection(conn);
                });
                // 给服务器注册用户读写事件回调
                server_.setMessageCallback([this](const TcpConnectionPtr&conn, // 连接
                            Buffer*buf,         //缓冲区
                            Timestamp time){
                                this->onMessage(conn,buf,time);
                });

                // 设置服务端现成数量，1个I/O线程，3个worker线程
                server_.setThreadNum(4);

            }

            // 开启事件循环
            void start(){
                server_.start();
            }

private:
    // 专门处理用户的连接创建和断开
    void onConnection(const TcpConnectionPtr& conn){
//         conn->peerAddress()   // 对端地址（对方）
// conn->localAddress()  // 本地地址（自己）
        if(conn->connected()){
            cout<<conn->peerAddress().toIpPort()<<" -> "<<conn->localAddress().toIpPort()<<endl;
        }
        else{
            cout<<conn->peerAddress().toIpPort()<<" -> "<<conn->localAddress().toIpPort()<<endl;
            cout<<"对端已经下线"<<endl;
            conn->shutdown(); //close(fd)
        }
        
    }
    // 专门处理用户的读写事件
    void onMessage(const TcpConnectionPtr&conn, // 连接
                    Buffer*buffer,         //缓冲区
                    Timestamp time)
            {    // 接收消息的时间信息
                string buf = buffer->retrieveAllAsString();
                cout<<"recv data "<<buf<<" time "<<time.toString()<<endl;
                conn->send(buf);

            }
    TcpServer server_;
    EventLoop* loop_;
};

int main(){
    // epoll
    EventLoop loop;
    InetAddress addr(6000,"127.0.0.1");

    ChatServer server(&loop,addr,"ChatServer");
    // listenfd epoll_ctl=>epoll
    server.start();
    // loop.loop() 进入了事件循环，程序会在这里"卡住"，不断处理网络事件，直到循环被停止。
    loop.loop();    

    return 0;
}
