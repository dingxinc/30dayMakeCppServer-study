src=$(wildcard src/*.cpp)

server:
	g++ -std=c++11 -pthread -g \
	$(src) \
	server.cpp \
	-o server

client:
	g++ src/util.cpp src/Buffer.cpp client.cpp -o client

clean:
	rm server && rm client && rm test