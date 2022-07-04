//
// Created by song on 7/3/2022.
//

#ifndef TINY_MUDUO_TCPSERVER_H
#define TINY_MUDUO_TCPSERVER_H

#include "Callbacks.h"
#include "TcpConnection.h"
#include "InetAddress.h"

#include <map>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>

namespace muduo {
    class EventLoop;

    class Acceptor;

    class TcpServer : boost::noncopyable {
    public:
        TcpServer(EventLoop *loop, const InetAddress &listenAddr);

        ~TcpServer();

        /// Starts the server if it's not listening.
        ///
        /// It's harmless to call it multiple times.
        /// Thread safe.
        void start();

        /// Set connection callback.
        /// Not thread safe.
        void setConnectionCallback(const ConnectionCallback &cb) {
            connectionCallback = cb;
        }

        /// Set message callback.
        /// Not thread safe.
        void setMessageCallback(const MessageCallback &cb) {
            messageCallback = cb;
        }

    private:
        void newConnection(int sockfd, const InetAddress& peerAddr);
        void removeConnection(const TcpConnectionPtr& conn);
        typedef std::map<std::string, TcpConnectionPtr> ConnectionMap;
        EventLoop* loop;
        const std::string name;
        boost::scoped_ptr<Acceptor> acceptor;

        ConnectionCallback connectionCallback;
        MessageCallback messageCallback;

        bool started;
        int nextConnId;
        ConnectionMap connectionMap;
    };
}


#endif //TINY_MUDUO_TCPSERVER_H
