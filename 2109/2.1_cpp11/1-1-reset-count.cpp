#include <iostream>
#include <memory>

int main() {
    auto sp1 = std::make_shared<int>(100); // 优先使用make_shared来构造智能指针
    // 相当于
    std::shared_ptr<int> sp2(new int(100));

    // std::shared_ptr<int> p = new int(1); // 不能将一个原始指针直接赋值给一个智能指针
    
    std::shared_ptr<int> p1;
    p1.reset(new int(1)); // 分配资源
    std::shared_ptr<int> p2 = p1;

    // 引用计数器此时应该是2
    std::cout << "p2.use_count() = " << p2.use_count() << std::endl;
    p1.reset(); // 释放资源, p1变成空
    std::cout << "p1.reset()\n";
    // 引用计数器此时应该是1
    std::cout << "p2.use_count() = " << p2.use_count() << std::endl;

    if (!p1) {
        std::cout << "p1 is empty\n";
    }
    if (!p2) {
        std::cout << "p2 is empty\n";
    }

    p2.reset();
    std::cout << "p2.reset()\n";
    std::cout << "p2.use_count() = " << p2.use_count() << std::endl;

    if (!p2) {
        std::cout << "p2 is empty" << std::endl;
    }
    
    return EXIT_SUCCESS;
    
}
