#include <iostream>

#if 0

class ZooShow {
public:
    void Show0() {
        cout << "show0" << endl;
    }
    void Show2() {
        cout << "show2" << endl;
    }
};

class ZooShowEx {
public:
    void Show1() {
        cout << "show1" << endl;
    }
    void Show3() {
        cout << "show3" << endl;
    }
};

#else if 2

class ZooShow {
public:
    ZooShow(int type = 1) : _type(type) {}

public:
    void Show() {
        if (Show0())
            PlayGame(); // 里氏替换
        Show1();
        Show2();
        Show3();
    }

// 接口隔离 不要让用户去选择它们不需要的接口
private:
    void PlayGame() {
        std::cout << "after Show0, then play game" << std::endl;
    }

// 
protected:
    virtual bool Show0() {
        std::cout << _type << " show0" << std::endl;
        return true;
    }

    virtual void Show1() {
        if (_type == 1) {
            std::cout << _type << " Show1" << std::endl;
        } else if (_type == 2) {
            std::cout << _type << " Show1" << std::endl;
        } else if (_type == 3) {
            
        }
    }

    virtual void Show2() {
        if (_type == 20) {
            
        }
        std::cout << "base Show2" << std::endl;
    }

    virtual void Show3() {
        if (_type == 1) {
            std::cout << _type << " Show1" << std::endl;
        } else if (_type == 2) {
            std::cout << _type << " Show1" << std::endl;
        }
    }
private:
    int _type;
};

#endif

int main() {
#if 0
    ZooShow *zs = new ZooShow;
    ZooShowEx *zs1 = new ZooShowEx;
    zs->Show0();
    zs1->Show1();
    zs->Show2();
    zs1->Show3();
#else if 2
    ZooShow *zs = new ZooShow(1);
    zs->Show();
#endif
    return 0;
}
