#include <netinet/in.h>
#include <stdio.h>
#include <sys/epoll.h>
#include <sys/socket.h>

#define BUFFER_LENGTH 128
#define EVENTS_LENGTH 128

int main() {
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

    // epoll
    int epoll_fd = epoll_create(1);

    struct epoll_event ev, epoll_events[EVENTS_LENGTH];
    ev.events = EPOLLIN;
    ev.data.fd = listen_fd;

    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_fd, &ev);
    printf("epoll_fd: %d\n", epoll_fd);

    while (1) {
        int n_ready = epoll_wait(epoll_fd, epoll_events, EVENTS_LENGTH, 0);  // 0: 立即返回 -1: 阻塞, 0: 以上代表
        printf("------ n_ready: %d\n", n_ready);
        for (int i = 0; i < n_ready; ++i) {
            int client_fd = epoll_events[i].data.fd;

            if (listen_fd == client_fd) {  // accept
                struct sockaddr_in client;
                socklen_t len = sizeof(client);
                int connection_fd = accept(listen_fd, (struct sockaddr*)&client, &len);
                printf("accept\t connection_fd: %d\n", connection_fd);
                ev.events = EPOLLIN;
                ev.data.fd = connection_fd;
                epoll_ctl(epoll_fd, EPOLL_CTL_ADD, connection_fd, &ev);
            }
        }
    }

    return 0;
}