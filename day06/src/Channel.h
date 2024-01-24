#pragma once
#include <sys/epoll.h>
#include <functional>

class EventLoop;

class Channel {
private:
    // Epoll* ep;         // 因为 channel 管理的事件说白了还是 epoll 负责监听，ep 和 channel 是 一对多的关系
    EventLoop *loop;      // eventloop 中又 epoll 对象，又向上封装了一层
    int fd;            // channel 负责托管的 fd, fd 和 channel 是一对一的关系
    uint32_t events;   // 负责监听的事件
    uint32_t revents;  // 已经发生并返回的事件
    bool inEpoll;      // 标志位，表示 epoll 是否在树上
    std::function<void()> callback;  // 回调函数

public:
    // Channel(Epoll* ep, int fd);
    Channel(EventLoop *_loop, int _fd);
    ~Channel();

    int getFd();

    uint32_t getEvents();
    uint32_t getRevents();

    void setRevents(uint32_t);

    bool getInEpoll();
    void setInEpoll();

    void handleEvent();
    void enableReading();

    void setCallback(std::function<void()>);
};