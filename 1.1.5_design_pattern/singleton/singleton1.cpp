class Singleton {
public:
    static Singleton* GetInstance() {
        if (_instance == nullptr) {
            _instance = new Singleton();
        }
        return _instance;
    }

private:
    Singleton() {} // 构造 为了保证全局只有一个
    ~Singleton() {} // 避免直接被删除
    Singleton(const Singleton& clone) {} // 拷贝构造, 禁止拷贝构造
    Singleton& operator=(const Singleton&) {}
    static Singleton* _instance;
};

Singleton* Singleton::_instance = nullptr; // 静态成员需要初始化
