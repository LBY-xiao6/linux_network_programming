#include "wrap.h"

int sockfd;

void *routine(void *arg)
{
    char *buf = calloc(1, 1024);
    while(1)
    {
        int n = read(sockfd, buf, 1024);

        if(n > 0)
            printf("%s", buf);
        else
            break;
    }
}

int main(int argc, char **argv)
{
    if(argc != 3)
    {
        printf("参数错误！用法:<IP> <PORT>\n");
        exit(0);
    }

    // 1，创建TCP通信端点
    sockfd = Socket(AF_INET, SOCK_STREAM, 0);

    // 2，准备服务器地址（IP+PORT）
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    bzero(&addr, len);

    addr.sin_family = AF_INET;  // 指定协议族，这里是IPv4地址
    addr.sin_addr.s_addr = inet_addr(argv[1]); // IP地址
    addr.sin_port = htons(atoi(argv[2]));      // PORT端口号

    // 3，发起连接请求
    Connect(sockfd, (struct sockaddr *)&addr, len);

    // 4，子线程: 负责从服务器接收数据
    pthread_t tid;
    pthread_create(&tid, NULL, routine, NULL);

    // 5，主线程: 负责向服务器发送数据
    char buf[50];
    while(1)
    {
        bzero(buf, 50);
        if(fgets(buf, 50, stdin) == NULL)
            break;

        write(sockfd, buf, strlen(buf));
    }

    pthread_cancel(tid);
    close(sockfd);

    return 0;
}
