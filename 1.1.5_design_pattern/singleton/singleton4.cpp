#include <mutex>
#include <atomic>

class Singleton {
public:
    static Singleton* GetInstance() {
        Singleton* tmp = _instance.load(std::memory_order_relaxed);
        std::atomic_thread_fence(std::memory_order_acquire); // 获取内存屏障 // 会让new按顺序操作, 不会让new重排
        if (tmp == nullptr) {
            std::lock_guard<std::mutex> lock(_mutex);
            tmp = _instance.load(std::memory_order_relaxed);
            if (tmp == nullptr) {
                tmp = new Singleton;
                std::atomic_thread_fence(std::memory_order_release); // 释放内存屏障
                _instance.store(tmp, std::memory_order_relaxed);
                std::atexit(Destructor);
            }
        }
        return tmp;
    }

private:
    static void Destructor() {
        Singleton* tmp = _instance.load(std::memory_order_relaxed);
        if (nullptr != tmp) {
            delete tmp;
        }
    }
    Singleton(){}
    Singleton(const Singleton&) {};
    Singleton& operator=(const Singleton&) {}
    static std::atomic<Singleton*> _instance;
    static std::mutex _mutex;
};

std::atomic<Singleton*> Singleton::_instance;
std::mutex Singleton::_mutex;

// g++ Singleton.cpp -o Singleton -std=c++11
