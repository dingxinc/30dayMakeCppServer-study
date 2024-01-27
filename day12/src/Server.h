#pragma once
#include <map>
#include <vector>

class EventLoop;
class Acceptor;
class Connection;
class ThreadPool;
class Socket;

class Server {
private:
    EventLoop *mainReactor;
    Acceptor *acceptor;   // 用于接收 TCP 连接
    std::map<int, Connection*> connections;  // 所有的 TCP 连接
    std::vector<EventLoop*> subReactors;
    ThreadPool *thpool;

public:
    Server(EventLoop *loop);
    ~Server();

    void handleReadEvent(int);
    void newConnection(Socket *servsock);  // 新建 TCP 连接
    void deleteConnection(int sockfd);  // 删除 TCP 连接
};