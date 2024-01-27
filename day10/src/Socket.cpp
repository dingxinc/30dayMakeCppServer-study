#include "Socket.h"
#include "InetAddress.h"
#include "util.h"
#include <unistd.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <string.h>

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

void Socket::bind(InetAddress* _addr) {
    struct sockaddr_in addr = _addr->getAddr();
    socklen_t addr_len = _addr->getAddr_len();
    errif(::bind(fd, (sockaddr*)&addr, addr_len) == -1, "socket bind error");
    _addr->setInetAddr(addr, addr_len);
}

void Socket::listen() {
    errif(::listen(fd, SOMAXCONN) == -1, "listen error");
}

int Socket::accept(InetAddress* _addr) {
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);
    bzero(&addr, sizeof(addr));
    int clntsock = ::accept4(fd, (sockaddr*)&addr, &addr_len, SOCK_NONBLOCK);
    errif(clntsock == -1, "socket accept error");
    _addr->setInetAddr(addr, addr_len);
    return clntsock;
}

int Socket::getFd() {
    return fd;
}

/**
void Socket::connect(InetAddress* _addr) {
    struct sockaddr_in addr = _addr->getAddr();
    socklen_t addr_len = _addr->getAddr_len();
    errif(::connect(fd, (sockaddr*)&addr, addr_len) == -1, "socket connect error.");
}
**/