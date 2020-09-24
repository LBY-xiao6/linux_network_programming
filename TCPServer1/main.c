#include "wrap.h"

int main(int argc, char **argv) // ./main IP PORT
{
    if(argc != 3)
    {
        printf("参数错误！用法: <IP> <PORT>\n");
        exit(0);
    }

    // 1，创建TCP套接字
    int sockfd = Socket(AF_INET, SOCK_STREAM, 0);

    // 2，准备好地址结构体
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    bzero(&addr, len);

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(argv[1]);
    addr.sin_port = htons(atoi(argv[2]));

    // 3，绑定套接字和地址
    Bind(sockfd, (struct sockaddr *)&addr, len);

    // 4，将套接字设置为监听状态，顺便设置最大同时连接个数
    listen(sockfd, 3);


    struct sockaddr_in peer;
    len = sizeof(peer);

    // 5，循环地接收远端连接
    pid_t pid;

    int connfd;
    while(1)
    {
        bzero(&peer, len);
        connfd = accept(sockfd, (struct sockaddr *)&peer, &len);

        // 父进程持续监听连接请求
        if((pid=fork()) > 0)
        {
            continue;
        }

        // 子进程专门去处理新建的已连接套接字connfd
        char buf[100];
        while(1)
        {
            bzero(buf, 100);
            if(read(connfd, buf, 100) > 0)
            {
                    printf("[%s:%hu]: %s", inet_ntoa(peer.sin_addr),
                           ntohs(peer.sin_port), buf);
            }
        }
    }

    // 释放资源
    close(connfd);
    close(sockfd);

    return 0;
}
