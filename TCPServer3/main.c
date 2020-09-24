#include "wrap.h"
#include "kernel_list.h"

// TCP聊天室服务器（多路复用）


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

// 向所有用户提示新用户newone的上线
// 并向该新用户告知其ID
void welcome(struct user *client, struct user *newone)
{
    struct list_head *pos;
    list_for_each(pos, &client->list)
    {
        struct user *p = list_entry(pos, struct user, list);

        char buf[50];
        bzero(buf, 50);

        // a.告知新用户其ID
        if(p->ID == newone->ID)
            snprintf(buf, 50, "你的ID是:%d\n", newone->ID);
        // b.告知其他用户该用户上线的消息
        else
            snprintf(buf, 50, "%d上线了！\n", newone->ID);

        write(p->connfd, buf, strlen(buf));
    }
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
        write(p->connfd, msg, strlen(msg));
    }
}

int main(int argc, char **argv) // ./main IP PORT
{
    if(argc != 3)
    {
        printf("参数错误！用法: <IP> <PORT>\n");
        exit(0);
    }

    // 1，创建TCP套接字
    int sockfd = Socket(AF_INET, SOCK_STREAM, 0);

    // 2，准备好地址结构体
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    bzero(&addr, len);

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(argv[1]);
    addr.sin_port = htons(atoi(argv[2]));

    // 3，绑定套接字和地址
    Bind(sockfd, (struct sockaddr *)&addr, len);

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
            // 接受连接请求，并产生一个新用户节点
            struct user *newone = calloc(1, sizeof(struct user));
            newone->connfd = accept(sockfd, (struct sockaddr *)&newone->addr, &len);

            // 将新用户节点链入用户链表
            list_add_tail(&newone->list, &clients->list);

            // 给新用户分配一个随机ID
            newone->ID = rand() % 10000;

            // 给新加入的用户发欢迎信息和当前用户
            // 并立刻告知新用户的ID
            welcome(clients, newone);

            // 服务端也做个简单的提示
            printf("【%s:%hu】%d上线了!\n", inet_ntoa(newone->addr.sin_addr),
                                           ntohs(newone->addr.sin_port),
                                            newone->ID);
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
                printf("%d发来消息:%s", p->ID, buf);
                broadcastMsg(clients, p, buf);
            }

            // 用户跑了（关闭了）
            if(m == 0)
            {
                // a.更新用户链表
                printf("%d下线了！\n", p->ID);
                list_del(pos);
                free(p);

                // b.通知其他人
            }
        }
    }

    // 释放资源

    return 0;
}
