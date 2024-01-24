#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "src/util.h"

#define BUF_SIZE 1024

int main() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    errif(sockfd == -1, "socket create failed.\n");

    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = htons(8888);

    errif(connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1, "connect failed.");

    while (true) {
        char buf[BUF_SIZE];
        bzero(&buf, sizeof(buf));
        scanf("%s", buf);
        
        ssize_t write_bytes = write(sockfd, buf, sizeof(buf));
        if (write_bytes == -1) {
            printf("server already disconnect, can't write any thing.!\n");
            break;
        }
        bzero(&buf, sizeof(buf));
        ssize_t read_bytes = read(sockfd, buf, sizeof(buf));
        if (read_bytes > 0) {
            printf("message form server : %s.\n", buf);
        } else if (read_bytes == 0) {
            printf("server socket disconnect.\n");
            break;
        } else if (read_bytes == -1) {
            errif(true, "server send message error.");
            close(sockfd);
        }
    }
    close(sockfd);
    return 0;
}