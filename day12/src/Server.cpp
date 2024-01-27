#include "EventLoop.h"
#include "Socket.h"
#include "InetAddress.h"
#include "Acceptor.h"
#include "Connection.h"
#include "Server.h"
#include "Channel.h"
#include "ThreadPool.h"
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <functional>

/* 完成监听套接字的创建和绑定监听 */
Server::Server(EventLoop *_loop) : mainReactor(_loop), acceptor(nullptr) {
    acceptor = new Acceptor(mainReactor);
    std::function<void(Socket*)> cb = std::bind(&Server::newConnection, this, std::placeholders::_1);
    acceptor->setNewConnectionCallback(cb);

    int size = std::thread::hardware_concurrency();
    thpool = new ThreadPool(size);
    for (int i = 0; i < size; ++i) {
        subReactors.push_back(new EventLoop());
    }

    for (int i = 0; i < size; ++i) {
        std::function<void()> sub_loop = std::bind(&EventLoop::loop, subReactors[i]);
        thpool->add(sub_loop);
    }
}

Server::~Server() {
    delete acceptor;
    delete thpool;
}

/* 处理新连接 */
void Server::newConnection(Socket *servsock) {
    if (servsock->getFd() != -1) {
        int random = servsock->getFd() % subReactors.size();
        Connection *conn = new Connection(subReactors[random], servsock);
        std::function<void(int)> cb = std::bind(&Server::deleteConnection, this, std::placeholders::_1);
        conn->setDeleteConnectionCallback(cb);
        connections[servsock->getFd()] = conn;
    }
}

void Server::deleteConnection(int sockfd) {
    if (sockfd != -1) {
        auto it = connections.find(sockfd);
        if (it != connections.end()) {
            Connection *conn = connections[sockfd];
            connections.erase(sockfd);
            // close(sockfd);     // 正常
            delete conn;          // 会 Segmant fault
        }
    }
}