#include "wrap.h"

int connfd;

// 信号响应函数
void recvOOB(int sig)
{
    char oob;
    recv(connfd, &oob, 1, MSG_OOB);
    printf("收到紧急数据: %c\n", oob);
}

int main(int argc, char **argv)
{
    if(argc != 3)
    {
        printf("参数错误！用法: <IP> <PORT>\n");
        exit(0);
    }

    // 1，我是B，等待A的连接
    int sockfd = Socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    bzero(&addr, len);

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(argv[1]);
    addr.sin_port = htons(atoi(argv[2]));

    Bind(sockfd, (struct sockaddr *)&addr, len);
    Listen(sockfd, 3);

    connfd = Accept(sockfd, NULL, NULL);

    // 2，将TCP的接收水位线设置为100个字节
    int lowat = 100;
    setsockopt(connfd, SOL_SOCKET, SO_RCVLOWAT, &lowat, sizeof(int));

    // 3，为紧急数据触发的SIGURG关联对应的处理函数，并设置属主
    signal(SIGURG, recvOOB);
    fcntl(connfd, F_SETOWN, getpid());

    // 4，静静地等待普通数据
    char buf[20];
    while(1)
    {
        bzero(buf, 20);
        int n = read(connfd, buf, 20);

        if(n > 0)
        {
            printf("收到普通数据:%s\n", buf);
        }

        if(n <= 0)
            break;
    }

    return 0;
}
