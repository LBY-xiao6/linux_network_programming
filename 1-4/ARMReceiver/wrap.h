#ifndef WRAP_H
#define WRAP_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>

#include <linux/fb.h>
#include <linux/input.h>

#include <string.h>
#include <errno.h>
#include <time.h>
#include <wait.h>
#include <fcntl.h>

#include <dirent.h>

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/socket.h>
#include <math.h>
#include <pthread.h>
#include <semaphore.h>

// 网络相关头文件
#include <arpa/inet.h>
#include <sys/socket.h>

int Socket(int domain, int type, int protocol);
int Bind(int sockfd, const struct sockaddr *addr,
                socklen_t addrlen);
int Connect(int sockfd, const struct sockaddr *addr,
                   socklen_t addrlen);
int Listen(int sockfd, int backlog);
int Accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);

struct fileinfo
{
    char name[50];
    int  size;
};

#endif
