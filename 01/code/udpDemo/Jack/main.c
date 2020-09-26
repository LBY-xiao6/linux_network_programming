#include "common.h"

int main(int argc, char **argv) // ./Jack 对方IP 对方端口
{
    // 1，创建UDP通信端点
    int sockfd = Socket(AF_INET, SOCK_DGRAM, 0);

    // 2，准备好对方的地址（IP+PORT）
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    bzero(&addr, len);

    addr.sin_family = AF_INET;  // 指定协议族，这里是IPv4地址
    addr.sin_addr.s_addr = inet_addr(argv[1]); // IP地址
    addr.sin_port = htons(atoi(argv[2]));      // PORT端口号

    // 3，给对方发去消息
    char buf[50];
    while(1)
    {
        bzero(buf, 50);
        fgets(buf, 50, stdin);

        sendto(sockfd, buf, strlen(buf), 0,
               (struct sockaddr *)&addr, len);
    }

    return 0;
}
