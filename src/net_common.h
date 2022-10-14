#ifndef NET_COMMON_H_
#define NET_COMMON_H_
#include <asio.hpp>
#include <asio/error_code.hpp>
#include <asio/local/stream_protocol.hpp>
#include <iostream>
#include <memory>
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
            memcpy(&out, &msg.body[msg.body.size()-dataSize], dataSize);
            msg.body.resize(msg.body.size()-dataSize);
            return msg;
        }
};

/// forward declaration
template <typename T>
class Connection;

template <typename T>
class ClientIf{
    public:
        ClientIf<T>(){
        }
        void connect(std::string domain){
            //construct a unique instance of the connection class and let it set up it's asio stuff
            con_ptr = std::make_unique<Connection<T>>(Connection<T>::CLIENT, domain, context,q_rec);
            con_ptr->connect();
        }
        void start(){
        }
        void end(){
        }
        std::shared_ptr<TsDeque<Message<T>>> q_send_reff=nullptr; /// refference to connection's outgoing message que
        TsDeque<Message<T>> q_rec; /// incoming message que
        asio::io_context context; /// reference to the client's asio context
    protected:
        std::unique_ptr<Connection<T>> con_ptr=nullptr;
};

template <typename T>
class Connection : std::enable_shared_from_this<Connection<T>>{
    public:
        enum Type{
            CLIENT,
            SERVER
        };
        Type type;
        Connection<T>(Type TYPE, std::string Domain_path, asio::io_context& Context, TsDeque<Message<T>>& Q_rec)
            : context(Context), q_rec_reff(Q_rec), sock(Context){
            type = TYPE;
            domain_path = Domain_path;
        }
        void connect(){
            asio::local::stream_protocol::endpoint ep(domain_path); /// create end point
            sock.async_connect(ep,[this](const asio::error_code& ec){/// async connection
                if (!ec){
                    std::cout << "Successfully connected!\n";
                    // here we want to stage some further async work
                }
                else {std::cout <<"Failed to connect!\n";}
            });
            context.run(); //set off our work
        }
        asio::io_context &context; /// reference to the client's asio context
        TsDeque<Message<T>>& q_rec_reff;
        TsDeque<Message<T>> q_out; /// outgoing message que
    protected:
        asio::local::stream_protocol::socket sock;
        std::string domain_path;
};



#endif // NET_COMMON_H_
