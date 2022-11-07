#include <string>
#include <vector>

#define BUFFER_LENGTH 128
#define EVENTS_LENGTH 128

struct socket_item {
    int fd;

    std::string r_buffer;

    std::string w_buffer;

    int event;

    void (*recv_cb)(int fd, char* buffer, int length);
    void (*send_cb)(int fd, char* buffer, int length);
    void (*accept_cb)(int fd, char* buffer, int length);
};

struct reactor {
    int epoll_fd;
    std::vector<socket_item> items;
};

int main() {
    int listen_fd = socket()
}