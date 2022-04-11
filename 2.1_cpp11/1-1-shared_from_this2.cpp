#include <iostream>
#include <memory>

using namespace std;

class A 
{
public:
    ~A() {
        cout << "delete A" << endl;
    }
};

int main()
{
    {
        A *pa = new A;
        std::shared_ptr<A> pb(new A);
    }    

    cout << "main finish" << endl;
    return 0;
}
