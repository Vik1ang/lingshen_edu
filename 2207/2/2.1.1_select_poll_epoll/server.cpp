#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <array>
#include <cstring>
#include <iostream>
#include <thread>
#include <vector>

enum { BUFFER_LENGTH = 128 };

/*void* routine(void *arg){
    int client_fd = *static_cast<int*>(arg);

    while (1) {
        std::array<unsigned char, BUFFER_LENGTH> buffer = {0};
        size_t ret = recv(client_fd, buffer.data(), BUFFER_LENGTH, 0);
        std::cout << "buffer : " << buffer.data() << ", ret : " << ret
                  << std::endl;
        // send 返回大于0 不等于发送成功, 所以可能会丢失消息
        ret = send(client_fd, buffer.data(), ret, 0);
        //        std::cout << "ret : " << ret << std::endl;
    }
}*/

void my_routine(int &client_fd) {
    while (true) {
        std::array<unsigned char, BUFFER_LENGTH> buffer = {0};
        size_t ret = recv(client_fd, buffer.data(), BUFFER_LENGTH, 0);
        if (ret == 0) {
            close(client_fd);
            break;
        }
        std::cout << "buffer : " << buffer.data() << ", ret : " << ret
                  << std::endl;

        // send 返回大于0 不等于发送成功, 所以可能会丢失消息
        send(client_fd, buffer.data(), ret, 0);
        //        std::cout << "ret : " << ret << std::endl;
    }
}

int main() {
    // 默认是阻塞的
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd == -1) {
        std::cout << "create socket error: " << strerror(errno) << "errno"
                  << errno << std::endl;
        return 0;
    }

    struct sockaddr_in serv_addr {};
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr =
        htonl(INADDR_ANY);  // 其实对于0.0.0.0转不转没有所谓
    serv_addr.sin_port = htons(19999);  // 端口随便填

    if (bind(listen_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) ==
        -1) {
        std::cout << "bind socket error: " << strerror(errno) << "errno"
                  << errno << std::endl;
        return 0;
    }

    if (listen(listen_fd, 10) == -1) {
        std::cout << "listen socket error: " << strerror(errno) << "errno"
                  << errno << std::endl;
        return 0;
    }

#if 0
    int flag = fcntl(listen_fd, F_GETFL, 0);
    flag |= O_NONBLOCK;
    fcntl(listen_fd, F_SETFL, flag);
#endif

#if 0
    //
    struct sockaddr_in client {};
    socklen_t len = sizeof(client);
    int client_fd =
        accept4(listen_fd, reinterpret_cast<struct sockaddr*>(&client), &len,
                SOCK_CLOEXEC);
    while (1) {
        std::array<unsigned char, BUFFER_LENGTH> buffer = {0};
        size_t ret = recv(client_fd, buffer.data(), BUFFER_LENGTH, 0);
        std::cout << "buffer : " << buffer.data() << ", ret : " << ret
                  << std::endl;

        // send 返回大于0 不等于发送成功, 所以可能会丢失消息
        ret = send(client_fd, buffer.data(), ret, 0);
        //        std::cout << "ret : " << ret << std::endl;
    }
    return 0;
#elif 0
    while (1) {
        struct sockaddr_in client {};
        socklen_t len = sizeof(client);
        int client_fd = accept(
            listen_fd, reinterpret_cast<struct sockaddr *>(&client), &len);
        std::thread t([&] { my_routine(client_fd); });
        t_pool.push_back(std::move(t));
    }
#else

    fd_set read_fds, write_fds, r_set, w_set;

    FD_ZERO(&read_fds);
    FD_ZERO(&write_fds);
    FD_SET(listen_fd, &read_fds);

    int max_fd = listen_fd;

    while (1) {
        r_set = read_fds;
        w_set = write_fds;

        int n_ready = select(max_fd + 1, &r_set, &w_set, nullptr, nullptr);
        if (FD_ISSET(listen_fd, &r_set)) {
            std::cout << "listen_fd --> " << std::endl;
            struct sockaddr_in client;
            socklen_t len = sizeof(client);
            int client_fd = accept(listen_fd, (struct sockaddr *)&client, &len);
            if (client_fd == -1) {
                printf("accept socket error: %s(errno: %d)\n", strerror(errno),
                       errno);
                return 0;
            }
            FD_SET(client_fd, &read_fds);
            if (client_fd > max_fd) {
                max_fd = client_fd;
            }
            //            std::array<unsigned char, BUFFER_LENGTH> buffer = {0};
            char buffer[BUFFER_LENGTH] = {0};
            size_t ret = 0;
            int i = 0;
            for (i = listen_fd + 1; i <= max_fd; i++) {
                if (FD_ISSET(i, &r_set)) {
                    ret = recv(i, buffer, BUFFER_LENGTH, 0);
                    std::cout << "buffer : " << buffer << ", ret : " << ret
                              << std::endl;
                    if (ret == 0) {
                        FD_CLR(i, &read_fds);
                        std::cout << "Disconnect" << std::endl;
                        close(i);
                    }
                    //                    FD_SET(i, &write_fds);
                    if (--n_ready == 0) {
                        break;
                    }
                } else if (FD_ISSET(i, &w_set)) {
                    ret = send(i, buffer, ret, 0);
                }
            }
        }
    }

#endif

    close(listen_fd);
    return 0;
}