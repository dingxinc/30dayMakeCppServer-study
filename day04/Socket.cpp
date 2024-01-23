#include "Socket.h"
#include "InetAddress.h"
#include "util.h"
#include <unistd.h>
#include <netinet/tcp.h>
#include <sys/socket.h>

Socket::Socket() : fd(-1) {
    fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP);
    errif(fd == -1, "socket create failed.");

    // 设置 listenfd 的属性
    int opt;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, static_cast<socklen_t>(sizeof(opt)));   // 必须的
    setsockopt(fd, SOL_SOCKET, TCP_NODELAY, &opt, static_cast<socklen_t>(sizeof(opt)));    // 必须的
    setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &opt, static_cast<socklen_t>(sizeof(opt)));   // Reactor 中用处不大
    setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &opt, static_cast<socklen_t>(sizeof(opt)));   // 建议自己实现心跳机
}

Socket::Socket(int _fd) : fd(_fd) {
    errif(fd == -1, "socket create error");
}

Socket::~Socket() {
    if (fd != -1) {
        close(fd);
        fd = -1;
    }
}

void Socket::bind(InetAddress* addr) {
    errif(::bind(fd, (sockaddr*)&addr->addr, addr->addr_len) == -1, "bind error.");
}

void Socket::listen() {
    errif(::listen(fd, SOMAXCONN) == -1, "listen error");
}

int Socket::accept(InetAddress* addr) {
    int clnt_sock = ::accept4(fd, (sockaddr*)&addr->addr, &addr->addr_len, SOCK_NONBLOCK);
    errif(clnt_sock == -1, "accept error");
    return clnt_sock;
}

int Socket::getFd() {
    return fd;
}