#include <iostream>
#include <memory>

class A : public std::enable_shared_from_this<A>
{
public:
    std::shared_ptr<A> GetSelf() {
        return shared_from_this();
    }
    ~A() {
        std::cout << "Deconstruction A" << std::endl;
    }
};

int main() 
{
    std::shared_ptr<A> sp1(new A);
    std::shared_ptr<A> sp2 = sp1->GetSelf(); // ok

    std::cout << "sp1.use_count() = " << sp1.use_count()<< std::endl;
    std::cout << "sp2.use_count() = " << sp2.use_count()<< std::endl;
    std::cout << "leave {}" << std::endl;

    return 0;
}
