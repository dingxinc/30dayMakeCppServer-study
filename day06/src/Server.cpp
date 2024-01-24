#include "EventLoop.h"
#include "Socket.h"
#include "InetAddress.h"
#include "Server.h"
#include "Channel.h"
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>

#define READ_BUFFER 1024

/* 完成监听套接字的创建和绑定监听 */
Server::Server(EventLoop *_loop) : loop(_loop) {
    Socket *servsock = new Socket();
    InetAddress *servaddr = new InetAddress("127.0.0.1", 8888);
    servsock->bind(servaddr);
    servsock->listen();

    Channel *servChannel = new Channel(loop, servsock->getFd());
    std::function<void()> cb = std::bind(&Server::newConnection, this, servsock);
    servChannel->setCallback(cb);
    servChannel->enableReading();
}

Server::~Server() {

}

/* 处理读事件 */
void Server::handleReadEvent(int sockfd) {
    char buf[READ_BUFFER];
    while (true) {       // 非阻塞模式，必须一次读完
        bzero(&buf, sizeof(buf));
        ssize_t read_bytes = read(sockfd, buf, sizeof(buf));
        if (read_bytes > 0) {
            printf("message from client fd %d : %s.\n", sockfd, buf);
            write(sockfd, buf, sizeof(buf));    
        } else if (read_bytes == -1 && errno == EINTR) {  // 信号中断，继续读
            printf("siglnal eintr, continue read...\n");
            continue;
        } else if (read_bytes == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK))) {  // 非阻塞模式下，表示读完
            printf("read complete.\n");
            break;
        } else if (read_bytes == 0) {  // EOF, 客户端断开连接
            printf("EOF client sock fd %d, is disconnection.\n", sockfd);
            close(sockfd);  // 关闭文件描述符，从红黑树上移除
            break;
        }
    }
}

/* 处理新连接 */
void Server::newConnection(Socket *servsock) {
    InetAddress *clntaddr = new InetAddress();
    Socket *clntsock = new Socket(servsock->accept(clntaddr));
    printf("new client fd %d! IP: %s Port: %d\n", clntsock->getFd(), inet_ntoa(clntaddr->addr.sin_addr), ntohs(clntaddr->addr.sin_port));

    Channel *clntChannel = new Channel(loop, clntsock->getFd()); // loop 和 ep 没区别，就是一层简单的封装
    std::function<void()> cb = std::bind(&Server::handleReadEvent, this, clntsock->getFd());
    clntChannel->setCallback(cb);
    clntChannel->enableReading();      // 客户端的文件描述符也挂上去了
}