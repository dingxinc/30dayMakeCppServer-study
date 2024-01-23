#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <errno.h>
#include <netinet/tcp.h>
#include <sys/fcntl.h>
#include <fcntl.h>
#include "util.h"

#define MAX_EVENTS 1024
#define BUF_SIZE 1024

int main() {
    int sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP);    
    errif(sockfd == -1, "sock create failed.");

    // 设置 listenfd 的属性
    int opt;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, static_cast<socklen_t>(sizeof(opt)));   // 必须的
    setsockopt(sockfd, SOL_SOCKET, TCP_NODELAY, &opt, static_cast<socklen_t>(sizeof(opt)));    // 必须的
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &opt, static_cast<socklen_t>(sizeof(opt)));   // Reactor 中用处不大
    setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, &opt, static_cast<socklen_t>(sizeof(opt)));   // 建议自己实现心跳机

    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = htons(8888);

    errif(bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1, "bind failed.");
    errif(listen(sockfd, 128) == -1, "listen falied");   // SOMAXCONN 的值默认是 128

    int epfd = epoll_create1(0);       // 创建红黑树实例
    errif(epfd == -1, "epoll_create() falied");

    struct epoll_event events[MAX_EVENTS], ev;
    bzero(&events, sizeof(events));

    bzero(&ev, sizeof(ev));
    ev.data.fd = sockfd;
    ev.events = EPOLLIN | EPOLLET;       // 使用 ET 模式监听 sockfd 上的 读事件
    epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &ev);

    while (true) {
        int nfds = epoll_wait(epfd, events, MAX_EVENTS, -1);
        errif(nfds == -1, "epoll_wait failed.");
        for (int i = 0; i < nfds; ++i) {
            if (events[i].data.fd == sockfd) {          // 有新客户端连接
                struct sockaddr_in clientaddr;
                bzero(&clientaddr, sizeof(clientaddr));
                socklen_t len = sizeof(clientaddr);

                int client_sock = accept4(sockfd, (struct sockaddr*)&clientaddr, &len, SOCK_NONBLOCK);
                errif(client_sock == -1, "socket accept error");
                printf("new client fd %d! IP: %s Port: %d\n", client_sock, inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

                bzero(&ev, sizeof(ev));
                ev.data.fd = client_sock;
                ev.events = EPOLLIN | EPOLLET;
                epoll_ctl(epfd, EPOLL_CTL_ADD, client_sock, &ev);     // 监听客户端的读事件
            } else if (events[i].events & (EPOLLIN | EPOLLPRI)) {   // 可读事件
                char buf[BUF_SIZE];
                while (true) {       // 非阻塞模式，必须一次读完
                    bzero(&buf, sizeof(buf));
                    ssize_t read_bytes = read(events[i].data.fd, buf, sizeof(buf));
                    if (read_bytes > 0) {
                        printf("message from client fd %d : %s.\n", events[i].data.fd, buf);
                        write(events[i].data.fd, buf, sizeof(buf));    
                    } else if (read_bytes == -1 && errno == EINTR) {  // 信号中断，继续读
                        printf("siglnal eintr, continue read...\n");
                        continue;
                    } else if (read_bytes == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK))) {  // 非阻塞模式下，表示读完
                        printf("read complete.\n");
                        break;
                    } else if (read_bytes == 0) {  // EOF, 客户端断开连接
                        printf("client sock fd %d, is disconnection.\n", events[i].data.fd);
                        close(events[i].data.fd);  // 关闭文件描述符，从红黑树上移除
                        break;
                    }
                }
            } else {
                printf("do something else.\n");
            }
        }
    }
    close(sockfd);
    return 0;
}