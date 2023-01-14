#include <array>
#include <sys/socket.h>
#include <iostream>
#include <netinet/in.h>
#include <cstring>
#include <arpa/inet.h>

constexpr std::string_view DNS_SEVER = "114.114.114.114";

#define DNS_HOST            0x01
#define DNS_CNAME            0x05

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

const std::array domains = {
//	"www.ntytcp.com",
//	"bojing.wang",
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

int dns_create_header(struct dns_header *header) {
    if (header == nullptr) {
        return -1;
    }
    memset(header, 0, sizeof(struct dns_header));

    srandom(time(nullptr));

    header->id = random();
    header->flags |= htons(0x0100); // 转换为网络字节序
    header->question_count = htons(1); // 每次查询一个域名

    return 0;
}

int dns_create_question(struct dns_question *question, const char *host) {
    if (question == nullptr) {
        return -1;
    }
    memset(question, 0, sizeof(struct dns_question));

    question->q_name = (char *) malloc(strlen(host) + 2);
    if (question->q_name == nullptr) {
        return -2;
    }
    question->length = strlen(host) + 2;
    question->q_type = htons(1); // 查询类型, (1代表 由域名获得ip地址)
    question->q_class = htons(1); // 通常为1, 表示Internet数据

    const char delim[2] = ".";
    char *hostname_dup = strdup(host); // 这个函数造出来的东西必须free
    char *token = strtok(hostname_dup, delim);

    char *qname_p = question->q_name;

    while (token != nullptr) {

        size_t len = strlen(token); // 第一个循环token为www, 长度为3
        *qname_p = len; // 把长度放上去
        qname_p++;

        strncpy(qname_p, token, len + 1); // 复制www, 长度+1是为了把'\0'也复制进来, 如果这里不复制只需要最后加上'\0'
        qname_p += len;

        token = strtok(nullptr, delim); // 继续获取, 因为依赖上一次的结果所以线程不安全
    }

    free(hostname_dup);

    return 0;

}

int dns_build_request(struct dns_header *header, struct dns_question *question, char *request) {
    int header_size = sizeof(struct dns_header);
    int question_size = question->length + sizeof(question->q_type) + sizeof(question->q_class);
    int length = header_size + question_size;

    int offset = 0;

    memcpy(request + offset, header, sizeof(struct dns_header)); // 把header放进request
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

void dns_client_commit(const char *domain) {

    int socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_fd < 0) {
        perror("create socket failed\n");
        exit(-1);
    }
    std::cout << "url: " << domain << std::endl;

    struct sockaddr_in dest{};
    bzero(&dest, sizeof(dest));
    dest.sin_family = AF_INET;
    dest.sin_port = htons(53);
    dest.sin_addr.s_addr = inet_addr("114.114.114.114");

    int ret = connect(socket_fd, (struct sockaddr *) &dest, sizeof(dest));
    std::cout << "connect: " << ret << std::endl;

    struct dns_header header = {0};
    dns_create_header(&header);

    struct dns_question question = {0};
    dns_create_question(&question, domain);

    char request[1024] = {};
    int req_len = dns_build_request(&header, &question, request);
    size_t s_len = sendto(socket_fd, request, req_len, 0, (struct sockaddr *) &dest,
                          sizeof(struct sockaddr));

    char buffer[1024] = {0};
    struct sockaddr_in addr{};
    size_t addr_len = sizeof(struct sockaddr_in);

    size_t n = recvfrom(socket_fd, buffer, sizeof(buffer), 0, (struct sockaddr *) &addr, (socklen_t *) &addr_len);
    std::cout << "recvfrom n: " << n << std::endl;

    struct dns_item *domains = nullptr;
    dns_parse_response(buffer, &domains);
}

int main() {
    int count = domains.size();

    for (int i = 0; i < count; ++i) {
        dns_client_commit(domains[i]);
    }

    getchar();
}


