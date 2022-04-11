#include <iostream>

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
    }    

    cout << "main finish" << endl;
    return 0;
}
