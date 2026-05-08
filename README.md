```markdown
# ChatServer - 集群聊天服务器

本项目是一个**高并发、可扩展的集群聊天服务器**，基于自研的 `my_muduo` 网络库实现。  
支持跨服务器消息分发、Nginx TCP 负载均衡、Redis 发布/订阅、MySQL 数据持久化。

**自研 my_muduo 网络库地址**：  
https://github.com/shuang-ai/muduo.git

---

## 整体架构

- **网络层**：自研 `my_muduo`（Reactor + epoll + 多线程模型）  
- **协议**：JSON 私有通信协议（序列化/反序列化）  
- **负载均衡**：Nginx TCP 负载均衡（集群部署）  
- **跨服通信**：Redis 发布/订阅  
- **数据存储**：MySQL（用户/离线消息/好友关系等）

---

## 快速开始

### 1. 编译自研 my_muduo 库

```bash
git clone https://github.com/shuang-ai/muduo.git
cd muduo
chmod +x autobuild.sh
./autobuild.sh
```

执行后会在 `muduo/lib` 目录下生成静态库/动态库。

### 2. 导入数据库

```bash
cd chatServer/database
mysql -u root -p
source chat.sql
```

脚本会自动创建数据库 `ChatServer` 及相关表（用户、好友、群组、离线消息等）。

### 3. 编译本项目

```bash
cd chatServer
chmod +x run.sh
./run.sh
```

执行后会在 `bin/` 目录下生成可执行文件：`ChatServer` 和 `ChatClient`。

### 4. 运行服务端

```bash
./bin/ChatServer <IP> <Port>
```

例如：

```bash
./bin/ChatServer 0.0.0.0 8888
```

### 5. 运行客户端

```bash
./bin/ChatClient <IP> <Port>
```

例如：

```bash
./bin/ChatClient 127.0.0.1 8888
```

---

## 项目结构

```
chatServer/
├── bin/                   # 编译生成的可执行文件
├── database/
│   └── chat.sql           # MySQL 建表及初始化脚本
├── include/               # 头文件
├── test/                  # 单元测试 / 压测脚本
├── thirdparty/            # 第三方依赖（如 hiredis、mysql-connector）
├── run.sh                 # 一键编译脚本
├── CMakeLists.txt
└── README.md
```

---

## 技术亮点

- **自研网络库**：完全自主实现的 Reactor 模型，包含 `EventLoop`、`Poller(epoll)`、`Channel`、`TcpConnection`、`Buffer`、`EventLoopThreadPool` 等核心组件。
- **跨线程任务调度**：通过 `eventfd` + `wakeup` 机制实现安全的任务投递。
- **高性能 Buffer**：利用 `readv/writev` 配合动态扩容，减少内存拷贝。
- **集群支持**：Nginx TCP 负载均衡 + Redis 发布/订阅，支持多节点水平扩展。

---

## 依赖环境

- Linux（推荐 Ubuntu 20.04+）
- C++11 及以上编译器（g++ 7.5+）
- CMake 3.10+
- MySQL 5.7+
- Redis 6.0+（开启发布/订阅功能）
- Nginx 1.18+（编译时添加 `--with-stream` 模块支持 TCP 代理）
