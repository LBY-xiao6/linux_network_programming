#include "common.h"

int main(int argc, char **argv) // ./Rose 自身IP 自身端口
{
    // 1，创建UDP通信端点
    int sockfd = Socket(AF_INET, SOCK_DGRAM, 0);

    // 2，准备好要绑定的自身的地址（IP+PORT）
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    bzero(&addr, len);

    addr.sin_family = AF_INET;  // 指定协议族，这里是IPv4地址
    addr.sin_addr.s_addr = inet_addr(argv[1]); // IP地址
    addr.sin_port = htons(atoi(argv[2]));      // PORT端口号

    // 3，绑定地址
    Bind(sockfd, (struct sockaddr *)&addr, len);

    // 4，静静等待对方的数据
    char buf[50];
    while(1)
    {
        bzero(buf, 50);
        recvfrom(sockfd, buf, 50, 0, NULL, NULL);

        printf("收到对方消息:%s", buf);
    }

    return 0;
}
