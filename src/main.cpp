// #define DO_DEBUG true
#include <cstdint>
#include <iostream>
#include "net_common.h"
#include <unistd.h>
#define DOMAIN "/tmp/asio_test.sock"

using namespace std;
int main(int argc, char *argv[]) {

    enum class msgs {
    invalid_msg,
    testMsg
    };

    Message<msgs> msg(msgs::testMsg);
    char testCharSent[] = "hello";
    char testCharRec[] = "overw";
    msg << testCharSent;

    /// test the client class
    unlink(DOMAIN);
    ClientIf<msgs> client;
    ServerIf<msgs> server(DOMAIN);
    server.start();
    client.connect(DOMAIN);
    sleep(1);
    client.send(msg);
    bool clientStopped = client.context.stopped();
    bool serverStopped = server.context.stopped();
    auto server_Ep = server.con_ptr.front()->local_endpoint();
    auto client_Ep = client.con_ptr->local_endpoint();
    bool doCont = true;
    while (doCont){
        doCont = server.q_rec.empty();
        sleep(1);
    }
    auto rec = server.q_rec.pop_front();
    rec.msg >> testCharRec;
    std::cout << "Recieved msg: " << testCharRec << "\n";
    getchar();
    client.disconnect();
    server.stop();
    return 0;
}
