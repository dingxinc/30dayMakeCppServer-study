#include "Epoll.h"
#include "util.h"
#include "Channel.h"
#include <unistd.h>
#include <string.h>

#define MAX_EVENTS 1024

Epoll::Epoll() : epfd(-1), events(nullptr) {
    epfd = epoll_create1(0);
    errif(epfd == -1, "epoll_create error.");
    events = new epoll_event[MAX_EVENTS];      // 注意这里 epoll_event 可以 new 出来
    // 这里第一个参数是一个指针，events 本身就是一个指针，所以就不是填 &events，而就是 events，不然 epoll_wait() 返回 -1
    // 初始化为 0 是一个好习惯，但是记住，对指针和数组的操作一定要小心再小心，不然会引发难以预料的错误
    bzero(events, sizeof(*events) * MAX_EVENTS);    // 这个一个数组指针
}

Epoll::~Epoll() {
    if (epfd != -1) {
        close(epfd);
        epfd = -1;
    }
    delete [] events;
}

void Epoll::addFd(int fd, uint32_t op) {
    struct epoll_event ev;
    bzero(&ev, sizeof(ev));
    ev.data.fd = fd;
    ev.events = op;
    errif(epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) == -1, "epoll add event error");
}

/**
std::vector<epoll_event> Epoll::poll(int timeout) {
    std::vector<epoll_event> evs;
    bzero(&evs, sizeof(evs));
    int nfds = epoll_wait(epfd, events, MAX_EVENTS, timeout);  // epoll_wait 会将发生的事件返回到 events中
    errif(nfds == -1, "epoll_wait error.");
    for (int i = 0; i < nfds; ++i) {
        evs.push_back(events[i]);
    }

    return evs;   // evs 中都是已发生的事件
}
**/

void Epoll::updateChannel(Channel* channel) {
    int fd = channel->getFd();
    struct epoll_event ev;
    bzero(&ev, sizeof(ev));
    ev.data.ptr = channel;
    ev.events = channel->getEvents();    // 这里的 getEvents() 是在 Channel 类中的 enableReading() 中设置的
    if (!channel->getInEpoll()) {
        errif(epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) == -1, "epoll_ctl error");
        channel->setInEpoll();        
    } else {  // 已经在树上，直接修改
        errif(epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev) == -1, "epoll_ctl error");
    }
}

std::vector<Channel*> Epoll::poll(int timeout) {
    std::vector<Channel*> channels;
    int nfds = epoll_wait(epfd, events, MAX_EVENTS, timeout);
    errif(nfds == -1, "epoll wait error");
    for (int i = 0; i < nfds; ++i) {
        Channel* ch = (Channel*)events[i].data.ptr;
        ch->setRevents(events[i].events);
        channels.push_back(ch);
    }
    return channels;
}