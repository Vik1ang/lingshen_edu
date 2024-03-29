#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUFFER_LENGTH 128
#define EVENTS_LENGTH 128
#define ITEM_LENGTH   128
#define PORT_COUNT    100

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

struct event_block {
    struct socket_item* items;
    struct event_block* next;
};

struct reactor {
    int epoll_fd;

    struct event_block* ev_blk;

    int blk_cnt;
};

int reactor_malloc(struct reactor* r) {  // new event block
    if (r == NULL) {
        return -1;
    }

    struct event_block* blk = r->ev_blk;

    while (blk != NULL && blk->next != NULL) {
        blk = blk->next;
    }

    struct socket_item* item = (struct socket_item*)malloc(ITEM_LENGTH * sizeof(struct socket_item));
    if (item == NULL) {
        return -4;
    }
    memset(item, 0, ITEM_LENGTH * sizeof(struct socket_item));

    struct event_block* block = malloc(sizeof(struct event_block));
    if (block == NULL) {
        free(item);
        return -5;
    }
    memset(block, 0, sizeof(struct event_block));

    block->items = item;
    block->next = NULL;

    if (blk == NULL) {
        r->ev_blk = block;
    } else {
        blk->next = block;
    }
    r->blk_cnt++;

    return 0;
}

struct socket_item* reactor_lookup(struct reactor* r, int socket_fd) {
    if (r == NULL || socket_fd <= 0) {
        return NULL;
    }

    printf("reactor_lookup --> %d\n", r->blk_cnt);

    int blk_idx = socket_fd / ITEM_LENGTH;

    while (blk_idx >= r->blk_cnt) {
        reactor_malloc(r);
    }

    struct event_block* blk = r->ev_blk;
    int i = 0;
    while (i++ < blk_idx && blk != NULL) {
        blk = blk->next;
    }

    return &blk->items[socket_fd % ITEM_LENGTH];
}

int is_listen_fd(int* fds, int conn_fd) {
    for (int i = 0; i < PORT_COUNT; ++i) {
        if (fds[i] == conn_fd) {
            return 1;
        }
    }
    return 0;
}

int init_server(short port) {
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd == -1) {
        return -1;
    }

    //    printf("listen_fd: %d\n", listen_fd);

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(port);

    if (bind(listen_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) {
        return -2;
    }

#if 1
    int flag = fcntl(listen_fd, F_GETFL, 0);
    flag |= O_NONBLOCK;
    fcntl(listen_fd, F_SETFL, flag);
#endif

    listen(listen_fd, 10);

    return listen_fd;
}

int main() {
    setbuf(stdout, 0);  // 奇怪有时候在某些windows电脑上使用不加这个会报错

#if 0
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

    r->epoll_fd = epoll_create(1);

    struct epoll_event ev, epoll_events[EVENTS_LENGTH];
    ev.events = EPOLLIN;
    ev.data.fd = listen_fd;
#else

    struct reactor* r = (struct reactor*)calloc(1, sizeof(struct reactor));
    if (r == NULL) {
        return -3;
    }

    r->epoll_fd = epoll_create(1);

    struct epoll_event ev, epoll_events[EVENTS_LENGTH];

    int socket_fds[PORT_COUNT] = {0};
    for (int i = 0; i < PORT_COUNT; ++i) {
        socket_fds[i] = init_server(19999 + i);
        ev.events = EPOLLIN;
        ev.data.fd = socket_fds[i];
        epoll_ctl(r->epoll_fd, EPOLL_CTL_ADD, socket_fds[i], &ev);
    }
#endif

    /*r->items = (struct socket_item*)calloc(EVENTS_LENGTH, sizeof(struct socket_item));
    if (r->items == NULL) {
        free(r);
        return -4;
    }*/

    // epoll

    printf("epoll_fd: %d\n", r->epoll_fd);

    while (1) {
        int n_ready = epoll_wait(r->epoll_fd, epoll_events, EVENTS_LENGTH, -1);  // 0: 立即返回 -1: 阻塞, 0: 以上代表
        //        printf("------ n_ready: %d\n", n_ready);
        for (int i = 0; i < n_ready; ++i) {
            int client_fd = epoll_events[i].data.fd;

            if (is_listen_fd(socket_fds, client_fd)) {  // accept
                struct sockaddr_in client;
                socklen_t len = sizeof(client);
                int conn_fd = accept(client_fd, (struct sockaddr*)&client, &len);
                if (conn_fd <= 0) {
                    break;
                }
                if (conn_fd % 19999 == 9999) {
                    printf("accept\t conn_fd: %d\n", conn_fd);
                }
                //                printf("accept\t conn_fd: %d\n", conn_fd);
                //                ev.events = EPOLLIN | EPOLLET;  // 水平触发 边沿触发

#if 1
                int flag = fcntl(conn_fd, F_GETFL, 0);
                flag |= O_NONBLOCK;
                fcntl(conn_fd, F_SETFL, flag);
#endif

                ev.events = EPOLLIN;
                ev.data.fd = conn_fd;
                epoll_ctl(r->epoll_fd, EPOLL_CTL_ADD, conn_fd, &ev);

                /*int flag = fcntl(conn_fd, F_GETFL, 0);
                flag |= O_NONBLOCK;
                fcntl(conn_fd, F_SETFL, flag);*/

#if 0
                r->items[conn_fd].fd = conn_fd;

                r->items[conn_fd].r_buffer = calloc(1, BUFFER_LENGTH);
                r->items->r_length = 0;

                r->items[conn_fd].w_buffer = calloc(1, BUFFER_LENGTH);
                r->items[conn_fd].w_length = 0;

                r->items[conn_fd].event = EPOLLIN;
#else
                struct socket_item* item = reactor_lookup(r, conn_fd);
                item->fd = conn_fd;
                item->r_buffer = calloc(1, BUFFER_LENGTH);
                item->r_length = 0;
                item->w_buffer = calloc(1, BUFFER_LENGTH);
                item->w_length = 0;
                item->event = EPOLLIN;
#endif
            } else if (epoll_events[i].events & EPOLLIN) {  // receive
#if 0
                char* r_buffer = r->items[client_fd].r_buffer;
                char* w_buffer = r->items[client_fd].w_buffer;
#else
                struct socket_item* item = reactor_lookup(r, client_fd);
                char* r_buffer = item->r_buffer;
                char* w_buffer = item->w_buffer;
#endif
                int n = recv(client_fd, r_buffer, BUFFER_LENGTH, 0);
                if (n > 0) {
                    //                    r_buffer[n] = '\0';
                    printf("recv: %s\n", r_buffer);

                    memcpy(w_buffer, r_buffer, BUFFER_LENGTH);

                    ev.events = EPOLLOUT;
                    ev.data.fd = client_fd;

                    epoll_ctl(r->epoll_fd, EPOLL_CTL_MOD, client_fd, &ev);
                    //                    send(client_fd, buffer, n, 0);
                } else if (n == 0) {
                    free(r_buffer);
                    free(w_buffer);

#if 0
                    r->items[client_fd].fd = 0;
#else
                    item->fd = 0;
#endif
                    close(client_fd);
                }
            } else if (epoll_events[i].events & EPOLLOUT) {
#if 0
                char* w_buffer = r->items[client_fd].w_buffer;
#else
                struct socket_item* item = reactor_lookup(r, client_fd);
                char* w_buffer = item->w_buffer;
#endif
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