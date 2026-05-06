#include"chatserver.hpp"
#include<iostream>
#include"chatservice.hpp"
#include<signal.h>
void resetHandler(int)
{
    ChatService::instance()->reset();
    exit(0);
}

int main(int argc,const char* argv[] ){

    if(argc<3)
    {
        std::cerr<<"command invalid! example: ./ChatServer 127.0.0.1 6000"<<std::endl;
        return -1;
    }
    const char* ip = argv[1];
    uint16_t port = atoi(argv[2]);
    signal(SIGINT,resetHandler);
    EventLoop loop;
    InetAddress addr(ip,port);
    ChatServer server(&loop,addr,"chatserver");

    server.start();
    loop.loop();

    return 0;
}