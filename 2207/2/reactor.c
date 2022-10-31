#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUFFER_LENGTH 128
#define EVENTS_LENGTH 128

struct socket_item {
    int fd;

    char* r_buffer;
    int r_length;

    char* w_buffer;
    int w_length;

    int event;

    void (*recv_cb)(int fd, char* buffer, int length);
    void (*send_cb)(int fd, char* buffer, int length);
    void (*accept_cb)(int fd, char* buffer, int length);
};

struct reactor {
    int epoll_fd;
    struct socket_item* items;
};

int main() {
    setbuf(stdout, 0); // 奇怪有时候在某些windows电脑上使用不加这个会报错

    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd == -1) {
        return -1;
    }

    printf("listen_fd: %d\n", listen_fd);

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(19999);

    if (bind(listen_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) {
        return -2;
    }

    listen(listen_fd, 10);

    struct reactor* r = (struct reactor*)calloc(1, sizeof(struct reactor));
    if (r == NULL) {
        return -3;
    }
    r->items = (struct socket_item*)calloc(EVENTS_LENGTH, sizeof(struct socket_item));
    if (r->items == NULL) {
        free(r);
        return -4;
    }
    // epoll
    r->epoll_fd = epoll_create(1);

    struct epoll_event ev, epoll_events[EVENTS_LENGTH];
    ev.events = EPOLLIN;
    ev.data.fd = listen_fd;

    epoll_ctl(r->epoll_fd, EPOLL_CTL_ADD, listen_fd, &ev);
    printf("epoll_fd: %d\n", r->epoll_fd);

    while (1) {
        int n_ready = epoll_wait(r->epoll_fd, epoll_events, EVENTS_LENGTH, 1000);  // 0: 立即返回 -1: 阻塞, 0: 以上代表
        printf("------ n_ready: %d\n", n_ready);
        for (int i = 0; i < n_ready; ++i) {
            int client_fd = epoll_events[i].data.fd;

            if (listen_fd == client_fd) {  // accept
                struct sockaddr_in client;
                socklen_t len = sizeof(client);
                int conn_fd = accept(listen_fd, (struct sockaddr*)&client, &len);
                printf("accept\t conn_fd: %d\n", conn_fd);
                //                ev.events = EPOLLIN | EPOLLET;  // 水平触发 边沿触发
                ev.events = EPOLLIN;
                ev.data.fd = conn_fd;
                epoll_ctl(r->epoll_fd, EPOLL_CTL_ADD, conn_fd, &ev);

                r->items[conn_fd].fd = conn_fd;

                r->items[conn_fd].r_buffer = calloc(1, BUFFER_LENGTH);
                r->items->r_length = 0;

                r->items[conn_fd].w_buffer = calloc(1, BUFFER_LENGTH);
                r->items[conn_fd].w_length = 0;

                r->items[conn_fd].event = EPOLLIN;
            } else if (epoll_events[i].events & EPOLLIN) {  // receive
                char* r_buffer = r->items[client_fd].r_buffer;
                char* w_buffer = r->items[client_fd].w_buffer;
                int n = recv(client_fd, r_buffer, BUFFER_LENGTH, 0);
                if (n > 0) {
                    r_buffer[n] = '\0';
                    printf("recv: %s\n", r_buffer);

                    memcpy(w_buffer, r_buffer, BUFFER_LENGTH);

                    ev.events = EPOLLOUT;
                    ev.data.fd = client_fd;
                    epoll_ctl(r->epoll_fd, EPOLL_CTL_MOD, client_fd, &ev);
                    //                    send(client_fd, buffer, n, 0);
                } else if (n == 0) {
                    free(r_buffer);
                    free(w_buffer);

                    r->items[client_fd].fd = 0;

                    close(client_fd);
                }
            } else if (epoll_events[i].events & EPOLLOUT) {
                char* w_buffer = r->items[client_fd].w_buffer;
                int sent = send(client_fd, w_buffer, BUFFER_LENGTH, 0);
                printf("send: %d\n", sent);

                ev.events = EPOLLIN;
                ev.data.fd = client_fd;
                epoll_ctl(r->epoll_fd, EPOLL_CTL_MOD, client_fd, &ev);
            }
        }
    }

    return 0;
}