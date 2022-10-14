#include <iostream>
#include "net_common.h"

using namespace std;

int main(int argc, char *argv[]) {
    Message<int> message;
    int a = 1;
    int b = 2;
    struct C {
        float z;
        float x;
        float y;
    };

    C c{3.14, 99.2,44.3};
    int a_out;
    int b_out;
    C c_out;
    message << c << b << a;
    message >> a_out >> b_out >> c_out;
    std::cout << "a out: " << a_out << "\n";
    std::cout << "b out: " << b_out << "\n";
    std::cout << "c.z out: " << c_out.z << "\n";
    return 0;
}
