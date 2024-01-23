# 30dayMakeCppServer-study

## day01

一个最简单的 socket 服务器通信程序，服务器端绑定到本机 "127.0.0.1" 的 8888 端口，客户端连接上来后，在服务端打印了连接信息，没有做任何数据的收发操作。

## day02

添加了一个错误处理函数，用来处理 `socket`、`bind`、`listen`、`accept`、`connect` 等函数的错误返回信息。
```c++
void errif(bool condition, const char* errmsg) {
    if (condition) {
        perror(errmsg);
        exit(EXIT_FAILURE);
    }
}
```
并且通过 `while(true)` 完成客户端和服务端的简单通信。

## day03

在上一天，我们写了一个简单的echo服务器，但只能同时处理一个客户端的连接。但在这个连接的生命周期中，绝大部分时间都是空闲的，活跃时间（发送数据和接收数据的时间）占比极少，这样独占一个服务器是严重的资源浪费。事实上所有的服务器都是高并发的，可以同时为成千上万个客户端提供服务，这一技术又被称为IO复用。

epoll默认采用LT触发模式，即水平触发，只要fd上有事件，就会一直通知内核。这样可以保证所有事件都得到处理、不容易丢失，但可能发生的大量重复通知也会影响epoll的性能。如使用ET模式，即边缘触法，fd从无事件到有事件的变化会通知内核一次，之后就不会再次通知内核。这种方式十分高效，可以大大提高支持的并发度，但程序逻辑必须一次性很好地处理该fd上的事件，编程比LT更繁琐。注意ET模式必须搭配非阻塞式socket使用。

## day04

把所有逻辑放在一个程序里显然是一种错误的做法，我们需要对程序进行模块化，每一个模块专门处理一个任务，这样可以增加程序的可读性，也可以写出更大庞大、功能更加复杂的程序。不仅如此，还可以很方便地进行代码复用。

C++是一门面向对象的语言，最低级的模块化的方式就是构建一个类。在服务器开发中，我们或许会建立多个socket口，或许会处理多个客户端连接，但我们并不希望每次都重复编写这么多行代码，我们希望这样使用：
```c++
Socket *serv_sock = new Socket();
InetAddress *serv_addr = new InetAddress("127.0.0.1", 8888);
serv_sock->bind(serv_addr);
serv_sock->listen();   
InetAddress *clnt_addr = new InetAddress();  
Socket *clnt_sock = new Socket(serv_sock->accept(clnt_addr));    
```
对于 Epoll 的操作我们希望这样用：
```c++
Epoll *ep = new Epoll();
ep->addFd(serv_sock->getFd(), EPOLLIN | EPOLLET);
while(true){
    vector<epoll_event> events = ep->poll();
    for(int i = 0; i < events.size(); ++i){
        // handle event
    }
}
```
同样完全忽略了如错误处理之类的底层细节，大大简化了编程，增加了程序的可读性。