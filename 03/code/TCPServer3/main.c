#include "wrap.h"
#include "kernel_list.h"

// TCP聊天室服务器（多路复用）
#define IN  0
#define OUT 1

// 用户节点
struct user
{
    int ID;
    int connfd;
    struct sockaddr_in addr;

    struct list_head list;
};

// 初始化一个空的用户链表
struct user *init_list()
{
    struct user *clients = calloc(1, sizeof(struct user));
    if(clients != NULL)
    {
        INIT_LIST_HEAD(&clients->list);
    }
    return clients;
}

// 有人上线时，更新用户列表并通知所有人
// 有人下线时，更新用户列表并通知所有人
void inform(struct user *clients, int ID, int how)
{
    char *allClients = calloc(1, 2048);
    snprintf(allClients+strlen(allClients), 2048, "\n===========\n");
    snprintf(allClients+strlen(allClients), 2048, "当前活跃用户:\n");

    // 将所有的现存的用户ID放入一个列表中
    struct user *p;
    list_for_each_entry(p, &clients->list, list)
    {
        if(p->ID == ID && how == IN)
            snprintf(allClients+strlen(allClients), 2048, "[%d](新人，欢迎你！)\n", ID);
        else if(p->ID == ID && how == OUT)
            snprintf(allClients+strlen(allClients), 2048, "[%d](下线，再见！)\n", ID);
        else
            snprintf(allClients+strlen(allClients), 2048, "[%d]\n", p->ID);
    }
    snprintf(allClients+strlen(allClients), 2048, "===========\n\n");

    // 将当前服务器上的活跃用户列表发给所有人
    list_for_each_entry(p, &clients->list, list)
    {
        write(p->connfd, allClients, strlen(allClients));
    }
    printf("%s", allClients);

    free(allClients);
#ifdef DEBUG
    printf("[%s:%d] 更新用户列表并通知完毕\n", __FUNCTION__, __LINE__);
#endif
}

// 将用户p发来的消息，群发给其他人
void broadcastMsg(struct user *clients, struct user *sender, const char *msg)
{
    struct list_head *pos;
    list_for_each(pos, &clients->list)
    {
        struct user *p = list_entry(pos, struct user, list);

        // 跳过信息发送者本人
        if(sender->ID == p->ID)
        {
            continue;
        }

        // 群发用户的消息msg
        int n = write(p->connfd, msg, strlen(msg));

#ifdef DEBUG
        printf("已向[%d]发送消息%d个字节消息:%s\n", p->ID, n, msg);
#endif
    }

}

// 私聊消息
void privateTalk(struct user *clients, struct user *sender,
                 int recvID, const char *msg)
{
    printf("私聊消息:%s", msg);
    struct user *p;
    list_for_each_entry(p, &clients->list, list)
    {
        if(p->ID == recvID)
        {
            int n = write(p->connfd, msg, strlen(msg));
#ifdef DEBUG
            printf("已向[%d]发送消息%d个字节消息:%s\n", p->ID, n, msg);
#endif
            return;
        }
    }

    // 找不到信息的接收者，要发送一个OOB通知发送者该情况
    send(sender->connfd, "x", 1, MSG_OOB);
}

// 聊天室服务器
int main(int argc, char **argv) // ./main IP PORT
{
    if(argc != 3)
    {
        printf("参数错误！用法: <IP> <PORT>\n");
        exit(0);
    }

    // 1，创建TCP套接字
    int sockfd = Socket(AF_INET, SOCK_STREAM, 0);
#ifdef DEBUG
    printf("[%s:%d] 创建套接字成功\n", __FUNCTION__, __LINE__);
#endif

    int on = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    // 2，准备好地址结构体
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    bzero(&addr, len);

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(argv[1]);
    addr.sin_port = htons(atoi(argv[2]));

    // 3，绑定套接字和地址
    Bind(sockfd, (struct sockaddr *)&addr, len);
#ifdef DEBUG
    printf("[%s:%d] 绑定地址成功\n", __FUNCTION__, __LINE__);
#endif

    // 4，将套接字设置为监听状态，顺便设置最大同时连接个数
    listen(sockfd, 3);

    // 5，准备好用户链表
    struct user *clients = init_list();
    struct list_head *pos;
    struct user *p;

    // 6，循环多路监控所有的套接字
    fd_set rset; // 读就绪套接字集合
    fd_set wset; // 写就绪套接字集合
    fd_set eset; // 异常就绪套接字集合

    while(1)
    {
        FD_ZERO(&rset);
        FD_ZERO(&wset);
        FD_ZERO(&eset);

        // 将监听套接字，置入rset中等待读就绪
        int maxfd = sockfd;
        FD_SET(sockfd, &rset);

        // 将用户链表中的所有connfd，置入rset中等待读就绪
        list_for_each(pos, &clients->list)
        {
            p = list_entry(pos, struct user, list);
            FD_SET(p->connfd, &rset);
            maxfd = (maxfd>p->connfd) ? maxfd : p->connfd;
        }

        // 同时多路监控所有集合中的所有套接字
        select(maxfd+1, &rset, &wset, &eset, NULL);

        // a.判断是否有新的连接
        socklen_t len;
        if(FD_ISSET(sockfd, &rset))
        {
            #ifdef DEBUG
                printf("[%s:%d] 收到新连接请求\n", __FUNCTION__, __LINE__);
            #endif

            // 接受连接请求，并产生一个新用户节点
            struct user *newone = calloc(1, sizeof(struct user));
            newone->connfd = accept(sockfd, (struct sockaddr *)&newone->addr, &len);

            // 给新用户分配一个随机ID
            newone->ID = rand() % 10000;

            // 将新用户节点链入用户链表
            list_add_tail(&newone->list, &clients->list);

            // 给新加入的用户发欢迎信息和当前用户
            // 并立刻告知新用户的ID
            inform(clients, newone->ID, IN);
        }

        // b.判断是否有用户发来数据
        struct list_head *n;
        list_for_each_safe(pos, n, &clients->list)
        {
            p = list_entry(pos, struct user, list);

            // 判断每一个用户是否已发来数据
            if(!FD_ISSET(p->connfd, &rset))
                continue;

            char buf[100];
            bzero(buf, 100);
            int m = read(p->connfd, buf, 100);

            // 将数据群发给其他所有人
            if(m > 0)
            {
                char *msg = strstr(buf, ":");

                // a.群发消息
                if(msg == NULL)
                {
                    #ifdef DEBUG
                        printf("[%s:%d] 收到群发消息\n", __FUNCTION__, __LINE__);
                    #endif

                    broadcastMsg(clients, p, buf);
                }

                // b.私聊消息
                else
                {
                    #ifdef DEBUG
                        printf("[%s:%d] 收到私聊消息\n", __FUNCTION__, __LINE__);
                    #endif

                    privateTalk(clients, p, atoi(buf), msg+1);
                }
            }

            // 用户跑了（关闭了）
            if(m == 0)
            {
                // a.通知所有客户端当前的活跃用户列表
                inform(clients, p->ID, OUT);

                // b.更新用户链表
                printf("%d下线了！\n", p->ID);
                list_del(pos);
                free(p);
#ifdef DEBUG
                printf("[%s:%d] 删除用户完毕\n", __FUNCTION__, __LINE__);
#endif
            }
        }
    }

    // 释放资源

    return 0;
}
