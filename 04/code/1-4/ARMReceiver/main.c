#include "wrap.h"

int main(int argc, char **argv)
{
    if(argc != 3)
    {
        printf("参数错误！用法:<IP> <PORT>\n");
        exit(0);
    }

    // 1，创建UDP通信端点
    int sockfd = Socket(AF_INET, SOCK_DGRAM, 0);

    // 2，准备开发板的地址（IP+PORT）并绑定好socket
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    bzero(&addr, len);

    addr.sin_family = AF_INET;  // 指定协议族，这里是IPv4地址
    addr.sin_addr.s_addr = inet_addr(argv[1]); // IP地址
    addr.sin_port = htons(atoi(argv[2]));      // PORT端口号

    Bind(sockfd, (struct sockaddr *)&addr, len);

    // 3，等待ubuntu发来文件的属性信息
    struct fileinfo finfo;
    struct sockaddr_in ubuntuAddr;
    len = sizeof(ubuntuAddr);
    while(1)
    {
        bzero(&finfo, sizeof(finfo));
        bzero(&ubuntuAddr, len);
        recvfrom(sockfd, &finfo, sizeof(finfo), 0,
                    (struct sockaddr *)&ubuntuAddr, &len);

        printf("ubuntu发来文件:\n");
        printf("名字:%s\n, 大小:%d\n", finfo.name, finfo.size);

        printf("是否接收?[y/n]\n");
        char response = getchar();
        getchar(); // 吃掉回车符

        // 给ubuntu发去确认信息:y/n
        sendto(sockfd, &response, 1, 0,
                (struct sockaddr *)&ubuntuAddr, len);

        if(response == 'n')
            continue;


        // 开始接收ubuntu的文件内容
        int fd = open(finfo.name, O_WRONLY|O_TRUNC|O_CREAT, 0666);

        char buf[1024];
        int total = 0;
        while(total < finfo.size)
        {
            bzero(buf, 1024);
            int n = recvfrom(sockfd, buf, 1024, 0, NULL, NULL);

            if(n == -1)
            {
                perror("接收文件内容失败");
                exit(0);
            }

            total += n;
            write(fd, buf, n);
        }

        close(fd);
        printf("接收完毕\n");
    }

    return 0;
}
