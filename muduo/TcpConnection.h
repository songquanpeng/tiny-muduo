//
// Created by song on 7/3/2022.
//

#ifndef TINY_MUDUO_TCPCONNECTION_H
#define TINY_MUDUO_TCPCONNECTION_H


#include "Callbacks.h"
#include "InetAddress.h"

#include <boost/any.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

namespace muduo {
    class Channel;

    class EventLoop;

    class Socket;

    class TcpConnection : boost::noncopyable, public boost::enable_shared_from_this<TcpConnection> {
    public:
        /// Constructs a TcpConnection with a isConnected sockfd
        ///
        /// User should not create this object.
        TcpConnection(EventLoop *loop, const std::string &name, int sockfd, const InetAddress &localAddr,
                      const InetAddress &peerAddr);

        ~TcpConnection();

        EventLoop *getLoop() const {
            return loop;
        }

        const std::string &getName() const {
            return name;
        }

        const InetAddress &getLocalAddress() const {
            return localAddr;
        }

        const InetAddress &getPeerAddress() const {
            return peerAddr;
        }

        bool isConnected() const { return state == kConnected; }

        void setConnectionCallback(const ConnectionCallback &cb) {
            connectionCallback = cb;
        }

        void setMessageCallback(const MessageCallback &cb) {
            messageCallback = cb;
        }

        /// Internal use only.
        /// called when TcpServer accepts a new connection
        /// should be called only once
        void connectEstablished();

    private:
        enum StateE {
            kConnecting, kConnected
        };

        void setState(StateE s) {
            state = s;
        }

        void handleRead();

        EventLoop *loop;
        std::string name;

        StateE state;  // FIXME: use atomic variable
        boost::scoped_ptr<Socket> socket;
        boost::scoped_ptr<Channel> channel;
        InetAddress localAddr;
        InetAddress peerAddr;
        ConnectionCallback connectionCallback;
        MessageCallback messageCallback;
    };
}


#endif //TINY_MUDUO_TCPCONNECTION_H
