//
// Created by song on 7/3/2022.
//

#include "TcpServer.h"
#include "logging/Logging.h"
#include "Acceptor.h"
#include "EventLoop.h"
#include "SocketsOps.h"
#include "EventLoopThreadPool.h"

#include <boost/bind.hpp>

#include <cstdio>  // snprintf

using namespace muduo;

TcpServer::TcpServer(EventLoop *loop, const InetAddress &listenAddr)
        : loop(CHECK_NOTNULL(loop)),
          name(listenAddr.toHostPort()),
          acceptor(new Acceptor(loop, listenAddr)),
          threadPool(new EventLoopThreadPool(loop)),
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
    EventLoop* ioLoop = threadPool->getNextLoop();
    // FIXME poll with zero timeout to double confirm the new connection
    TcpConnectionPtr conn(
            new ::TcpConnection(ioLoop, connName, sockfd, localAddr, peerAddr));  // TODO: try to use make_new
    connectionMap[connName] = conn;
    conn->setConnectionCallback(connectionCallback);
    conn->setMessageCallback(messageCallback);
    conn->setCloseCallback(boost::bind(&TcpServer::removeConnection, this, _1));
    conn->setWriteCompleteCallback(writeCompleteCallback);
    // conn->connectEstablished();  // Use next line instead.
    ioLoop->runInLoop(boost::bind(&TcpConnection::connectEstablished, conn));
}

void TcpServer::removeConnection(const TcpConnectionPtr &conn) {
    // P326
    // Go to the main loop, otherwise we will need lock
    loop->runInLoop(boost::bind(&TcpServer::removeConnectionInLoop, this, conn));
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr &conn) {
    loop->assertInLoopThread();
    LOG_INFO << "TcpServer::removeConnectionInLoop [" << name << "] - connection " << conn->getName();
    auto n = connectionMap.erase(conn->getName());
    assert(n == 1);
    (void) n; // ???
    // Move back to conn's own loop. It's good for client code's convenience.
    EventLoop* ioLoop =  conn->getLoop();
    // queueInLoop() is necessary.
    // P311
    ioLoop->queueInLoop(boost::bind(&TcpConnection::connectDestroyed, conn));
}

void TcpServer::setThreadNum(int numThreads) {
    assert(numThreads >= 0);
    threadPool->setThreadNum(numThreads);
}
