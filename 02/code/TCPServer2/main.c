#include "wrap.h"

// TCP轮询服务器

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

    // 5，将监听套接字设置为非阻塞状态
    int state = fcntl(sockfd, F_GETFL);
    state |= O_NONBLOCK;
    fcntl(sockfd, F_SETFL, state);

    struct sockaddr_in peer;
    len = sizeof(peer);

    // 6，循环地接收远端连接
    int connfd[100];
    bzero(connfd, sizeof(connfd));
    int k = 0;
    while(1)
    {
        bzero(&peer, len);
        connfd[k] = accept(sockfd, (struct sockaddr *)&peer, &len);
        if(connfd[k] == -1)
            continue;

        printf("欢迎[%s:%hu]\n", inet_ntoa(peer.sin_addr),
               ntohs(peer.sin_port));

        // 将新的已连接套接字设置为非阻塞
        int state = fcntl(connfd[k], F_GETFL);
        state |= O_NONBLOCK;
        fcntl(connfd[k], F_SETFL, state);

        k++;
        char buf[100];
        for(int i=0; i<k; i++)
        {
            // 跳过所有已经关闭了的客户端
            if(connfd[i] == -1)
                continue;

            bzero(buf, 100);
            int n = read(connfd[i], buf, 100);
            printf("n:%d\n", n);

            // 客户端发来了数据
            if(n > 0)
            {
                printf("%s", buf);
            }

            // 客户端已经关闭了
            else if(n == 0)
            {
                close(connfd[i]);
                connfd[i] = -1;
            }
        }
    }

    // 释放资源
    close(connfd);
    close(sockfd);

    return 0;
}
