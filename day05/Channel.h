#pragma once
#include <sys/epoll.h>

class Epoll;

class Channel {
private:
    Epoll* ep;         // 因为 channel 管理的事件说白了还是 epoll 负责监听，ep 和 channel 是 一对多的关系
    int fd;            // channel 负责托管的 fd, fd 和 channel 是一对一的关系
    uint32_t events;   // 负责监听的事件
    uint32_t revents;  // 已经发生并返回的事件
    bool inEpoll;      // 标志位，表示 epoll 是否在树上

public:
    Channel(Epoll* ep, int fd);
    ~Channel();

    int getFd();

    uint32_t getEvents();
    uint32_t getRevents();

    void setRevents(uint32_t);

    bool getInEpoll();
    void setInEpoll();

    void enableReading();
};