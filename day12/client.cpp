#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "src/util.h"
#include "src/Buffer.h"
#include "src/InetAddress.h"
#include "src/Socket.h"

int main() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    errif(sockfd == -1, "socket create failed.\n");

    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = htons(1234);

    errif(connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1, "connect failed.");

    Buffer *sendBuffer = new Buffer();
    Buffer *readBuffer = new Buffer();

    while (true) {
        sendBuffer->getline();     // 键盘输入一行     
        ssize_t write_bytes = write(sockfd, sendBuffer->c_str(), sendBuffer->size());  // 发送到服务端
        if (write_bytes == -1) {
            printf("server already disconnect, can't write any thing.!\n");
            break;
        }
        int already_read = 0;
        char buf[1024];   // 这个 buf 大小无所谓
        while (true) {
            bzero(&buf, sizeof(buf));
            ssize_t read_bytes = read(sockfd, buf, sizeof(buf));
            if (read_bytes > 0) {
                readBuffer->append(buf, read_bytes);
                already_read += read_bytes;
            } else if (read_bytes == 0) {  // EOF
                printf("server disconnected!\n");
                exit(EXIT_SUCCESS);
            }
            if (already_read >= sendBuffer->size()) {
                printf("message from server: %s\n", readBuffer->c_str());
                break;
            }
        }
        readBuffer->clear();
    }
    return 0;
}