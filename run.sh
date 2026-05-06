#!/bin/bash
cd build
make
if [ $? -eq 0 ]; then
    echo "编译成功，启动服务器..."
    ../bin/ChatServer
else
    echo "编译失败"
fi