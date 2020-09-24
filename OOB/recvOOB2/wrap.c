#include "wrap.h"

int Socket(int domain, int type, int protocol)
{
    int sockfd = socket(domain, type, protocol);
    if(sockfd == -1)
    {
        perror("创建UDP套接字失败");
        exit(0);
    }

    return sockfd;
}


int Bind(int sockfd, const struct sockaddr *addr,
                socklen_t addrlen)
{
    if(bind(sockfd, addr, addrlen) == -1)
    {
        perror("绑定地址失败");
        exit(0);
    }
    return 0;
}


int Connect(int sockfd, const struct sockaddr *addr,
                   socklen_t addrlen)
{
    int ret = connect(sockfd, addr, addrlen);
    if(ret != 0)
    {
        perror("连接失败");
        exit(0);
    }
    return ret;
}


int Listen(int sockfd, int backlog)
{
    int ret = listen(sockfd, backlog);
    if(ret != 0)
    {
        perror("设置监听失败");
        exit(0);
    }
    return ret;
}

int Accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
    int connfd = accept(sockfd, addr, addrlen);
    if(connfd == -1)
    {
        perror("接收连接失败");
        exit(0);
    }
    return connfd;
}
