#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUFFER_LENGTH    1024
#define MAX_EPOLL_EVENTS 1024
#define SERVER_PORT      19999
#define PORT_COUNT       100

typedef int N_CALLBACK(int, int, void*);

struct nty_event {
    int fd;
    int events;
    void* arg;
    int (*callback)(int fd, int events, void* arg);

    int status;
    char buffer[BUFFER_LENGTH];
    int length;
};

struct event_block {
    struct nty_event* events;
    struct event_block* next;
};

struct nty_reactor {
    int epoll_fd;
    int blk_cnt;

    struct event_block* ev_blk;
};

int recv_cb(int fd, int events, void* arg);
int send_cb(int fd, int events, void* arg);

void nty_event_set(struct nty_event* ev, int fd, N_CALLBACK callback, void* arg) {
    ev->fd = fd;
    ev->callback = callback;
    ev->events = 0;
    ev->arg = arg;

    return;
}

int nty_event_add(int ep_fd, int events, struct nty_event* ev) {
    struct epoll_event ep_ev = {0, {0}};
    ep_ev.data.ptr = ev;
    ep_ev.events = ev->events = events;

    int op = 0;
    if (ev->status == 1) {
        op = EPOLL_CTL_MOD;
    } else {
        op = EPOLL_CTL_ADD;
        ev->status = 1;
    }

    if (epoll_ctl(ep_fd, op, ev->fd, &ep_ev) < 0) {
        printf("event add failed [fd=%d], events[%d]\n", ev->fd, events);
        return -1;
    }

    return 0;
}

int nty_event_del(int ep_fd, struct nty_event* ev) {
    struct epoll_event ep_ev = {0, {0}};

    if (ev->status != 1) {
        return -1;
    }

    ep_ev.data.ptr = ev;
    ev->status = 0;
    epoll_ctl(ep_fd, EPOLL_CTL_DEL, ev->fd, &ep_ev);

    return 0;
}

int nty_reactor_alloc(struct nty_reactor* reactor) {
    if (reactor == NULL || reactor->ev_blk == NULL) {
        return -1;
    }

    struct event_block* blk = reactor->ev_blk;

    while (blk->next != NULL) {
        blk = blk->next;
    }

    struct nty_event* event = (struct nty_event*)malloc((MAX_EPOLL_EVENTS) * sizeof(struct nty_event));
    if (event == NULL) {
        printf("nty_reactor_alloc nty_event failed\n");
        return -2;
    }
    memset(event, 0, MAX_EPOLL_EVENTS * sizeof(struct nty_event));

    struct event_block* block = malloc(sizeof(struct event_block));
    if (block == NULL) {
        printf("nty_reactor_alloc event_block failed\n");
        return -3;
    }
    block->events = event;
    block->next = NULL;

    blk->next = block;
    reactor->blk_cnt++;

    return 0;
}

struct nty_event* nty_reactor_idx(struct nty_reactor* reactor, int sock_fd) {
    if (reactor == NULL || reactor->ev_blk == NULL) {
        return NULL;
    }

    int blk_idx = sock_fd / MAX_EPOLL_EVENTS;
    while (blk_idx >= reactor->blk_cnt) {
        nty_reactor_alloc(reactor);
    }

    int i = 0;
    struct event_block* blk = reactor->ev_blk;
    while (i++ != blk_idx && blk != NULL) {
        blk = blk->next;
    }

    return &blk->events[sock_fd % MAX_EPOLL_EVENTS];
}

int recv_cb(int fd, int events, void* arg) {
    struct nty_reactor* reactor = (struct nty_reactor*)arg;
    struct nty_event* ev = nty_reactor_idx(reactor, fd);

    if (ev == NULL) {
        return -1;
    }

    int len = recv(fd, ev->buffer, BUFFER_LENGTH, 0);
    nty_event_del(reactor->epoll_fd, ev);

    if (len > 0) {
        ev->length = len;
        ev->buffer[len] = '\0';

        printf("recv [%d]:%s\n", fd, ev->buffer);

        nty_event_set(ev, fd, send_cb, reactor);
        nty_event_add(reactor->epoll_fd, EPOLLOUT, ev);
    } else if (len == 0) {
        nty_event_del(reactor->epoll_fd, ev);
        printf("recv_cv --> disconnect\n");
        close(ev->fd);
    } else {
        if (errno == EAGAIN && errno == EWOULDBLOCK) {  //

        } else if (errno == ECONNRESET) {
            nty_event_del(reactor->epoll_fd, ev);
            close(ev->fd);
        }
        printf("recv[fd=%d] error[%d]:%s\n", fd, errno, strerror(errno));
    }

    return len;
}

int send_cb(int fd, int events, void* arg) {
    struct nty_reactor* reactor = (struct nty_reactor*)arg;
    struct nty_event* ev = nty_reactor_idx(reactor, fd);

    if (ev == NULL) {
        return -1;
    }

    int len = send(fd, ev->buffer, ev->length, 0);
    if (len > 0) {
        printf("send[fd=%d], [%d]%s\n", fd, len, ev->buffer);

        nty_event_del(reactor->epoll_fd, ev);
        nty_event_set(ev, fd, recv_cb, reactor);
        nty_event_add(reactor->epoll_fd, EPOLLIN, ev);
    } else {
        nty_event_del(reactor->epoll_fd, ev);
        close(ev->fd);

        printf("send[fd=%d] error %s\n", fd, strerror(errno));
    }

    return len;
}

int accept_cb(int fd, int events, void* arg) {
    struct nty_reactor* reactor = (struct nty_reactor*)arg;
    if (reactor == NULL) {
        return -1;
    }

    struct sockaddr_in client_addr;
    socklen_t len = sizeof(client_addr);

    int client_fd = 0;
    if ((client_fd = accept(fd, (struct sockaddr*)&client_addr, &len) == -1)) {
        if (errno != EAGAIN && errno != EINTR) {
        }
        printf("accept: %s\n", strerror(errno));
        return -1;
    }

    int flag = 0;
    if ((flag = fcntl(client_fd, F_SETFL, O_NONBLOCK)) < 0) {
        printf("%s: fcntl nonblocking failed, %d\n", __func__, MAX_EPOLL_EVENTS);
        return -1;
    }

    struct nty_event* event = nty_reactor_idx(reactor, client_fd);

    if (event == NULL) {
        return -1;
    }

    nty_event_set(event, client_fd, recv_cb, reactor);
    nty_event_add(reactor->epoll_fd, EPOLLIN, event);

    printf("new connect [%s:%d], pos[%d]\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), client_fd);

    return 0;
}

int nty_reactor_init(struct nty_reactor* reactor) {
    if (reactor == NULL) {
        return -1;
    }
    memset(reactor, 0, sizeof(struct nty_reactor));

    reactor->epoll_fd = epoll_create(1);
    if (reactor->epoll_fd <= 0) {
        printf("create epfd in %s err %s\n", __func__, strerror(errno));
        return -2;
    }

    struct nty_event* event = (struct nty_event*)malloc((MAX_EPOLL_EVENTS) * sizeof(struct nty_event));
    if (event == NULL) {
        printf("create epfd in %s err %s\n", __func__, strerror(errno));
        close(reactor->epoll_fd);
        return -3;
    }
    memset(event, 0, (MAX_EPOLL_EVENTS) * sizeof(struct nty_event));

    struct event_block* block = (struct event_block*)malloc(sizeof(struct event_block));
    if (block == NULL) {
        free(event);
        close(reactor->epoll_fd);
        return -3;
    }

    block->events = event;
    block->next = NULL;

    reactor->ev_blk = block;
    reactor->blk_cnt = 1;

    return 0;
}

int nty_reactor_destroy(struct nty_reactor* reactor) {
    close(reactor->epoll_fd);

    struct event_block* blk = reactor->ev_blk;
    struct event_block* next = NULL;
    while (blk != NULL) {
        next = blk->next;
        free(blk->events);
        free(blk);

        blk = next;
    }

    return 0;
}

int nty_reactor_add_listener(struct nty_reactor* reactor, int sock_fd, N_CALLBACK* acceptor) {
    if (reactor == NULL || reactor->ev_blk == NULL) {
        return -1;
    }

    struct nty_event* event = nty_reactor_idx(reactor, sock_fd);
    if (event == NULL) {
        return -1;
    }

    nty_event_set(event, sock_fd, acceptor, reactor);
    nty_event_add(reactor->epoll_fd, EPOLLIN, event);

    return 0;
}

int nty_reactor_run(struct nty_reactor* reactor) {
    if (reactor == NULL || reactor->epoll_fd < 0 || reactor->ev_blk == NULL) {
        return -1;
    }

    struct epoll_event events[MAX_EPOLL_EVENTS + 1];

    int check_pos = 0;

    while (1) {
        int n_ready = epoll_wait(reactor->epoll_fd, events, MAX_EPOLL_EVENTS, 1000);
        if (n_ready < 0) {
            printf("epoll_wait error, exit\n");
            continue;
        }

        for (int i = 0; i < n_ready; ++i) {
            struct nty_event* event = (struct nty_event*)events[i].data.ptr;

            if ((events[i].events & EPOLLIN) && (event->events & EPOLLIN)) {
                event->callback(event->fd, events[i].events, event->arg);
            } else if ((events[i].events & EPOLLOUT) && (event->events & EPOLLOUT)) {
                event->callback(event->fd, events[i].events, event->arg);
            }
        }
    }
}

int init_sock(short port) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    fcntl(fd, F_SETFL, O_NONBLOCK);

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);

    bind(fd, (struct sockaddr*)&server_addr, sizeof(server_addr));

    if (listen(fd, 20) < 0) {
        printf("listen failed : %s\n", strerror(errno));
        return -1;
    }

    printf("listen server port : %d\n", port);
    return fd;
}

int main(int argc, char* argv[]) {
    struct nty_reactor* reactor = (struct nty_reactor*)malloc(sizeof(struct nty_reactor));
    nty_reactor_init(reactor);

    unsigned short port = SERVER_PORT;
    if (argc == 2) {
        port = atoi(argv[1]);
    }

    int sock_fds[PORT_COUNT] = {0};

    for (int i = 0; i < PORT_COUNT; ++i) {
        sock_fds[i] = init_sock(port + i);
        nty_reactor_add_listener(reactor, sock_fds[i], accept_cb);
    }

    nty_reactor_run(reactor);
    nty_reactor_destroy(reactor);

    for (int i = 0; i < PORT_COUNT; i++) {
        close(sock_fds[i]);
    }
    free(reactor);

    return 0;
}