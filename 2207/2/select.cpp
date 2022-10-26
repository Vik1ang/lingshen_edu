#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <array>
#include <iostream>

#define BUFFER_LENGTH 128

void* routine(void* arg) {
    int client_fd = *static_cast<int*>(arg);

    while (true) {
        std::array<char, BUFFER_LENGTH> buffer{};
        int ret = recv(client_fd, buffer.data(), BUFFER_LENGTH, 0);
        if (ret == 0) {
            close(client_fd);
            break;
        }
        std::cout << "buffer: " << buffer.data() << " , ret: " << ret << std::endl;

        ret = send(client_fd, buffer.data(), ret, 0);
    }

    return nullptr;
}

int main() {
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd == -1) {
        return -1;
    }

    std::cout << "listen_fd: " << listen_fd << std::endl;

    struct sockaddr_in serv_addr {};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(19999);

    if (bind(listen_fd, reinterpret_cast<struct sockaddr*>(&serv_addr), sizeof(serv_addr)) == -1) {
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

    struct sockaddr_in client {};
    socklen_t len = sizeof(client);
    int client_fd = accept(listen_fd, reinterpret_cast<struct sockaddr*>(&client), &len);
    std::cout << "client_fd: " << client_fd << std::endl;

    while (true) {
        unsigned char buffer[128] = {0};
        int ret = recv(client_fd, buffer, BUFFER_LENGTH, 0);
        printf("buffer: %s, ret: %d\n", buffer, ret);

        ret = send(client_fd, buffer, ret, 0);
    }

#else

    while (true) {

        struct sockaddr_in client {};
        socklen_t len = sizeof(client);
        int client_fd = accept(listen_fd, reinterpret_cast<struct sockaddr*>(&client), &len);
        std::cout << "client_fd: " << client_fd << std::endl;

        pthread_t thread_id = 0;
        pthread_create(&thread_id, nullptr, routine, &client_fd);
    }

#endif

    return 0;
}