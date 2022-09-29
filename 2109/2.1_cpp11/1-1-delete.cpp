#include <iostream>
#include <memory>

void DeleteIntPtr(int* p) 
{
    std::cout << "call DeleteIntPtr" << std::endl;
    delete p;
}

int main() 
{
    std::shared_ptr<int> p(new int(1), DeleteIntPtr);
    std::shared_ptr<int> p2(new int(1), [](int* p) {
            std::cout << "call lambda delete p" << std::endl;
            delete p;
        }
    );
    std::shared_ptr<int> p3(new int[10], [](int* p) {
            std::cout << "call lambda delete p" << std::endl;
            delete [] p; // 数组删除
        }
    );

    return 0;
}
