#pragma once

class EventLoop;
class Acceptor;
class Socket;

class Server {
private:
    EventLoop *loop;
    Acceptor *acceptor;

public:
    Server(EventLoop *loop);
    ~Server();

    void handleReadEvent(int);
    void newConnection(Socket *servsock);
};