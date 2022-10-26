#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <array>
#include <cstring>
#include <iostream>
#include <string>

constexpr const std::string_view DNS_SVR = "114.114.114.114";

struct dns_header {
    uint16_t id;
    uint16_t flags;
    uint16_t qd_count;
    uint16_t an_count;
    uint16_t ns_count;
    uint16_t ar_count;
};

struct dns_question {
    int length;
    uint16_t q_class;
    uint16_t q_type;
    std::string q_name;
};

const std::array domains = {
    //	"www.ntytcp.com",
    //	"bojing.wang",
    "www.baidu.com",     "tieba.baidu.com",  "news.baidu.com",
    "zhidao.baidu.com",  "music.baidu.com",  "image.baidu.com",
    "v.baidu.com",       "map.baidu.com",    "baijiahao.baidu.com",
    "xueshu.baidu.com",  "cloud.baidu.com",  "www.163.com",
    "open.163.com",      "auto.163.com",     "gov.163.com",
    "money.163.com",     "sports.163.com",   "tech.163.com",
    "edu.163.com",       "www.taobao.com",   "q.taobao.com",
    "sf.taobao.com",     "yun.taobao.com",   "baoxian.taobao.com",
    "www.tmall.com",     "suning.tmall.com", "www.tencent.com",
    "www.qq.com",        "www.aliyun.com",   "www.ctrip.com",
    "hotels.ctrip.com",  "hotels.ctrip.com", "vacations.ctrip.com",
    "flights.ctrip.com", "trains.ctrip.com", "bus.ctrip.com",
    "car.ctrip.com",     "piao.ctrip.com",   "tuan.ctrip.com",
    "you.ctrip.com",     "g.ctrip.com",      "lipin.ctrip.com",
    "ct.ctrip.com"};

int dns_create_header(struct dns_header* header) {
    if (header == nullptr) {
        return -1;
    }
    bzero(header, sizeof(struct dns_header));

    srandom(time(nullptr));

    header->id = random();
    header->flags |= htons(0x0100);
    header->qd_count = htons(1);

    return 0;
}

int dns_create_question(struct dns_question* question,
                        const std::string_view hostname) {
    if (question == nullptr) {
        return -1;
    }
    bzero(question, sizeof(struct dns_question));

    question->length = static_cast<int>(hostname.length()) + 2;
    question->q_type = htons(1);
    question->q_class = htons(1);

    //    const char delim[2] = ".";
    const std::array<char, 2> delim{{"."}};

    char* hostname_dup = strdup(hostname.data());
    char* token = strtok(hostname_dup, delim.data());
    char* q_name_p = question->q_name.data();

    while (token != nullptr) {
        size_t len = strlen(token);

        *q_name_p = static_cast<char>(len);
        q_name_p++;

        strncpy(q_name_p, token, len + 1);
        q_name_p += len;

        token = strtok(nullptr, delim.data());
    }

    return 0;
}

int dns_client_commit(const std::string& domain) {
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        std::perror("create socket failed\n");
        exit(-1);
    }

    std::cout << "url: " << domain << std::endl;

    struct sockaddr_in dest {};
    bzero(&dest, sizeof(dest));
    dest.sin_family = AF_INET;
    dest.sin_port = htons(53);
    dest.sin_addr.s_addr = inet_addr(DNS_SVR.data());

    int ret = connect(socket_fd, reinterpret_cast<struct sockaddr*>(&dest),
                      sizeof(dest));
    std::cout << "connect: " << ret << std::endl;

    struct dns_header header {};
    dns_create_header(&header);

    struct dns_question question {};
    dns_create_question(&question, domain);

    return 0;
}

int main() {
    size_t count = domains.size();

    for (size_t i = 0; i < count; ++i) {
        dns_client_commit(domains[i]);
    }

    getchar();

    return 0;
}