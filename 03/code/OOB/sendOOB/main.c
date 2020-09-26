#include "wrap.h"

int main(int argc, char **argv)
{
    if(argc != 3)
    {
        printf("参数错误！用法: <IP> <PORT>\n");
        exit(0);
    }

    // 1，我是A，连接B（服务端）
    int sockfd = Socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    bzero(&addr, len);

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(argv[1]);
    addr.sin_port = htons(atoi(argv[2]));

    Connect(sockfd, (struct sockaddr *)&addr, len);

    // 2，试图给对方发送普通数据和紧急数据
    // 2.1 发送了3个字节的普通数据
    write(sockfd, "abc", 3);
    sleep(3);

    // 2.2 发送了1个字节的紧急数据
    send(sockfd, "x", 1, MSG_OOB);
    sleep(3);

    // 2.3 再次发送3个普通数据
    write(sockfd, "ijk", 3);
    sleep(3);

    // 2.4 紧急数据每次只能发一个字节，此处"w"是紧急数据
    //     "yz"会被视为普通数据一并发过去
    send(sockfd, "yzw", 3, MSG_OOB);
    sleep(3);

    write(sockfd, "888", 3);
    return 0;
}
