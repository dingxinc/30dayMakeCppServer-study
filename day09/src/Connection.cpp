#include "Connection.h"
#include "EventLoop.h"
#include "Socket.h"
#include "Channel.h"
#include "Buffer.h"
#include "util.h"
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

Connection::Connection(EventLoop *_loop, Socket *_sock) : loop(_loop), sock(_sock), channel(nullptr), inBuffer(new std::string()), readBuffer(nullptr) {
    channel = new Channel(loop, sock->getFd());
    std::function<void()> cb = std::bind(&Connection::echo, this, sock->getFd());
    channel->setCallback(cb);
    channel->enableReading();
    readBuffer = new Buffer();
}

Connection::~Connection() {
    delete channel;
    delete sock;
}

void Connection::echo(int sockfd) {
    char buf[1024];      // 这个 buf 大小无所谓
    while (true) {       // 非阻塞模式，必须一次读完
        bzero(&buf, sizeof(buf));
        ssize_t read_bytes = read(sockfd, buf, sizeof(buf));
        if (read_bytes > 0) {
            readBuffer->append(buf, read_bytes);    
        } else if (read_bytes == -1 && errno == EINTR) {  // 信号中断，继续读
            printf("siglnal eintr, continue read...\n");
            continue;
        } else if (read_bytes == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK))) {  // 非阻塞模式下，表示读完
            printf("finish reading once\n");
            printf("message from client fd %d: %s\n", sockfd, readBuffer->c_str());
            errif(write(sockfd, readBuffer->c_str(), readBuffer->size()) == -1, "socket write error");
            readBuffer->clear();
            break;
        } else if (read_bytes == 0) {  // EOF, 客户端断开连接
            printf("EOF client sock fd %d, is disconnection.\n", sockfd);
            //close(sockfd);  // 关闭文件描述符，从红黑树上移除
            deleteConnectionCallback(sock);    // 连接断开后，删除连接
            break;
        }
    }
}

void Connection::setDeleteConnectionCallback(std::function<void(Socket *)> _cb) {
    deleteConnectionCallback = _cb;
}