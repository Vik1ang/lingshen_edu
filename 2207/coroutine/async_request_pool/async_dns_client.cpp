#include <array>
#include <sys/epoll.h>
#include <unistd.h>
#include <netinet/in.h>
#include <cstring>
#include <iostream>
#include <arpa/inet.h>
#include <fcntl.h>

constexpr int ASYNC_EVENT_LENGTH = 1024;
constexpr std::string_view DNS_SERVER = "114.114.114.114";
constexpr int DNS_HOST = 0x01;
constexpr int DNS_CNAME = 0x05;

struct dns_header {
    unsigned short id; // 会话标识
    unsigned short flags; // 标志
    unsigned short question_count; // 问题数
    unsigned short answer_count; // 回答 资源记录数
    unsigned short auth_count; // 授权 资源记录数
    unsigned short addition_count; // 附加 资源记录数
};

struct dns_question {
    int length;
    unsigned short q_type;
    unsigned short q_class;
    char *q_name; // 域名
};

struct dns_item {
    char *domain;
    char *ip;
};

struct ep_arg {
    int socket_fd;
};

const std::array DOMAINS = {
        "www.ntytcp.com",
        "bojing.wang",
        "www.baidu.com",
        "tieba.baidu.com",
        "news.baidu.com",
        "zhidao.baidu.com",
        "music.baidu.com",
        "image.baidu.com",
        "v.baidu.com",
        "map.baidu.com",
        "baijiahao.baidu.com",
        "xueshu.baidu.com",
        "cloud.baidu.com",
        "www.163.com",
        "open.163.com",
        "auto.163.com",
        "gov.163.com",
        "money.163.com",
        "sports.163.com",
        "tech.163.com",
        "edu.163.com",
        "www.taobao.com",
        "q.taobao.com",
        "sf.taobao.com",
        "yun.taobao.com",
        "baoxian.taobao.com",
        "www.tmall.com",
        "suning.tmall.com",
        "www.tencent.com",
        "www.qq.com",
        "www.aliyun.com",
        "www.ctrip.com",
        "hotels.ctrip.com",
        "hotels.ctrip.com",
        "vacations.ctrip.com",
        "flights.ctrip.com",
        "trains.ctrip.com",
        "bus.ctrip.com",
        "car.ctrip.com",
        "piao.ctrip.com",
        "tuan.ctrip.com",
        "you.ctrip.com",
        "g.ctrip.com",
        "lipin.ctrip.com",
        "ct.ctrip.com"
};

struct async_context {
    int ep_fd;
    pthread_t t_id;
};

int dns_create_header(struct dns_header *header) {

    if (header == NULL) return -1;
    memset(header, 0, sizeof(struct dns_header));

    srandom(time(NULL));

    header->id = random();
    header->flags |= htons(0x0100);
    header->question_count = htons(1);

    return 0;
}

int dns_create_question(struct dns_question *question, const char *hostname) {

    if (question == NULL) return -1;
    memset(question, 0, sizeof(struct dns_question));

    question->q_name = (char *) malloc(strlen(hostname) + 2);
    if (question->q_name == NULL) return -2;

    question->length = strlen(hostname) + 2;

    question->q_type = htons(1);
    question->q_class = htons(1);

    const char delim[2] = ".";

    char *hostname_dup = strdup(hostname);
    char *token = strtok(hostname_dup, delim);

    char *qname_p = question->q_name;

    while (token != NULL) {

        size_t len = strlen(token);

        *qname_p = len;
        qname_p++;

        strncpy(qname_p, token, len + 1);
        qname_p += len;

        token = strtok(NULL, delim);
    }

    free(hostname_dup);

    return 0;

}

int dns_build_request(struct dns_header *header, struct dns_question *question, char *request) {

    int header_s = sizeof(struct dns_header);
    int question_s = question->length + sizeof(question->q_type) + sizeof(question->q_class);

    int length = question_s + header_s;

    int offset = 0;
    memcpy(request + offset, header, sizeof(struct dns_header));
    offset += sizeof(struct dns_header);

    memcpy(request + offset, question->q_name, question->length);
    offset += question->length;

    memcpy(request + offset, &question->q_type, sizeof(question->q_type));
    offset += sizeof(question->q_type);

    memcpy(request + offset, &question->q_class, sizeof(question->q_class));

    return length;

}

static int is_pointer(int in) {
    return ((in & 0xC0) == 0xC0);
}

void dns_parse_name(unsigned char *chunk, unsigned char *ptr, char *out, int *len) {
    int flag = 0, n = 0, alen = 0;
    char *pos = out + (*len);

    while (true) {

        flag = (int) ptr[0];
        if (flag == 0) break;

        if (is_pointer(flag)) {

            n = (int) ptr[1];
            ptr = chunk + n;
            dns_parse_name(chunk, ptr, out, len);
            break;

        } else {

            ptr++;
            memcpy(pos, ptr, flag);
            pos += flag;
            ptr += flag;

            *len += flag;
            if ((int) ptr[0] != 0) {
                memcpy(pos, ".", 1);
                pos += 1;
                (*len) += 1;
            }
        }

    }
}

int dns_parse_response(char *buffer, struct dns_item **domains) {
    int i = 0;
    unsigned char *ptr = reinterpret_cast<unsigned char *>(buffer);

    ptr += 4;
    int querys = ntohs(*(unsigned short *) ptr);

    ptr += 2;
    int answers = ntohs(*(unsigned short *) ptr);

    ptr += 6;
    for (i = 0; i < querys; i++) {
        while (1) {
            int flag = (int) ptr[0];
            ptr += (flag + 1);

            if (flag == 0) break;
        }
        ptr += 4;
    }

    char cname[128], aname[128], ip[20], netip[4];
    int len, type, ttl, datalen;

    int cnt = 0;
    struct dns_item *list = (struct dns_item *) calloc(answers, sizeof(struct dns_item));
    if (list == NULL) {
        return -1;
    }

    for (i = 0; i < answers; i++) {

        bzero(aname, sizeof(aname));
        len = 0;

        dns_parse_name(reinterpret_cast<unsigned char *>(buffer), ptr, aname, &len);
        ptr += 2;

        type = htons(*(unsigned short *) ptr);
        ptr += 4;

        ttl = htons(*(unsigned short *) ptr);
        ptr += 4;

        datalen = ntohs(*(unsigned short *) ptr);
        ptr += 2;

        if (type == DNS_CNAME) {

            bzero(cname, sizeof(cname));
            len = 0;
            dns_parse_name(reinterpret_cast<unsigned char *>(buffer), ptr, cname, &len);
            ptr += datalen;

        } else if (type == DNS_HOST) {

            bzero(ip, sizeof(ip));

            if (datalen == 4) {
                memcpy(netip, ptr, datalen);
                inet_ntop(AF_INET, netip, ip, sizeof(struct sockaddr));

                printf("%s has address %s\n", aname, ip);
                printf("\tTime to live: %d minutes , %d seconds\n", ttl / 60, ttl % 60);

                list[cnt].domain = (char *) calloc(strlen(aname) + 1, 1);
                memcpy(list[cnt].domain, aname, strlen(aname));

                list[cnt].ip = (char *) calloc(strlen(ip) + 1, 1);
                memcpy(list[cnt].ip, ip, strlen(ip));

                cnt++;
            }

            ptr += datalen;
        }
    }

    *domains = list;
    ptr += 2;

    return cnt;

}

static int set_block(int fd, int block) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) return flags;

    if (block) {
        flags &= ~O_NONBLOCK;
    } else {
        flags |= O_NONBLOCK;
    }

    if (fcntl(fd, F_SETFL, flags) < 0) return -1;

    return 0;
}

int dns_async_client_commit(struct async_context *ctx, const char *domain) {
    if (ctx == nullptr) {
        return -EINVAL;
    }

    int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd < 0) {
        perror("create socket failed\n");
        exit(-1);
    }

    std::cout << "url: " << domain << std::endl;

    set_block(sock_fd, 0); //nonblock

    struct sockaddr_in dest;
    bzero(&dest, sizeof(dest));
    dest.sin_family = AF_INET;
    dest.sin_port = htons(53);
    dest.sin_addr.s_addr = inet_addr(DNS_SERVER.data());

    int ret = connect(sock_fd, (struct sockaddr *) &dest, sizeof(dest));

    struct dns_header header{};
    dns_create_header(&header);

    struct dns_question question{};
    dns_create_question(&question, domain);

    char request[1024] = {0};
    int req_len = dns_build_request(&header, &question, request);
    size_t s_len = sendto(sock_fd, request, req_len, 0, (struct sockaddr *) &dest, sizeof(struct sockaddr));

    struct ep_arg *arg = (struct ep_arg *) (calloc(1, sizeof(struct ep_arg)));
    if (arg == nullptr) {
        return -1;
    }

    struct epoll_event ev{};
    ev.data.ptr = arg;
    ev.events = EPOLLIN;

    return epoll_ctl(ctx->ep_fd, EPOLL_CTL_ADD, sock_fd, &ev);
}

void *dns_async_client_callback(void *arg) {
    struct async_context *ctx = (struct async_context *) arg;
    while (true) {
        struct epoll_event events[ASYNC_EVENT_LENGTH] = {};
        int n_ready = epoll_wait(ctx->ep_fd, events, ASYNC_EVENT_LENGTH, -1); // 检测IO事件是否可读
        for (int i = 0; i < n_ready; i++) {
            struct ep_arg *arg = (struct ep_arg *) events[i].data.ptr;
            int sock_fd = arg->socket_fd;
            char buffer[1024] = {0};
            struct sockaddr_in addr{};
            size_t addr_len = sizeof(struct sockaddr_in);

            size_t n = recvfrom(sock_fd, buffer, sizeof(buffer), 0, (struct sockaddr *) &addr, (socklen_t *) &addr_len);
            std::cout << "recvfrom n: " << n << std::endl;

            struct dns_item *domains = nullptr;
            dns_parse_response(buffer, &domains);
            epoll_ctl(ctx->ep_fd, EPOLL_CTL_DEL, sock_fd, nullptr);
            close(sock_fd);
            free(arg);
        }
    }
}

int dns_async_client_destroy(struct async_context *ctx) {
    if (ctx == nullptr) {
        return -EINVAL;
    }

    pthread_cancel(ctx->t_id);
    close(ctx->ep_fd);

    return 0;
}

int dns_async_client_init(struct async_context *ctx) {
    if (ctx == nullptr) {
        return -EINVAL;
    }

    ctx->ep_fd = epoll_create(1);

    int ret = pthread_create(&ctx->t_id, nullptr, dns_async_client_callback, ctx);
    if (ret) {
        perror("pthread_create fail");
        return -1;
    }

    return 0;
}

int main() {
    struct async_context ctx{};
    dns_async_client_init(&ctx);
    for (auto domain: DOMAINS) {
        dns_async_client_commit(&ctx, domain);
    }

    getchar();
}