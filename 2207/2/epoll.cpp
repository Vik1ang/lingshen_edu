#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>

#include <iostream>

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

    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = listen_fd;

    epo

    return 0;
}