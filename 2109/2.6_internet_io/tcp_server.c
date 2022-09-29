#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <sys/poll.h>
#include <sys/epoll.h>

#include <pthread.h>

#define MAXLINE 1024

#define POLL_SIZE 4096

void *client_routine(void *arg) 
{
    int connfd = *(int *)arg;

    char buff[MAXLINE];

    while (1) {
        
        int n = recv(connfd, buff, MAXLINE, 0);
        if (n > 0) {
            buff[n] = '\0';
            printf("recv msg from client: %s\n", buff);

            send(connfd, buff, n, 0);
        } else if (n == 0) {
            close(connfd);
            break;
        }
    }

    return NULL;
}

int main(int argc, char **argv)
{
    int listenfd, connfd, n;
    struct sockaddr_in serveraddr;
    char buff[MAXLINE];

    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        printf("create socket error: %s(errno: %d)\n", strerror(errno), errno);
        return 0;
    }

    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(9999);

    if (bind(listenfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) == -1) {
        printf("bind socket error: %s(errno: %d)\n", strerror(errno), errno);
        return 0;
    }

    if (listen(listenfd, 10) == -1) {
        printf("listen socket error: %s(errno: %d)\n", strerror(errno), errno);
        return 0;
    }

#if 0
    struct sockaddr_in client;
    socklen_t len = sizeof(client);
    if ((connfd = accept(listenfd, (struct sockaddr *)&client, &len)) == -1) {
        printf("accept socket error: %s(errno: %d)\n", strerror(errno), errno);
        return 0;
    }

    printf("========waiting for client's request========\n");

    while (1) {

        n = recv(connfd, buff, MAXLINE, 0);
        if (n > 0) {
            buff[n] = '\0';
            printf("recv msg from client: %s\n", buff);

            send(connfd, buff, n, 0);
        } else if (n == 0) {
            close(connfd);
        }

        // close(connfd);
    }

#elif 0

    printf("========waiting for client's request========\n");

    while (1) {
        struct sockaddr_in client;
        socklen_t len = sizeof(client);
        if ((connfd == accept(listenfd, (struct sockaddr *)&client, &len)) == -1) {
            printf("accept socket error: %s(errno: %d)\n", strerror(errno), errno);
            return 0;
        }

        n = recv(connfd, buff, MAXLINE, 0);
        if (n > 0) {
            buff[n] = '\0';
            printf("recv msg from client: %s\n", buff);

            send(connfd, buff, n, 0);
        } else if (n == 0) {
            close(connfd);
        }

    }

#elif 0
    while (1) {
        struct sockaddr_in client;
        socklen_t len = sizeof(client);
        if ((connfd = accept(listenfd, (struct sockaddr *)&client, &len)) == -1) {
            printf("accept socket error: %s(errno: %d)\n", strerror(errno), errno);
            return 0;
        }

        pthread_t threadid;
        pthread_create(&threadid, NULL, client_routine, (void *)&connfd);
    }
#elif 1
    // select
    // fd的集合, 连接的集合
    fd_set rfds, rset;

    FD_ZERO(&rfds); // 清空
    FD_SET(listenfd, rfds); // 

    int max_fd = listenfd;

    while (1) {
        rset = rfds;
        int nready = select(max_fd + 1, &rset, NULL, NULL, NULL);

        if (FD_ISSET(listenfd, &rset)) {
            struct sockaddr_in client;
            socklen_t len = sizeof(client);
            if ((connfd = accept(listenfd, (struct sockaddr *)&client, &len)) == -1) {
                printf("accept socket error: %s(errno: %d)\n", strerror(errno), errno);
                return 0;
            }
            
            FD_SET(connfd, &rfds);

            if (connfd > max_fd) max_fd = connfd; // 

            if (--nready == 0) continue;
        }

        int i = 0;
        for (i = listenfd + 1; i <= max_fd; i++) {
            if (FD_ISSET(i, &rset)) {
                n = recv(i, buff, MAXLINE, 0);
                if (n > 0) {
                    buff[0] = '\0';
                    printf("recv msg from client: %s\n", buff);
                    send(i, buff, n, 0);
                } else if (n == 0) {
                    // 关闭之后, 需要释放
                    FD_CLR(i, &rfds);
                    close(i);
                }
                
                if (--nready == 0) break;
            }
            
        }
    }



#else
#endif
    close(listenfd);
    return 0;
}
