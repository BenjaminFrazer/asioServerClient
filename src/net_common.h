#ifndef NET_COMMON_H_
#define NET_COMMON_H_
#include <asio.hpp>
#include <iostream>
#include "tsDeque.h"

template <typename T>
struct Header{
    T type;
    uint32_t length;
};

template <typename T>
class Message{
    public:
        Header<T> header;
        std::vector<uint8_t> body;
        template<typename TypeName>
        friend Message<T>& operator<<(Message<T>& msg, TypeName &in){
            static_assert(std::is_pod<TypeName>::value,"Cannot take complex data types");
            int dataSize = sizeof(TypeName);
            int oldSize = msg.body.size();
            msg.body.resize(dataSize+oldSize);
            memcpy(&msg.body[oldSize], &in, dataSize);
            return msg;
        }
        template<typename TypeName>
        friend Message<T>& operator>>(Message<T>& msg, TypeName &out){
            static_assert(std::is_pod<TypeName>::value,"Cannot take complex data types");
            int dataSize = sizeof(TypeName);
            int oldSize = msg.body.size();
            memcpy(&out, &msg.body[msg.body.size()-dataSize], dataSize);
            msg.body.resize(msg.body.size()-dataSize);
            return msg;
        }
};

template <typename T>
class ClientIf{
    public:
        TsDeque<Message<T>>& q; /// refference to connection's incoming message que
        asio::io_context context; /// reference to the client's asio context
};

template <typename T>
class Connection{
    public:
        TsDeque<Message<T>> msgQue; /// incoming message que
        asio::io_context &context; /// reference to the client's asio context

};



#endif // NET_COMMON_H_
