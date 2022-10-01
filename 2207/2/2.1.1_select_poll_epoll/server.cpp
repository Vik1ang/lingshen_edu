#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <array>
#include <iostream>
#include <thread>
#include <vector>

enum { BUFFER_LENGTH = 128 };

class ThreadPool {
    std::vector<std::thread> m_pool;

 public:
    void push_back(std::thread thr) {
        m_pool.push_back(std::move(thr));
    }

    ~ThreadPool() {                     // main 函数退出后会自动调用
        for(auto &t: m_pool) t.join();  // 等待线程池全部执行完毕
    }
};

ThreadPool t_pool;

/*void* routine(void *arg){
    int client_fd = *static_cast<int*>(arg);

    while (1) {
        std::array<unsigned char, BUFFER_LENGTH> buffer = {0};
        size_t ret = recv(client_fd, buffer.data(), BUFFER_LENGTH, 0);
        std::cout << "buffer : " << buffer.data() << ", ret : " << ret
                  << std::endl;
        // send 返回大于0 不等于发送成功, 所以可能会丢失消息
        ret = send(client_fd, buffer.data(), ret, 0);
        //        std::cout << "ret : " << ret << std::endl;
    }
}*/

void my_routine(int &client_fd) {
    while (true) {
        std::array<unsigned char, BUFFER_LENGTH> buffer = {0};
        size_t ret = recv(client_fd, buffer.data(), BUFFER_LENGTH, 0);
        if (ret == 0) {
            close(client_fd);
            break;
        }
        std::cout << "buffer : " << buffer.data() << ", ret : " << ret
                  << std::endl;

        // send 返回大于0 不等于发送成功, 所以可能会丢失消息
        send(client_fd, buffer.data(), ret, 0);
        //        std::cout << "ret : " << ret << std::endl;
    }
}

int main() {
    // 默认是阻塞的
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd == -1) {
        return -1;
    }
    struct sockaddr_in serv_addr {};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr =
        htonl(INADDR_ANY);  // 其实对于0.0.0.0转不转没有所谓
    serv_addr.sin_port = htons(19999);  // 端口随便填

    if (bind(listen_fd, reinterpret_cast<struct sockaddr *>(&serv_addr),
             sizeof(serv_addr)) == -1) {
        return -2;
    }

#if 0
    int flag = fcntl(listen_fd, F_GETFL, 0);
    flag |= O_NONBLOCK;
    fcntl(listen_fd, F_SETFL, flag);
#endif

    listen(listen_fd, 10);

#if 0
    //
    struct sockaddr_in client {};
    socklen_t len = sizeof(client);
    int client_fd =
        accept4(listen_fd, reinterpret_cast<struct sockaddr*>(&client), &len,
                SOCK_CLOEXEC);
    while (1) {
        std::array<unsigned char, BUFFER_LENGTH> buffer = {0};
        size_t ret = recv(client_fd, buffer.data(), BUFFER_LENGTH, 0);
        std::cout << "buffer : " << buffer.data() << ", ret : " << ret
                  << std::endl;

        // send 返回大于0 不等于发送成功, 所以可能会丢失消息
        ret = send(client_fd, buffer.data(), ret, 0);
        //        std::cout << "ret : " << ret << std::endl;
    }
    return 0;
#else
    while (1) {
        struct sockaddr_in client {};
        socklen_t len = sizeof(client);
        int client_fd = accept(
            listen_fd, reinterpret_cast<struct sockaddr *>(&client), &len);
        std::thread t([&] { my_routine(client_fd);});
        t_pool.push_back(std::move(t));
    }
#endif
}