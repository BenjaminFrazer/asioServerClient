#include <cstdint>
#include <iostream>
#include "net_common.h"
#define DOMAIN "/tmp/asio_test.sock"

using namespace std;
int main(int argc, char *argv[]) {
    /// test message class
    // Message<int> message;
    // int a = 1;
    // int b = 2;
    // struct C {
    //     float z;
    //     float x;
    //     float y;
    // };

    // C c{3.14, 99.2,44.3};
    // int a_out;
    // int b_out;
    // C c_out;
    // message << c << b << a;
    // message >> a_out >> b_out >> c_out;
    // std::cout << "a out: " << a_out << "\n";
    // std::cout << "b out: " << b_out << "\n";
    // std::cout << "c.z out: " << c_out.z << "\n";
    // return 0;
    enum class msgs {
    testMsg
    };

    Message<msgs> msg(msgs::testMsg);

    /// test the client class
    unlink(DOMAIN);
    ClientIf<msgs> client;
    ServerIf<msgs> server(DOMAIN);
    server.start();
    client.connect(DOMAIN);
    client.send(msg);
    client.disconnect();
    server.stop();
    return 0;
}
