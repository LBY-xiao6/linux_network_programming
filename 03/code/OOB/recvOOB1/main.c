#include "wrap.h"

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

    int connfd = Accept(sockfd, NULL, NULL);

    // 2，将TCP的接收水位线设置为100个字节
    int lowat = 100;
    setsockopt(connfd, SOL_SOCKET, SO_RCVLOWAT, &lowat, sizeof(int));

    // 3，使用select同时监控connfd的读就绪和异常就绪
    fd_set rset;
    fd_set eset;

    char buf[20];
    while(1)
    {
        FD_ZERO(&rset);
        FD_ZERO(&eset);

        // 将相应的套接字置入对应的集合中
        FD_SET(connfd, &rset);
        FD_SET(connfd, &eset);

        // 同时监听普通数据和紧急数据
        select(connfd+1, &rset, NULL, &eset, NULL);

        // 对方有普通数据来了！
        if(FD_ISSET(connfd, &rset))
        {
            bzero(buf, 20);
            int n = read(connfd, buf, 20);
            if(n > 0)
            {
                printf("收到普通数据: %s\n", buf);
            }
            if(n <= 0)
                break;
        }

        // 对方有紧急数据来了！
        if(FD_ISSET(connfd, &eset))
        {
            char oob;
            recv(connfd, &oob, 1, MSG_OOB);
            printf("收到紧急数据: %c\n", oob);
        }
    }

    return 0;
}
