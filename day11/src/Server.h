#pragma once
#include <map>

class EventLoop;
class Acceptor;
class Connection;
class Socket;

class Server {
private:
    EventLoop *loop;
    Acceptor *acceptor;   // 用于接收 TCP 连接
    std::map<int, Connection*> connections;  // 所有的 TCP 连接

public:
    Server(EventLoop *loop);
    ~Server();

    void handleReadEvent(int);
    void newConnection(Socket *servsock);  // 新建 TCP 连接
    void deleteConnection(int sockfd);  // 删除 TCP 连接
};