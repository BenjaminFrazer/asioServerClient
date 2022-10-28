#ifndef MESSAGE_H_
#define MESSAGE_H_
#include <asio.hpp>
#include <asio/error_code.hpp>
#include <asio/local/stream_protocol.hpp>
#include <iostream>
#include <memory>
#include "tsDeque.h"
#include "connection.h"
/// forward declaration
template <typename T>
class Connection;

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
            memcpy(&out, &msg.body[msg.body.size()-dataSize], dataSize);
            msg.body.resize(msg.body.size()-dataSize);
            return msg;
        }
};

template <typename T>
struct OwnedMessage {
        std::shared_ptr<Connection<T>> owner=nullptr; /// pointer to owning connection object
        Message<T> msg;
};

#endif // MESSAGE_H_
