//
// Created by song on 7/3/2022.
//

#include "TcpServer.h"
#include "logging/Logging.h"
#include "Acceptor.h"
#include "EventLoop.h"
#include "SocketsOps.h"

#include <boost/bind.hpp>

#include <cstdio>  // snprintf

using namespace muduo;

TcpServer::TcpServer(EventLoop *loop, const InetAddress &listenAddr)
        : loop(CHECK_NOTNULL(loop)),
          name(listenAddr.toHostPort()),
          acceptor(new Acceptor(loop, listenAddr)),
          started(false),
          nextConnId(1) {
    acceptor->setNewConnectionCallback(boost::bind(&TcpServer::newConnection, this, _1, _2));
}

TcpServer::~TcpServer() = default;

void TcpServer::start() {
    if (!started) {
        started = true;
    }
    if (!acceptor->isListening()) {
        loop->runInLoop(boost::bind(&Acceptor::listen, boost::get_pointer(acceptor)));
    }
}

void TcpServer::newConnection(int sockfd, const InetAddress &peerAddr) {
    loop->assertInLoopThread();
    char buf[32];
    snprintf(buf, sizeof buf, "#%d", nextConnId);
    ++nextConnId;
    std::string connName = name + buf;

    LOG_INFO << "TcpServer::newConnection [" << name << "] - new connection [" << connName << "] from "
             << peerAddr.toHostPort();
    InetAddress localAddr(sockets::getLocalAddr(sockfd));
    // FIXME poll with zero timeout to double confirm the new connection
    TcpConnectionPtr conn(
            new ::TcpConnection(loop, connName, sockfd, localAddr, peerAddr));  // TODO: try to use make_new
    connectionMap[connName] = conn;
    conn->setConnectionCallback(connectionCallback);
    conn->setMessageCallback(messageCallback);
    conn->connectEstablished();
}
