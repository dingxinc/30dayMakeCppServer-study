#include "Connection.h"
#include "EventLoop.h"
#include "Socket.h"
#include "Channel.h"
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#define READ_BUFFER 1024

Connection::Connection(EventLoop *_loop, Socket *_sock) : loop(_loop), sock(_sock), channel(nullptr) {
    channel = new Channel(loop, sock->getFd());
    std::function<void()> cb = std::bind(&Connection::echo, this, sock->getFd());
    channel->setCallback(cb);
    channel->enableReading();
}

Connection::~Connection() {
    delete channel;
    delete sock;
}

void Connection::echo(int sockfd) {
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

void Connection::setDeleteConnectionCallback(std::function<void(Socket *)> _cb) {
    deleteConnectionCallback = _cb;
}