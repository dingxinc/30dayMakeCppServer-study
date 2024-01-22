#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>

int main() {
    int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = htons(8888);

    bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
    listen(sockfd, SOMAXCONN);   // SOMAXCONN 的值默认是 128

    struct sockaddr_in clientaddr;
    socklen_t len = sizeof(clientaddr);
    bzero(&clientaddr, sizeof(clientaddr));

    int client_sock = accept(sockfd, (struct sockaddr*)&clientaddr, &len);
    printf("client(fd=%d) is connection, ip = %s, port = %d\n", client_sock, inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

    return 0;
}