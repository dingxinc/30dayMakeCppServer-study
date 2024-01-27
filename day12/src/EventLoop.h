#pragma once
#include <sys/epoll.h>
#include <functional>

class Epoll;
class Channel;
class ThreadPool;

class EventLoop {       // 就是 Epoll
private:
    Epoll *ep;
    ThreadPool *threadPool;
    bool quit;        // 事件循环是否停止的标志位

public:
    EventLoop();
    ~EventLoop();

    void loop();
    void updateChannel(Channel*);

    void addThread(std::function<void()>);
};