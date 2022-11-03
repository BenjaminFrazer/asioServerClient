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
    uint32_t length=0;
};

template <typename T>
class Message{
    public:
        Message<T>(T type){
            header.type = type;
        };
        Message<T>(){
        };
        Header<T> header;
        std::vector<uint8_t> body;
        template<typename TypeName>
        friend Message<T>& operator<<(Message<T>& msg, TypeName &in){
            static_assert(std::is_pod<TypeName>::value,"Cannot take complex data types");
            int dataSize = sizeof(TypeName);
            int oldSize = msg.body.size();
            msg.body.resize(dataSize+oldSize);
            memcpy(&msg.body[oldSize], &in, dataSize);
            msg.header.length = msg.body.size();
            return msg;
        }
        template<typename TypeName>
        friend Message<T>& operator>>(Message<T>& msg, TypeName &out){
            static_assert(std::is_pod<TypeName>::value,"Cannot take complex data types");
            size_t dataSize = sizeof(TypeName);
            if (dataSize <= msg.body.size()){
            memcpy(&out, &msg.body[msg.body.size()-dataSize], dataSize);
            msg.body.resize(msg.body.size()-dataSize);
            msg.header.length = msg.body.size();
            }
            else{
                std::cout << "Message body smaller than type requested!\n";
            }
            return msg;

        }
};

template <typename T>
struct OwnedMessage {
        std::shared_ptr<Connection<T>> owner=nullptr; /// pointer to owning connection object
        Message<T> msg;
};

#endif // MESSAGE_H_
