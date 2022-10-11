#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define BUFFER_LENGTH 128
#define EVENTS_LENGTH 128

// thread --> fd
void *routine(void *arg) {
    int clientfd = *(int *)arg;

    while (1) {
        unsigned char buffer[BUFFER_LENGTH] = {0};
        int ret = recv(clientfd, buffer, BUFFER_LENGTH, 0);
        if (ret == 0) {
            close(clientfd);
            break;
        }
        printf("buffer : %s, ret: %d\n", buffer, ret);

        ret = send(clientfd, buffer, ret, 0);  //
    }
}

// socket -->
// bash --> execve("./server", "");
//
// 0, 1, 2
// stdin, stdout, stderr
int main() {
    // block
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);  //
    if (listenfd == -1)
        return -1;
    // listenfd
    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(19999);

    if (-1 == bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr))) {
        return -2;
    }

#if 0  // nonblock
	int flag = fcntl(listenfd, F_GETFL, 0);
	flag |= O_NONBLOCK;
	fcntl(listenfd, F_SETFL, flag);
#endif

    listen(listenfd, 10);

#if 0
	// int
	struct sockaddr_in client;
	socklen_t len = sizeof(client);
	int clientfd = accept(listenfd, (struct sockaddr*)&client, &len);

	unsigned char buffer[BUFFER_LENGTH] = {0};
	int ret = recv(clientfd, buffer, BUFFER_LENGTH, 0);
	if (ret == 0) {
		close(clientfd);

	}
	printf("buffer : %s, ret: %d\n", buffer, ret);

	ret = send(clientfd, buffer, ret, 0); //

	//printf("sendbuffer : %d\n", ret);
#elif 0

    while (1) {
        struct sockaddr_in client;
        socklen_t len = sizeof(client);
        int clientfd = accept(listenfd, (struct sockaddr *)&client, &len);

        pthread_t threadid;
        pthread_create(&threadid, NULL, routine, &clientfd);

        // fork();
    }

#elif 0

    fd_set rfds, wfds, rset, wset;

    FD_ZERO(&rfds);
    FD_SET(listenfd, &rfds);

    FD_ZERO(&wfds);

    int maxfd = listenfd;

    unsigned char buffer[BUFFER_LENGTH] = {0};  // 0
    int ret = 0;

    // int fd,
    while (1) {
        rset = rfds;
        wset = wfds;

        int nready = select(maxfd + 1, &rset, &wset, NULL, NULL);
        if (FD_ISSET(listenfd, &rset)) {
            printf("listenfd --> \n");

            struct sockaddr_in client;
            socklen_t len = sizeof(client);
            int clientfd = accept(listenfd, (struct sockaddr *)&client, &len);

            FD_SET(clientfd, &rfds);

            if (clientfd > maxfd)
                maxfd = clientfd;
        }

        int i = 0;
        for (i = listenfd + 1; i <= maxfd; i++) {
            if (FD_ISSET(i, &rset)) {  //

                ret = recv(i, buffer, BUFFER_LENGTH, 0);
                if (ret == 0) {
                    close(i);
                    FD_CLR(i, &rfds);

                } else if (ret > 0) {
                    printf("buffer : %s, ret: %d\n", buffer, ret);
                    FD_SET(i, &wfds);
                }

            } else if (FD_ISSET(i, &wset)) {
                ret = send(i, buffer, ret, 0);  //

                FD_CLR(i, &wfds);  //
                FD_SET(i, &rfds);
            }
        }

        //
    }

#else
    // fd -> epoll
    int epfd = epoll_create(1);

    struct epoll_event ev, events[EVENTS_LENGTH];
    ev.events = EPOLLIN;
    ev.data.fd = listenfd;

    epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &ev);

    printf("fd : %d\n", epfd);

    while (1) {
        // -1 有事件才返回 0 立即返回 1000: 1s返回
        int nready = epoll_wait(epfd, events, EVENTS_LENGTH, 1000);
        printf("-------%d\n", nready);
        int i = 0;
        for (i = 0; i < nready; i++) {
            int clientfd = events[i].data.fd;

            if (listenfd == clientfd) {  // accept
                struct sockaddr_in client;
                socklen_t len = sizeof(client);
                int connfd = accept(listenfd, (struct sockaddr *)&client, &len);
                if (connfd == -1) {
                    break;
                }
                printf("accept: %d\n", connfd);
                //                ev.events = EPOLLIN | EPOLLET;
                ev.events = EPOLLIN;
                ev.data.fd = connfd;
                epoll_ctl(epfd, EPOLL_CTL_ADD, connfd, &ev);
            } else if (events[i].events & EPOLLIN) {  // clientfd
                char buffer[BUFFER_LENGTH] = {0};
                int n = recv(clientfd, buffer, BUFFER_LENGTH, 0);
                if (n > 0) {
                    buffer[n] = '\0';

                    printf("recv: %s, n: %d\n", buffer, n);
                    /*int j = 0;
                    for (j = 0; j < BUFFER_LENGTH; ++j) {
                        buffer[j] = 'a' + (j % 26);
                    }*/
                    int sent = send(clientfd, buffer, n, 0);
                    printf("sent: %d\n", sent);
                }
            } else if (events[i].events & EPOLLOUT) {
            }
        }
    }

#endif
}
