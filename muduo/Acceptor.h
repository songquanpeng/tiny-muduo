//
// Created by song on 7/1/2022.
//

#ifndef TINY_MUDUO_ACCEPTOR_H
#define TINY_MUDUO_ACCEPTOR_H

#include <boost/noncopyable.hpp>
#include <boost/function.hpp>

#include "Channel.h"
#include "Socket.h"


namespace muduo {
    class Acceptor : boost::noncopyable {
    public:
        typedef boost::function<void (int sockfd, const InetAddress&)> NewConnectionCallback;
        Acceptor(EventLoop* loop, const InetAddress& listenAddr);

        void setNewConnectionCallback(const NewConnectionCallback& cb) {
            newConnectionCallback = cb;
        }

        bool isListening() const {
            return listening;
        }

        void listen();

    private:
        // Will call accept(2) to accept new connection and call user's callback
        void handleRead();

        EventLoop* loop;
        Socket acceptSocket;
        // Channel is used to watch readable event of above socket, and then call handleRead()
        Channel acceptChannel;
        NewConnectionCallback newConnectionCallback;
        bool listening;
    };
}





#endif //TINY_MUDUO_ACCEPTOR_H
