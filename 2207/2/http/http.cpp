#include <fcntl.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

#include <array>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

namespace http {

constexpr int32_t BUFFER_LENGTH = 1024;
constexpr int32_t MAX_EPOLL_EVENTS = 1024;
constexpr int32_t RESOURCE_LENGTH = 1024;
constexpr int16_t SERVER_PORT = 19996;
constexpr int32_t PORT_COUNT = 1;
constexpr std::string_view HTTP_WEB_ROOT = "/home/vik1ang/Workspace/lingshen/2207/2";

int recv_cb(int fd, int events, void* arg);
int send_cb(int fd, int events, void* arg);

using N_CALLBACK = int(int, int, void*);

struct nty_event {
    int fd;
    int events;
    void* arg;
    int (*callback)(int fd, int events, void* arg);

    int status;

    std::array<char, BUFFER_LENGTH> buffer;
    int length;

    std::array<char, BUFFER_LENGTH> w_buffer;
    int w_length;

    int method;
    std::array<char, RESOURCE_LENGTH> resource;
};

struct event_block {
    std::vector<nty_event> events;
};

struct nty_reactor {
    int ep_fd;
    //    int blk_cnt;

    std::vector<event_block> ev_blk;
    //    event_block* ev_blk;
};

void nty_event_set(nty_event* ev, int fd, N_CALLBACK callback, void* arg) {
    ev->fd = fd;
    ev->callback = callback;
    ev->events = 0;
    ev->arg = arg;
}

int nty_event_add(nty_event* ev, int ep_fd, int events) {
    epoll_event ep_ev = {0, {nullptr}};
    ep_ev.data.ptr = ev;
    ev->events = events;
    ep_ev.events = ev->events;

    int op = 0;
    if (ev->status == 1) {
        op = EPOLL_CTL_MOD;
    } else {
        op = EPOLL_CTL_ADD;
        ev->status = 1;
    }

    if (epoll_ctl(ep_fd, op, ev->fd, &ep_ev) < 0) {
        std::ostringstream ss;
        ss << "event add failed [fd=" << ev->fd << "]";
        ss << ", events[" << events << "]";
        std::cout << ss.str() << std::endl;
        return -1;
    }
    return 0;
}

int nty_event_del(nty_event* ev, int ep_fd) {
    if (ev->status != 1) {
        return -1;
    }
    epoll_event ep_ev = {0, {nullptr}};

    ep_ev.data.ptr = ev;
    ev->status = 0;
    epoll_ctl(ep_fd, EPOLL_CTL_DEL, ev->fd, &ep_ev);

    return 0;
}

int nty_reactor_alloc(nty_reactor* reactor) {
    if (reactor == nullptr) {
        return -1;
    }

    event_block block{};

    std::vector<nty_event> events;
    events.reserve(MAX_EPOLL_EVENTS);

    block.events = events;
    reactor->ev_blk.push_back(block);

    return 0;
}

nty_event* nty_reactor_idx(nty_reactor* reactor, int sock_fd) {
    if (reactor == nullptr) {
        return nullptr;
    }

    size_t blk_idx = sock_fd / MAX_EPOLL_EVENTS;
    while (blk_idx >= reactor->ev_blk.size()) {
        nty_reactor_alloc(reactor);
    }

    auto blk = reactor->ev_blk.back();

    //    return &blk.events.at(sock_fd % MAX_EPOLL_EVENTS);

    return &blk.events[sock_fd % MAX_EPOLL_EVENTS];
}

int recv_cb(int fd, int events, void* arg) {
    auto* reactor = static_cast<nty_reactor*>(arg);
    nty_event* ev = nty_reactor_idx(reactor, fd);

    if (ev == nullptr) {
        return -1;
    }

    int len = recv(fd, ev->buffer.data(), BUFFER_LENGTH, 0);
    nty_event_del(ev, reactor->ep_fd);

    if (len > 0) {
        ev->length = len;
        ev->buffer[len] = '\0';

        std::ostringstream ss;
        ss << "recv [" << fd << "]: " << ev->buffer.data();
        std::cout << ss.str() << std::endl;

        nty_event_set(ev, fd, send_cb, reactor);
        nty_event_add(ev, reactor->ep_fd, ev->events);
    } else if (len == 0) {
        nty_event_del(ev, reactor->ep_fd);
        close(ev->fd);
    } else {
        if (errno == EAGAIN && errno == EWOULDBLOCK) {
        } else if (errno == ECONNRESET) {
            nty_event_del(ev, reactor->ep_fd);
            close(ev->fd);
        }
    }

    return len;
}

int send_cb(int fd, int events, void* arg) {
    auto* reactor = static_cast<nty_reactor*>(arg);
    nty_event* ev = nty_reactor_idx(reactor, fd);
    if (ev == nullptr) {
        return -1;
    }

    int len = send(fd, ev->w_buffer.data(), ev->w_buffer.size(), 0);
    if (len > 0) {
        nty_event_del(ev, reactor->ep_fd);
        nty_event_set(ev, fd, recv_cb, reactor);
        nty_event_add(ev, reactor->ep_fd, EPOLLIN);
    } else {
        nty_event_del(ev, reactor->ep_fd);
        close(ev->fd);
    }

    return len;
}

}  // namespace http

int main() {}