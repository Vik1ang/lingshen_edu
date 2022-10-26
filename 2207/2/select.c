#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUFFER_LENGTH 128

void* routine(void* arg) {
    int client_fd = *(int*)arg;

    while (1) {
        unsigned char buffer[BUFFER_LENGTH] = {0};
        int ret = recv(client_fd, buffer, BUFFER_LENGTH, 0);
        if (ret == 0) {
            close(client_fd);
            break;
        }
        printf("buffer: %s, ret: %d\n", buffer, ret);

        ret = send(client_fd, buffer, ret, 0);
    }
}

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

#if 0
    // 默认是 block的
    int flag = fcntl(listen_fd, F_GETFL, 0);
    flag |= O_NONBLOCK;
    fcntl(listen_fd, F_SETFL, flag);
#endif

    listen(listen_fd, 10);

#if 0

    // 单线程
    struct sockaddr_in client;
    socklen_t len = sizeof(client);
    int client_fd = accept(listen_fd, (struct sockaddr*)&client, &len);
    printf("client_fd: %d\n", client_fd);


    while (1) {
        unsigned char buffer[128] = {0};
        int ret = recv(client_fd, buffer, BUFFER_LENGTH, 0);
        printf("buffer: %s, ret: %d\n", buffer, ret);

        ret = send(client_fd, buffer, ret, 0);
    }

#elif 0

    // 多线程

    while (1) {
        struct sockaddr_in client;
        socklen_t len = sizeof(client);
        int client_fd = accept(listen_fd, (struct sockaddr*)&client, &len);

        pthread_t thread_id = 0;
        pthread_create(&thread_id, NULL, routine, &client_fd);
    }

#else
    // select
    // IO 多路复用
    // 检查 IO 是否有事件
    fd_set r_fds, w_fds, r_set, w_set;

    FD_ZERO(&r_fds);
    FD_SET(listen_fd, &r_fds);

    FD_ZERO(&w_fds);

    int max_fd = listen_fd;
    while (1) {
        r_set = r_fds;
        w_set = w_fds;

        int n_ready = select(max_fd + 1, &r_set, &w_set, NULL, NULL);
        if (FD_ISSET(listen_fd, &r_set)) {
            printf("listen_fd --->\n");

            struct sockaddr_in client;
            socklen_t len = sizeof(client);
            int client_fd = accept(listen_fd, (struct sockaddr*)&client, &len);
            printf("client_fd: %d\n", client_fd);

            FD_SET(client_fd, &r_fds);

            if (client_fd > max_fd) {
                max_fd = client_fd;
            }
        }

        for (int i = listen_fd + 1; i <= max_fd; i++) {
            if (FD_ISSET(i, &r_set)) {  // 可读
                unsigned char buffer[BUFFER_LENGTH] = {0};
                int ret = recv(i, buffer, BUFFER_LENGTH, 0);
                if (ret == 0) {
                    close(i);
                }
                printf("buffer: %s, ret: %d\n", buffer, ret);
                ret = send(i, buffer, ret, 0);
            }
        }
    }

#endif

    return 0;
}
