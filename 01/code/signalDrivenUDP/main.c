#include "wrap.h"

int sockfd;

// 信号处理函数
void readMsg(int sig)
{
    char buf[50];
    bzero(buf, 50);

    recvfrom(sockfd, buf, 50, 0, NULL, NULL);
    printf("收到信息: %s", buf);
}

int main(int argc, char **argv) // ./main IP PORT
{
    // 1，创建UDP套接字
    sockfd = Socket(AF_INET, SOCK_DGRAM, 0);

    // 2，绑定IP、端口
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    bzero(&addr, len);

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(argv[1]);
    addr.sin_port = htons(atoi(argv[2]));

    Bind(sockfd, (struct sockaddr *)&addr, len);

    // 3，准备使用信号驱动的方式来接收UDP数据
    // 3.1 关联信号及其处理函数
    signal(SIGIO, readMsg);

    // 3.2 将套接字的模式设定更为异步模式，从而触发信号
    int state = fcntl(sockfd, F_GETFL);
    state |= O_ASYNC;
    fcntl(sockfd, F_SETFL, state);

    // 3.3 设定信号的属主
    fcntl(sockfd, F_SETOWN, getpid());

    // 服务器现在可以去干别的了
    for(int i=0; ; i++)
    {
        printf("%d\n", i);
        sleep(1);
    }

    return 0;
}
