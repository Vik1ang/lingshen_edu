class Singleton {
public:
    ~Singleton(){}
    static Singleton& GetInstance() {
        static Singleton instance;
        return instance;
    }
private:
    Singleton(){}
    Singleton(const Singleton&) {}
    Singleton& operator=(const Singleton&) {}
};
