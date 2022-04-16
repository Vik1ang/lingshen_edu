#include <iostream>
#include <memory>

class A 
{
public:
    std::shared_ptr<A> GetSelf() {
        return std::shared_ptr<A>(this); // 不要这么操作
    }
    ~A() {
        std::cout << "Deconstruction A" << std::endl;
    }
};

int main() 
{
    std::shared_ptr<A> sp1(new A);
    std::shared_ptr<A> sp2 = sp1->GetSelf();

    return 0;
}
