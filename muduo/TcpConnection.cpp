//
// Created by song on 7/3/2022.
//

#include "TcpConnection.h"

#include "logging/Logging.h"
#include "Channel.h"
#include "EventLoop.h"
#include "Socket.h"

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
    messageCallback(shared_from_this(), buf, n);
    // FIXME: close connection if n == 0
}

