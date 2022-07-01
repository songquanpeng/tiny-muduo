//
// Created by song on 7/1/2022.
//

#include "Acceptor.h"


#include "logging/Logging.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include "SocketsOps.h"

#include <boost/bind.hpp>

using namespace muduo;

Acceptor::Acceptor(EventLoop *loop, const InetAddress &listenAddr)
        : loop(loop),
          acceptSocket(sockets::createNonblockingOrDie()),
          acceptChannel(loop, acceptSocket.fd()),
          listening(false) {
    acceptSocket.setReuseAddr(true);
    acceptSocket.bindAddress(listenAddr);
    acceptChannel.setReadCallback(boost::bind(&Acceptor::handleRead, this));
}

void Acceptor::listen() {
    loop->assertInLoopThread();
    listening = true;
    acceptSocket.listen();
    acceptChannel.enableReading();
}

void Acceptor::handleRead() {
    loop->assertInLoopThread();
    InetAddress peerAddr(0);
    // FIXME here we only accept one, maybe we can make a loop to accept all possible ones.
    int connfd = acceptSocket.accept(&peerAddr);
    if (connfd >= 0) {
        if (newConnectionCallback) {
            newConnectionCallback(connfd, peerAddr);
        } else {
            sockets::close(connfd);
        }
    }
}
