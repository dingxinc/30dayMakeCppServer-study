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