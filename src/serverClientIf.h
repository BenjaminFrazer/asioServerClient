#ifndef SERVERCLIENTIF_H_
#define SERVERCLIENTIF_H_
#include <asio.hpp>
#include <asio/error_code.hpp>
#include <asio/local/stream_protocol.hpp>
#include <iostream>
#include <memory>
#include "tsDeque.h"
#include "connection.h"
#include "message.h"

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
        // refference to connection's outgoing message que
        std::shared_ptr<TsDeque<Message<T>>> q_send_reff=nullptr;
        TsDeque<OwnedMessage<T>> q_rec; /// incoming message que
        asio::io_context context; /// reference to the client's asio context
    protected:
        std::unique_ptr<Connection<T>> con_ptr=nullptr;
};

template <typename T>
class ServerIf{
    public:
        ServerIf<T>(std::string domain) : acceptor(context,
                                                   asio::local::stream_protocol::endpoint(domain)){
        };
        void start(){
            // start accepting connections
            acceptor.async_accept([this](std::error_code ec,
                                         asio::local::stream_protocol::socket sock){
                if (!ec){
                    auto newCon = std::make_shared<Connection<T>>(Connection<T>::SERVER,
                                                                  std::move(sock),
                                                                  context,
                                                                  q_rec);
                    con.push_back(std::move(newCon));
                }
            });
            std::thread([this](){
                context.run();
            });
        };
        void end(){
        };
        TsDeque<OwnedMessage<T>> q_rec; /// incoming message que
    protected:
        std::vector<std::shared_ptr<Connection<T>>> con; /// all currently active connecions
        asio::io_context context;
        asio::local::stream_protocol::acceptor acceptor;
};

#endif // SERVERCLIENTIF_H_
