#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include "InetAddress.h"
#include "Socket.h"
#include "Epoll.h"
#include "util.h"
#include "Channel.h"

#define MAX_EVENTS 1024
#define BUF_SIZE 1024

void handleReadEvent(int);    // 处理可读事件

int main() {
    Socket *servsock = new Socket();

    InetAddress *servaddr = new InetAddress("127.0.0.1", 8888);

    servsock->bind(servaddr);
    servsock->listen();

    Epoll *ep = new Epoll();
    // ep->addFd(servsock->getFd(), EPOLLIN | EPOLLET);  // 将 listenfd 添加到红黑树上监听
    Channel *servChannel = new Channel(ep, servsock->getFd());
    servChannel->enableReading();

    while (true) {
        // std::vector<epoll_event> evs = ep->poll();
        std::vector<Channel*> activeChannels = ep->poll();
        for (int i = 0; i < activeChannels.size(); ++i) {
            if (activeChannels[i]->getFd() == servsock->getFd()) {          // 有新客户端连接
                InetAddress *clntaddr = new InetAddress();      // 会发生内存泄漏，没有 delete
                Socket *clntsock = new Socket(servsock->accept(clntaddr));

                printf("new client fd %d! IP: %s Port: %d\n", clntsock->getFd(), inet_ntoa(clntaddr->addr.sin_addr), ntohs(clntaddr->addr.sin_port));

                Channel *clntChannel = new Channel(ep, clntsock->getFd());
                // ep->addFd(clntsock->getFd(), EPOLLIN | EPOLLET);
                clntChannel->enableReading();
            } else if (activeChannels[i]->getRevents() & (EPOLLIN | EPOLLPRI)) {   // 可读事件
                handleReadEvent(activeChannels[i]->getFd());
            } else {
                printf("do something else.\n");
            }
        }
    }
    delete servsock;     // 在析构函数中关闭了文件描述符
    delete servaddr;
    return 0;
}

void handleReadEvent(int sockfd) {
    char buf[BUF_SIZE];
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