#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <array>
#include <iostream>

enum { EVENTS_LENGTH = 128, BUFFER_LENGTH = 128 };

int main() {
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd == -1) {
        return -1;
    }

    struct sockaddr_in server_addr {};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(19999);

    if (bind(listen_fd, reinterpret_cast<struct sockaddr *>(&server_addr),
             sizeof(server_addr)) == -1) {
        return -2;
    }

    listen(listen_fd, 10);

    int epoll_fd = epoll_create(1);

    struct epoll_event ev {};
    //    struct epoll_event events[EVENTS_LENGTH];
    std::array<epoll_event, EVENTS_LENGTH> events{};
    ev.events = EPOLLIN;
    ev.data.fd = listen_fd;

    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_fd, &ev);

    std::cout << "fd : " << epoll_fd << std::endl;

    while (true) {
        int n_ready = epoll_wait(epoll_fd, events.data(), EVENTS_LENGTH, -1);
        std::cout << "---------------------" << n_ready << std::endl;
        for (int i = 0; i < n_ready; ++i) {
            int client_fd = events[i].data.fd;

            if (listen_fd == client_fd) {  // accept
                struct sockaddr_in client {};
                socklen_t len = sizeof(client);
                int conn_fd =
                    accept(listen_fd,
                           reinterpret_cast<struct sockaddr *>(&client), &len);
                if (conn_fd == -1) {
                    break;
                }
                std::cout << "accept: " << conn_fd << std::endl;

                ev.events = EPOLLIN;
                ev.data.fd = conn_fd;
                epoll_ctl(epoll_fd, EPOLL_CTL_ADD, conn_fd, &ev);
            } else if (events[i].events & EPOLLIN) {
                //                char buffer[BUFFER_LENGTH] = {0};
                std::array<char, BUFFER_LENGTH> buffer{};
                size_t n = recv(client_fd, buffer.data(), BUFFER_LENGTH, 0);
                if (n > 0) {
                    buffer[n] = '\0';
                    std::cout << "recv: " << buffer.data() << " , n: " << n
                              << std::endl;
                    size_t n_send = send(client_fd, buffer.data(), n, 0);
                    std::cout << "send: " << n_send << std::endl;
                }
            }
        }
    }
}