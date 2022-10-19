#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define BUFFER_LENGTH 128
#define EVENTS_LENGTH 128

// listenfd, clientfd
struct sock_item {  // conn_item

    int fd;  // clientfd

    char *rbuffer;
    int rlength;  //

    char *wbuffer;
    int wlength;

    int event;

    void (*recv_cb)(int fd, char *buffer, int length);
    void (*send_cb)(int fd, char *buffer, int length);

    void (*accept_cb)(int fd, char *buffer, int length);
};

struct reactor {
    int epfd;  // epoll

    struct sock_item *items;
};

// thread --> fd
void *routine(void *arg) {
    int clientfd = *(int *)arg;

    while (1) {
        unsigned char buffer[BUFFER_LENGTH] = {0};
        int ret = recv(clientfd, buffer, BUFFER_LENGTH, 0);
        if (ret == 0) {
            close(clientfd);
            break;
        }
        printf("buffer : %s, ret: %d\n", buffer, ret);

        ret = send(clientfd, buffer, ret, 0);  //
    }
}

// socket -->
// bash --> execve("./server", "");
//
// 0, 1, 2
// stdin, stdout, stderr
int main() {
    // block
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);  //
    if (listenfd == -1)
        return -1;
    // listenfd
    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(19999);

    if (-1 == bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr))) {
        return -2;
    }

    listen(listenfd, 10);

    ///  **********
    struct reactor *r = (struct reactor *)calloc(1, sizeof(struct reactor));
    if (r == NULL) {
        return -3;
    }
    // memset();

    r->items =
        (struct sock_item *)calloc(EVENTS_LENGTH, sizeof(struct sock_item));
    if (r->items == NULL)
        return -4;

    // fd --> epoll
    r->epfd = epoll_create(1);

    /// ************** //

    struct epoll_event ev, events[EVENTS_LENGTH];
    ev.events = EPOLLIN;
    ev.data.fd = listenfd;  //

    epoll_ctl(r->epfd, EPOLL_CTL_ADD, listenfd, &ev);  //

    while (1) {  // 7 * 24

        int nready = epoll_wait(r->epfd, events, EVENTS_LENGTH, -1);  // -1, ms
        printf("------- %d\n", nready);

        int i = 0;
        for (i = 0; i < nready; i++) {
            int clientfd = events[i].data.fd;

            if (listenfd == clientfd) {  // accept

                struct sockaddr_in client;
                socklen_t len = sizeof(client);
                int connfd = accept(listenfd, (struct sockaddr *)&client, &len);
                if (connfd == -1)
                    break;

                printf("accept: %d\n", connfd);
                ev.events = EPOLLIN;
                ev.data.fd = connfd;
                epoll_ctl(r->epfd, EPOLL_CTL_ADD, connfd, &ev);

                r->items[connfd].fd = connfd;

                r->items[connfd].rbuffer = calloc(1, BUFFER_LENGTH);
                r->items[connfd].rlength = 0;

                r->items[connfd].wbuffer = calloc(1, BUFFER_LENGTH);
                r->items[connfd].wlength = 0;

                r->items[connfd].event = EPOLLIN;

            } else if (events[i].events & EPOLLIN) {  // clientfd

                // char rbuffer[BUFFER_LENGTH] = {0};

                char *rbuffer = r->items[clientfd].rbuffer;
                char *wbuffer = r->items[clientfd].wbuffer;

                int n = recv(clientfd, rbuffer, BUFFER_LENGTH, 0);
                if (n > 0) {
                    // rbuffer[n] = '\0';

                    printf("recv: %s, n: %d\n", rbuffer, n);

                    memcpy(wbuffer, rbuffer, BUFFER_LENGTH);

                    ev.events = EPOLLOUT;
                    ev.data.fd = clientfd;

                    epoll_ctl(r->epfd, EPOLL_CTL_MOD, clientfd, &ev);
                } else if (n == 0) {
                    free(rbuffer);
                    free(wbuffer);

                    r->items[clientfd].fd = 0;

                    close(clientfd);
                }

            } else if (events[i].events & EPOLLOUT) {
                char *wbuffer = r->items[clientfd].wbuffer;

                int sent = send(clientfd, wbuffer, BUFFER_LENGTH, 0);  //
                printf("sent: %d\n", sent);

                ev.events = EPOLLIN;
                ev.data.fd = clientfd;

                epoll_ctl(r->epfd, EPOLL_CTL_MOD, clientfd, &ev);
            }
        }
    }
}
