#include "EventLoop.h"
#include "Socket.h"
#include "InetAddress.h"
#include "Acceptor.h"
#include "Connection.h"
#include "Server.h"
#include "Channel.h"
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>

#define READ_BUFFER 1024

/* 完成监听套接字的创建和绑定监听 */
Server::Server(EventLoop *_loop) : loop(_loop), acceptor(nullptr) {
    acceptor = new Acceptor(loop);
    std::function<void(Socket*)> cb = std::bind(&Server::newConnection, this, std::placeholders::_1);
    acceptor->setNewConnectionCallback(cb);
}

Server::~Server() {
    delete acceptor;
}

/* 处理新连接 */
void Server::newConnection(Socket *servsock) {
    if (servsock->getFd() != -1) {
        Connection *conn = new Connection(loop, servsock);
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