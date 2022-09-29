template<typename T>
class Singleton {
public:
    static T& GetInstance() {
        static T instance; // 这里要初始化DesignPattern, 需要调用DesignPattern构造函数, 同时会调用父类的构造函数
    }
protected:
    virtual ~Singleton() {}
    Singleton() {} // protected修饰构造函数, 才能让别人继承
    Singleton(const Singleton&) {}
    Singleton& operator=(const Singleton&) {}
};

class DesignPattern : public Singleton<DesignPattern> {
    friend class Singleton<DesignPattern>; // friend能让Singleton<T>访问到DesignPattern构造函数
public:
    ~DesignPattern() {}
private:
    DesignPattern(const DesignPattern&) {}
    DesignPattern& operator=(const DesignPattern&) {}
};
