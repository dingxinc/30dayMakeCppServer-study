#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "util.h"

int main() {
    int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    errif(sockfd == -1, "sock create failed.");

    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = htons(8888);

    errif(bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1, "bind failed.");
    errif(listen(sockfd, SOMAXCONN) == -1, "listen falied");   // SOMAXCONN 的值默认是 128

    struct sockaddr_in clientaddr;
    socklen_t len = sizeof(clientaddr);
    bzero(&clientaddr, sizeof(clientaddr));

    int client_sock = accept(sockfd, (struct sockaddr*)&clientaddr, &len);
    errif(client_sock == -1, "accept failed");
    printf("client(fd=%d) is connection, ip = %s, port = %d\n", client_sock, inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

    while (true) {
        char buf[1024];
        bzero(&buf, sizeof(buf));

        ssize_t read_bytes = read(client_sock, buf, sizeof(buf));
        if (read_bytes > 0) {    // 接收客户端发送数据并返回给客户端，echo 服务器
            printf("message from client fd %d: %s\n", client_sock, buf);
            write(client_sock, buf, sizeof(buf));
        } else if (read_bytes == 0) {   // 客户端断开连接
            printf("client fd %d disconnection.\n", client_sock);
            close(client_sock);
            break;
        } else if (read_bytes == -1) {
            close(client_sock);
            errif(true, "server read error");
        }
    }
    close(sockfd);
    return 0;
}