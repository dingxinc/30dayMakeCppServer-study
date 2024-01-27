#include "Channel.h"
#include "EventLoop.h"
#include <unistd.h>

Channel::Channel(EventLoop *_loop, int _fd) : loop(_loop), fd(_fd), events(0), ready(0), inEpoll(false), useThreadPool(true) {

}

Channel::~Channel() {
    if (fd != -1) close(fd);
    fd = -1;
}

int Channel::getFd() {
    return fd;
}

uint32_t Channel::getEvents() {
    return events;
}

uint32_t Channel::getReady() {
    return ready;
}

void Channel::setReady(uint32_t _ev) {
    ready = _ev;
}

bool Channel::getInEpoll() {
    return inEpoll;
}

void Channel::setInEpoll(bool _in) {
    inEpoll = _in;
}

void Channel::handleEvent() {
    // callback();     // 仿函数
    // loop->addThread(callback);
    if (ready & (EPOLLIN | EPOLLPRI)) {
        if (useThreadPool) loop->addThread(readCallback);
        else readCallback();
    }
    if (ready & (EPOLLOUT)) {
        if (useThreadPool) loop->addThread(writeCallback);
        else writeCallback();
    }
}

void Channel::enableReading() {
    events |= EPOLLIN | EPOLLET;
    loop->updateChannel(this);
}

void Channel::useET() {
    events |= EPOLLET;
}

void Channel::setReadCallback(std::function<void()> _cb) {
    readCallback = _cb;
}

void Channel::setUseThreadPool(bool use) {
    useThreadPool = use;
}
