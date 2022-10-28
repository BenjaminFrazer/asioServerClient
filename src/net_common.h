#ifndef NET_COMMON_H_
#define NET_COMMON_H_
#include <asio.hpp>
#include <asio/error_code.hpp>
#include <asio/local/stream_protocol.hpp>
#include <iostream>
#include <memory>
#include "tsDeque.h"

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
        std::shared_ptr<TsDeque<Message<T>>> q_send_reff=nullptr; /// refference to connection's outgoing message que
        TsDeque<OwnedMessage<T>> q_rec; /// incoming message que
        asio::io_context context; /// reference to the client's asio context
    protected:
        std::unique_ptr<Connection<T>> con_ptr=nullptr;
};

template <typename T>
class ServerIf{
    public:
        ServerIf<T>(std::string domain) : acceptor(context,asio::local::stream_protocol::endpoint(domain)){
        };
        void start(){
            // start accepting connections
            acceptor.async_accept([this](std::error_code ec,asio::local::stream_protocol::socket sock){
                if (!ec){
                    // Connection<T>(Type TYPE, asio::local::stream_protocol::socket Sock, asio::io_context& Context, TsDeque<OwnedMessage<T>>& Q_rec)
                    auto newCon = std::make_shared<Connection<T>>(Connection<T>::SERVER, std::move(sock), context, q_rec);
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

template <typename T>
class Connection : public std::enable_shared_from_this<Connection<T>> {
    public:
        enum Type{
            CLIENT,
            SERVER
        };
        Type type;
        Connection<T>(Type TYPE, std::string Domain_path, asio::io_context& Context, TsDeque<OwnedMessage<T>>& Q_rec)
            : context(Context), q_rec_reff(Q_rec), sock(Context){
            type = TYPE;
            domain_path = Domain_path;
        }
        Connection<T>(Type TYPE, asio::local::stream_protocol::socket Sock, asio::io_context& Context, TsDeque<OwnedMessage<T>>& Q_rec)
            : context(Context), q_rec_reff(Q_rec), sock(std::move(Sock)){
        }
        void connect(){
            asio::local::stream_protocol::endpoint ep(domain_path); /// create end point
            sock.async_connect(ep,[this](const asio::error_code& ec){/// async connection
                if (!ec){
                    std::cout << "Successfully connected!\n";
                    // here we want to stage some further async work
                    readHeader();
                }
                else {std::cout <<"Failed to connect!\n";
                }
            });
            std::thread([this](){
                context.run(); //set off our work
            });
        }
        asio::io_context &context; /// reference to the client's asio context
        TsDeque<OwnedMessage<T>>& q_rec_reff;
        TsDeque<Message<T>> q_out; /// outgoing message que
        void send(const Message<T>& msg){
            asio::post(context,[this,msg](){ // i guess we use post here for thread safety?
                bool currentlyNotWriting = q_out.empty();
                q_out.pushback(msg);
                // we infer that there isn't currently a write loop running if the que is empty
                if(currentlyNotWriting){ // start off a write task
                        writeNextHeader();
                }
            });
        }
    protected:
        void readHeader(){
            asio::async_read(sock,asio::buffer(&_tmpMsgIn.header,sizeof(_tmpMsgIn.header)),[this](std::error_code ec,std::size_t len){
                if(!ec){
                    if(_tmpMsgIn.header.length>0){
                        readBody(_tmpMsgIn.header.length);
                    }
                    else{
                        addToIncomingMsgQue();
                        readHeader();
                    };
                }
                else{
                    std::cout << "Failed to read header: "<< ec.message() << "\n";
                    sock.close();
                }
            });
        }
        void readBody(uint32_t size){
            _tmpMsgIn.body.resize(size);
            asio::async_read(sock,
                            asio::buffer(_tmpMsgIn.body.data(),_tmpMsgIn.body.size()),
                            [this](std::error_code ec, std::size_t size){
                                if (!ec){
                                    addToIncomingMsgQue();
                                    readHeader();
                                }
                                else{
                                    std::cout << "Failed to read message body:" << ec.message() << "\n";
                                    sock.close();
                                }
                            }
                            );
        }
        void writeNextHeader(){
            sock.write_some(&asio::buffer(q_out.front().header,sizeof(q_out.front().header.size)),[this](std::error_code ec,std::size_t length){
                if (!ec){
                    if (q_out.front().body.size()>0){ // check that the there is data in the body
                        writeNextBody();
                    }
                    else{
                        q_out.pop_front();
                        if (!q_out.empty()){
                            writeNextHeader();
                        }
                    }
                }
                else{
                    std::cout << "Message Header failed to send: "<< ec.message() << "\n";
                    sock.close();
                }
            }
            );
        };
        void writeNextBody(){
            sock.async_write_some(asio::buffer(q_out.front().body.data(),q_out.front().body.size()), [this](std::error_code ec, std::size_t len){
                if (!ec){
                    if (q_out.empty()){
                        writeNextHeader();
                    }
                }
                else{
                    std::cout << "Message Body failed to sent: " <<ec.message() << "\n";
                    sock.close();
                }
            }
            );
        };
    protected:
        void addToIncomingMsgQue(){
            // sign the
            q_rec_reff.push_back({this->shared_from_this(),_tmpMsgIn});
        };
    protected:
        asio::local::stream_protocol::socket sock;
        std::string domain_path;
        Message<T> _tmpMsgIn; /// current incoming message
};

#endif // NET_COMMON_H_
