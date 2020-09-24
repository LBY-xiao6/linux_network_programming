#include "wrap.h"

void readfile(char *content, const char *filename, int size)
{
    int fd = open(filename, O_RDONLY);
    if(fd == -1)
    {
        perror("文件无法打开");
        exit(0);
    }

    int total = 0;
    while(total < size)
    {
        int n = read(fd, content+total, size);
        if(n == -1)
        {
            perror("读取文件失败");
            exit(0);
        }

        total += n;
    }

    close(fd);
}

int main(int argc, char **argv)
{
    if(argc != 3)
    {
        printf("参数错误！用法:<IP> <PORT>\n");
        exit(0);
    }

    // 1，创建UDP通信端点
    int sockfd = Socket(AF_INET, SOCK_DGRAM, 0);

    // 2，准备开发板的地址（IP+PORT）
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    bzero(&addr, len);

    addr.sin_family = AF_INET;  // 指定协议族，这里是IPv4地址
    addr.sin_addr.s_addr = inet_addr(argv[1]); // IP地址
    addr.sin_port = htons(atoi(argv[2]));      // PORT端口号

    // 3，选择要发送给开发板的文件
    char filename[50];
    bzero(filename, 50);
    gets (filename);

    // 4，获取文件属性信息并发送给开发板
    struct stat info;
    bzero(&info, sizeof(info));
    stat(filename, &info);

    struct fileinfo finfo;
    bzero(&finfo, sizeof(finfo));
    strncpy(finfo.name, filename, 50);
    finfo.size = info.st_size;

    printf("名字:%s\n大小:%d\n", finfo.name, finfo.size);
    int n = sendto(sockfd, &finfo, sizeof(finfo), 0,
                   (struct sockaddr *)&addr, len);
    if(n > 0)
        printf("发送文件属性信息成功，发了%d个字节\n", n);
    else
    {
        perror("发送文件属性信息失败");
        exit(0);
    }

    // 5，等待开发板的确认
    char response;
    recvfrom(sockfd, &response, 1, 0, NULL, NULL);
    if(response == 'n')
        return 0;

    // 6，将文件内容放入content中
    char *content = calloc(1, finfo.size);
    readfile(content, finfo.name, finfo.size);

    // 7，开始给开发板发送文件的内容
    int total  = 0;
    while(total < finfo.size)
    {
        int remain = finfo.size-total;
        int m = remain>1024 ? 1024 : remain;

        // 每次最多发1k数据量
        total += sendto(sockfd, content+total, m, 0,
                        (struct sockaddr *)&addr, len);
    }

    printf("发送完毕\n");
    close(sockfd);

    return 0;
}
