
// listenfd, clientfd
struct socket_item {  // conn_item
    int fd;           // clientfd

    char *rbuffer;
    int rlength;

    char *wbuffer;
    int wlength;

    int event;

    void (*recv_cb)(int fd, char *buffer, int length);
    void (*send_cb)(int fd, char *buffer, int length);
    void (*accept_cb)(int fd, char *buffer, int length);
};

struct reactor {
    int epfd;
};