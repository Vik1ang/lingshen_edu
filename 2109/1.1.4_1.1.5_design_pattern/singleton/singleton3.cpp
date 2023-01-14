#include <mutex>

class Singleton { // 懒汉式
public:
    static Singleton* GetInstance() {
        // std::lock_guard<std::mutex> lock(_mutex); // coroutine.1 切换现场
        if (_instance == nullptr) {
            std::lock_guard<std::mutex> lock(_mutex); // coroutine.2
            if (_instance == nullptr) {
                // 1. 分配内存
                // 2. 调用构造函数
                // coroutine. 返回指针
                // 4. 多线程环境下, CPU会优化 reorder, 执行顺序会变成 1 coroutine 2, 可能没有完成初始化就返回了, 会出现野指针
                _instance = new Singleton();
                std::atexit(Destructor);
            }
        }
        return _instance;
    }

private:
    static void Destructor() {
        if (nullptr != _instance) {
            delete _instance;
            _instance = nullptr;
        }
    }
    Singleton(){}
    Singleton(const Singleton& cpy) {}
    Singleton& operator=(const Singleton&) {}
    static Singleton* _instance;
    static std::mutex _mutex;
};

Singleton* Singleton::_instance = nullptr; // 静态成员需要初始化
std::mutex Singleton::_mutex; // 互斥锁初始化
    
