//
// Created by song on 7/3/2022.
//

#include "TcpConnection.h"

#include "logging/Logging.h"
#include "Channel.h"
#include "EventLoop.h"
#include "Socket.h"
#include "SocketsOps.h"

#include <boost/bind.hpp>

#include <errno.h>
#include <stdio.h>

using namespace muduo;


TcpConnection::TcpConnection(EventLoop *loop, const string &name, int sockfd, const InetAddress &localAddr,
                             const InetAddress &peerAddr) :
        loop(CHECK_NOTNULL(loop)),
        name(name),
        state(kConnecting),
        socket(new Socket(sockfd)),
        channel(new Channel(loop, sockfd)),
        localAddr(localAddr),
        peerAddr(peerAddr) {
    LOG_DEBUG << "TcpConnection::ctor[" << name << "] at " << this << " fd=" << sockfd;
    channel->setReadCallback(boost::bind(&TcpConnection::handleRead, this));
    channel->setWriteCallback(boost::bind(&TcpConnection::handleWrite, this));
    channel->setCloseCallback(boost::bind(&TcpConnection::handleClose, this));
    channel->setErrorCallback(boost::bind(&TcpConnection::handleError, this));
}


TcpConnection::~TcpConnection() {
    LOG_DEBUG << "TcpConnection::dtor[" << name << "] at " << this << " fd=" << channel->getFd();
}

void TcpConnection::connectEstablished() {
    loop->assertInLoopThread();
    assert(state == kConnecting);
    setState(kConnected);
    channel->enableReading();
    connectionCallback(shared_from_this());
}

void TcpConnection::handleRead() {
    char buf[65536];
    ssize_t n = read(channel->getFd(), buf, sizeof buf);
    if (n > 0) {
        messageCallback(shared_from_this(), buf, n);
    } else if (n == 0) {
        handleClose();
    }  else {
        handleError();
    }
}

void TcpConnection::handleWrite() {

}

void TcpConnection::handleClose() {
    loop->assertInLoopThread();
    LOG_TRACE << "TcpConnection::handleClose state = " << state;
    // let the dtor to close fd
    channel->disableAll();
    // must be the last line!
    closeCallback(shared_from_this());
}

void TcpConnection::handleError() {
    int err = sockets::getSocketError(channel->getFd());
    LOG_ERROR << "TcpConnection::handleError [" << name << "] - SO_ERROR = " << err << strerror_tl(err);
}

void TcpConnection::connectDestroyed() {
    loop->assertInLoopThread();
    assert(state == kConnected);
    setState(kDisconnected);
    channel->disableAll();
    connectionCallback(shared_from_this());
    loop->removeChannel(boost::get_pointer(channel));
}

