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

重要：作为第一个小版本，需要搞懂面向对象的核心思想！！

增加了三个类 `InetAddress.cpp Socket.cpp Epoll.cpp` , 运用面向对象的思想对程序进行了封装， 注意对于类对象的使用，都是 new 出来的堆对象，**server 程序中使用的对象都没有进行 delete 操作。**

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

## day05

让我们来回顾一下我们是如何使用epoll：将一个文件描述符添加到epoll红黑树，当该文件描述符上有事件发生时，拿到它、处理事件，这样我们每次只能拿到一个文件描述符，也就是一个int类型的整型值。试想，如果一个服务器同时提供不同的服务，如HTTP、FTP等，那么就算文件描述符上发生的事件都是可读事件，不同的连接类型也将决定不同的处理逻辑，仅仅通过一个文件描述符来区分显然会很麻烦，我们更加希望拿到关于这个文件描述符更多的信息。

```c++
typedef union epoll_data {
  void *ptr;
  int fd;
  uint32_t u32;
  uint64_t u64;
} epoll_data_t;

struct epoll_event {
  uint32_t events;	/* Epoll events */
  epoll_data_t data;	/* User data variable */
} __EPOLL_PACKED;
```

可以看到，epoll中的data其实是一个union类型，可以储存一个指针。而通过指针，理论上我们可以指向任何一个地址块的内容，可以是一个类的对象，这样就可以将一个文件描述符封装成一个Channel类，一个Channel类自始至终只负责一个文件描述符，对不同的服务、不同的事件类型，都可以在类中进行不同的处理，而不是仅仅拿到一个int类型的文件描述符。

```c++
class Channel{
private:
    Epoll *ep;
    int fd;
    uint32_t events;
    uint32_t revents;
    bool inEpoll;
};
```

显然每个文件描述符会被分发到一个Epoll类，用一个ep指针来指向。类中还有这个Channel负责的文件描述符。另外是两个事件变量，events表示希望监听这个文件描述符的哪些事件，因为不同事件的处理方式不一样。revents表示在epoll返回该Channel时文件描述符正在发生的事件。inEpoll表示当前Channel是否已经在epoll红黑树中，为了注册Channel的时候方便区分使用EPOLL_CTL_ADD还是EPOLL_CTL_MOD。

注：在今天教程的源代码中，并没有将事件处理改为使用Channel回调函数的方式，仍然使用了之前对文件描述符进行处理的方法，这是错误的，将在明天的教程中进行改写。

## day06

我们的服务器已经基本成型，但目前从新建socket、接受客户端连接到处理客户端事件，整个程序结构是顺序化、流程化的，我们甚至可以使用一个单一的流程图来表示整个程序。而流程化程序设计的缺点之一是不够抽象，当我们的服务器结构越来越庞大、功能越来越复杂、模块越来越多，这种顺序程序设计的思想显然是不能满足需求的。

对于服务器开发，我们需要用到更抽象的设计模式。**从代码中我们可以看到，不管是接受客户端连接还是处理客户端事件，都是围绕epoll来编程，可以说epoll是整个程序的核心，服务器做的事情就是监听epoll上的事件，然后对不同事件类型进行不同的处理。** 这种以事件为核心的模式又叫事件驱动，事实上几乎所有的现代服务器都是事件驱动的。

接下来我们要将服务器改造成Reactor模式。首先我们将整个服务器抽象成一个Server类，这个类中有一个main-Reactor（在这个版本没有sub-Reactor），里面的核心是一个EventLoop（libevent中叫做EventBase），这是一个事件循环，我们添加需要监听的事务到这个事件循环内，每次有事件发生时就会通知（在程序中返回给我们Channel），然后根据不同的描述符、事件类型进行处理（以回调函数的方式）。

## day07

在上一天，我们分离了服务器类和事件驱动类，将服务器逐渐开发成Reactor模式。至此，所有服务器逻辑（目前只有接受新连接和echo客户端发来的数据）都写在Server类里。但很显然，Server作为一个服务器类，应该更抽象、更通用，我们应该对服务器进行进一步的模块化。

仔细分析可发现，对于每一个事件，不管提供什么样的服务，首先需要做的事都是调用accept()函数接受这个TCP连接，然后将socket文件描述符添加到epoll。当这个IO口有事件发生的时候，再对此TCP连接提供相应的服务。

这样一来，新建连接的逻辑就在Acceptor类中。但逻辑上新socket建立后就和之前监听的服务器socket没有任何关系了，TCP连接和Acceptor一样，拥有以上提到的三个特点，这两个类之间应该是平行关系。所以新的TCP连接应该由Server类来创建并管理生命周期，而不是Acceptor。并且将这一部分代码放在Server类里也并没有打破服务器的通用性，因为对于所有的服务，都要使用Acceptor来建立连接。(对应的是这一句话：**类中的socket fd就是服务器监听的socket fd，每一个Acceptor对应一个socket fd**)

## day08

增加 TCP 连接类 Connection.cpp 在 Server 类中管理监听文件描述符 listenfd 和 TCP 连接的关系，用 map 容器管理。

## day09

为每一个 TCP 连接分配一个读缓冲区后，就可以把客户端的信息读取到这个缓冲区内，缓冲区大小就是客户端发送的报文真实大小。主要改的是 connetcion 类和 client 客户端，增加缓冲区。Socket 类增加 connect 函数。

## day10

增加线程池，在 eventloop 类中创建线程池对象，修改 channel 类中的 handleEvent 函数，改成线程池模式。

## day11

完善线程池，使用模板修改了线程池 add 函数，修改了 channel 类。修改 acceptor 类，设置不使用线程池。
connection 类 echo 函数修改，增加 send 函数。Epoll 类修改。Server 类修改。

## day12

改成 one loop per thread 模式。