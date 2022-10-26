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

#else

    while (1) {
        struct sockaddr_in client;
        socklen_t len = sizeof(client);
        int client_fd = accept(listen_fd, (struct sockaddr*)&client, &len);

        pthread_t thread_id = 0;
        pthread_create(&thread_id, NULL, routine, &client_fd);
    }

#endif

    return 0;
}
