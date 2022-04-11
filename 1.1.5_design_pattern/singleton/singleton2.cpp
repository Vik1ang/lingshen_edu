#include <stdlib.h>

class Singleton {
public:
    static Singleton* GetInstance() {
        if (_instance == nullptr) {
            _instance = new Singleton();
            atexit(Destructor);
        }
        return _instance;
    }
    ~Singleton() {}

private:
    static void Destructor() {
        if (nullptr != _instance) { // 静态成员变量, 是所有类对象共享的
            delete _instance;
            _instance = nullptr;
        }
    }
    Singleton();
    Singleton(const Singleton& cpy);
    Singleton& operator=(const Singleton&) {}
    static Singleton* _instance;
};

int main() {
    Singleton* Singleton::_instance = nullptr; // 静态成员需要初始化
}
