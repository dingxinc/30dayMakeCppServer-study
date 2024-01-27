server:
	g++ -std=c++11 src/util.cpp src/InetAddress.cpp src/Socket.cpp src/Epoll.cpp src/Channel.cpp src/EventLoop.cpp src/Server.cpp src/Acceptor.cpp src/Connection.cpp src/Buffer.cpp server.cpp -o server &&\
    g++ src/util.cpp src/Socket.cpp src/InetAddress.cpp src/Buffer.cpp client.cpp -o client

clean:
	rm server && rm client