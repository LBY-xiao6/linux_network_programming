#include "wrap.h"

// 使用UDP发送广播消息

int main(int argc, char **argv) // ./main 192.168.1.100 50001 --> 单播
                                // ./main 192.168.1.255 50001 --> 广播
{
    // 1，准备好UDP通信端点
    int sockfd = Socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    bzero(&addr, len);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(argv[1]);
    addr.sin_port = htons(atoi(argv[2]));

    // 2，使能广播
    int on = 1;
    int a = setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST,
                        &on, sizeof(on));
    if(a != 0)
    {
        perror("使能广播失败");
        exit(0);
    }

    // 3，发送消息
    int b = sendto(sockfd, "abcd\n", 5, 0, (struct sockaddr *)&addr,
                    len);
    printf("发送了%d个字节\n", b);

    return 0;
}
