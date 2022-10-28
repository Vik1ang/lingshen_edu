#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>

#include <array>
#include <cstring>
#include <iostream>

#define BUFFER_LENGTH 128
#define EVENTS_LENGTH 128

std::array<char, BUFFER_LENGTH> r_buffer{};
std::array<char, BUFFER_LENGTH> w_buffer{};

int main() {
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    std::cout << "listen_fd: " << listen_fd << std::endl;

    struct sockaddr_in serv_addr {};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(19999);

    if (bind(listen_fd, reinterpret_cast<struct sockaddr*>(&serv_addr), sizeof(serv_addr)) == -1) {
        return -2;
    }

    listen(listen_fd, 10);

    // epoll
    int epoll_fd = epoll_create(1);

    struct epoll_event ev {};
    std::array<epoll_event, EVENTS_LENGTH> epoll_events{};

    ev.events = EPOLLIN;
    ev.data.fd = listen_fd;

    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_fd, &ev);
    std::cout << "epoll_fd: " << epoll_fd << std::endl;

    while (true) {
        int n_ready = epoll_wait(epoll_fd, epoll_events.data(), EVENTS_LENGTH, 1000);
        std::cout << "----- n_ready: " << n_ready << std::endl;
        for (int i = 0; i < n_ready; ++i) {
            int client_fd = epoll_events[i].data.fd;

            if (listen_fd == client_fd) {  // accept
                struct sockaddr_in client {};
                socklen_t len = sizeof(client);
                int conn_fd = accept(listen_fd, reinterpret_cast<struct sockaddr*>(&client), &len);
                std::cout << "accept\t conn_fd: " << conn_fd << std::endl;

                ev.events = EPOLLIN;
                ev.data.fd = conn_fd;
                epoll_ctl(epoll_fd, EPOLL_CTL_ADD, conn_fd, &ev);
            } else if (epoll_events[i].events & EPOLLIN) {
                int n = recv(client_fd, r_buffer.data(), BUFFER_LENGTH, 0);
                if (n > 0) {
                    r_buffer[n] = '\0';
                    std::cout << "recv: " << r_buffer.data() << std::endl;

                    std::memcpy(w_buffer.data(), r_buffer.data(), BUFFER_LENGTH);

                    ev.events = EPOLLOUT;
                    ev.data.fd = client_fd;
                    epoll_ctl(epoll_fd, EPOLL_CTL_MOD, client_fd, &ev);
                }
            } else if (epoll_events[i].events & EPOLLOUT) {
                int sent = send(client_fd, w_buffer.data(), BUFFER_LENGTH, 0);
                std::cout << "send: " << sent << std::endl;

                ev.events = EPOLLIN;
                ev.data.fd = client_fd;
                epoll_ctl(epoll_fd, EPOLL_CTL_MOD, client_fd, &ev);
            }
        }
    }

    return 0;
}