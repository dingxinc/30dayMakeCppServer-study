#include "util.h"
#include <stdio.h>
#include <stdlib.h>

/*
首先介绍一下：
exit(0): 正常执行程序并退出程序。
exit(1): 非正常执行导致退出程序。
其次介绍：
stdlib.h头文件中 定义了两个变量：
#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1
最后介绍:
exit(EXIT_SUCCESS)  : 代表安全退出。
exit(EXIT_FAILURE) ： 代表异常退出。
*/

void errif(bool condition, const char* errmsg) {
    if (condition) {
        perror(errmsg);
        exit(EXIT_FAILURE);
    }
}