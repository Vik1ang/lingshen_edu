#include <iostream>
#include <memory>
#include <pthread.h>

class A;
class B;

class A 
{
public:
    std::shared_ptr<B> bptr;
    ~A() {
        std::cout << "A is deleted" << std::endl;
    }
};

class B 
{
public:
    std::shared_ptr<A> aptr;
    ~B() {
        std::cout << "B is deleted" << std::endl;
    }
};

int main() 
{
    {
        std::shared_ptr<A> ap(new A);
        std::shared_ptr<B> bp(new B);

        ap->bptr = bp;
        bp->aptr = ap;
    }

    std::cout << "main level" << std::endl; //循环引用导致ap bp退出了作用域也没有析构
    
    return 0;
}
