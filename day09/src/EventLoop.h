#pragma once
#include <sys/epoll.h>

class Epoll;
class Channel;

class EventLoop {       // 就是 Epoll
private:
    Epoll *ep;
    bool quit;        // 事件循环是否停止的标志位

public:
    EventLoop();
    ~EventLoop();

    void loop();
    void updateChannel(Channel*);
};