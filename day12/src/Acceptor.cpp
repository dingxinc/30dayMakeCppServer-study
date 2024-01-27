#include "Acceptor.h"
#include "Socket.h"
#include "InetAddress.h"
#include "Channel.h"
#include "EventLoop.h"
#include "Server.h"
#include <stdio.h>

Acceptor::Acceptor(EventLoop *_loop) : loop(_loop) {
    sock = new Socket();
    InetAddress *addr = new InetAddress("127.0.0.1", 1234);
    sock->bind(addr);
    sock->listen();

    acceptChannel = new Channel(loop, sock->getFd());
    std::function<void()> cb = std::bind(&Acceptor::acceptConnection, this);
    acceptChannel->setReadCallback(cb);
    acceptChannel->enableReading();
    acceptChannel->setUseThreadPool(false);
    delete addr;
}

Acceptor::~Acceptor() {
    delete sock;
    delete acceptChannel;
}

void Acceptor::acceptConnection() {
    InetAddress *clntaddr = new InetAddress();
    Socket *clntsock = new Socket(sock->accept(clntaddr));
    printf("new client fd %d! IP: %s Port: %d\n", clntsock->getFd(), inet_ntoa(clntaddr->getAddr().sin_addr), ntohs(clntaddr->getAddr().sin_port));
    newConnectionCallback(clntsock);
    delete clntaddr;
}

void Acceptor::setNewConnectionCallback(std::function<void(Socket*)> _cb) {
    newConnectionCallback = _cb;
}