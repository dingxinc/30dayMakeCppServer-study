#pragma once
#include <sys/epoll.h>
#include <vector>

class Channel;

class Epoll {
private:
    int epfd;
    struct epoll_event* events;

public:
    Epoll();
    ~Epoll();

    // void addFd(int fd, uint32_t op);
    void updateChannel(Channel*);        // 更新 channel
    void deleteChannel(Channel*);        // 删除 channel
    // std::vector<epoll_event> poll(int timeout = -1);
    std::vector<Channel*> poll(int timeout = -1);
};