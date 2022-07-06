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
    channel->setReadCallback(boost::bind(&TcpConnection::handleRead, this, _1));
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

void TcpConnection::handleRead(Timestamp receiveTime) {
    int saveErrno = 0;
    ssize_t n = inputBuffer.readFd(channel->getFd(), &saveErrno);
    if (n > 0) {
        messageCallback(shared_from_this(), &inputBuffer, receiveTime);
    } else if (n == 0) {
        handleClose();
    } else {
        errno = saveErrno;
        LOG_SYSERR << "TcpConnection::handleRead";
        handleError();
    }
}

// TODO: p319
void TcpConnection::handleWrite() {
    loop->assertInLoopThread();
    if (channel->isWriting()) {
        auto n = write(channel->getFd(), outputBuffer.peek(), outputBuffer.readableBytes());
        if (n > 0) {
            outputBuffer.retrieve(n);
            if (outputBuffer.readableBytes() == 0) {
                channel->disableWriting();
                if (state == kDisconnecting) {
                    shutdownInLoop();
                }
            } else {
                LOG_TRACE << "Going to write more data";
            }
        } else {
            LOG_SYSERR << "TcpConnection::handleWrite";
        }
    } else {
        LOG_TRACE << "Connection is down, writing no more";
    }
}

void TcpConnection::handleClose() {
    loop->assertInLoopThread();
    LOG_TRACE << "TcpConnection::handleClose state = " << state;
    // TODO: why those two states?
    assert(state == kConnected || state == kDisconnecting);
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
    // TODO: why those two states?
    assert(state == kConnected || state == kDisconnecting);
    setState(kDisconnected);
    channel->disableAll();
    connectionCallback(shared_from_this());
    loop->removeChannel(boost::get_pointer(channel));
}

void TcpConnection::send(const string &message) {
    if (state == kConnected) {
        if (loop->isInLoopThread()) {
            sendInLoop(message);
        } else {
            loop->runInLoop(boost::bind(&TcpConnection::sendInLoop, this, message));
        }
    }
}

// TODO: p318
void TcpConnection::sendInLoop(const string &message) {
    loop->assertInLoopThread();
    ssize_t nwrote = 0;
    // In nothing left in the output queue, try writing directly
    // TODO: why? P318
    if (!channel->isWriting() && outputBuffer.readableBytes() == 0) {
        nwrote = write(channel->getFd(), message.data(), message.size());
        if (nwrote >= 0) {
            if (implicit_cast<size_t>(nwrote) < message.size()) {
                LOG_TRACE << "Going to write more data";
            }
        } else {
            nwrote = 0;
            if (errno != EWOULDBLOCK) {
                LOG_SYSERR << "TcpConnection::sendInLoop";
            }
        }
    }

    assert(nwrote >= 0);
    if (implicit_cast<size_t>(nwrote) < message.size()) {
        outputBuffer.append(message.data() + nwrote, message.size() - nwrote);
        if (!channel->isWriting()) {
            channel->enableWriting();
        }
    }
}

void TcpConnection::shutdown() {
    // FIXME: use compare & swap
    if (state == kConnected) {
        setState(kDisconnecting);
        loop->runInLoop(boost::bind(&TcpConnection::shutdownInLoop, this));
    }
}


void TcpConnection::shutdownInLoop() {
    loop->assertInLoopThread();
    if (!channel->isWriting()) {
        socket->shutdownWrite();
    }
}

