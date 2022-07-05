#include "TcpServer.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include <cstdio>

void onConnection(const muduo::TcpConnectionPtr &conn) {
    if (conn->isConnected()) {
        printf("onConnection(): new connection [%s] from %s\n", conn->getName().c_str(),
               conn->getPeerAddress().toHostPort().c_str());
    } else {
        printf("onConnection(): connection [%s] is down\n", conn->getName().c_str());
    }
}

void onMessage(const muduo::TcpConnectionPtr &conn, muduo::Buffer *buf, muduo::Timestamp receiveTime) {
    printf("onMessage(): received %zd bytes from connection [%s] at %s\n",
           buf->readableBytes(), conn->getName().c_str(), receiveTime.toFormattedString().c_str());
    printf("onMessage(): [%s]\n", buf->retrieveAsString().c_str());
}

int main() {
    printf("main(): pid = %d\n", getpid());

    muduo::InetAddress listenAddr(9981);
    muduo::EventLoop loop;

    muduo::TcpServer server(&loop, listenAddr);
    server.setConnectionCallback(onConnection);
    server.setMessageCallback(onMessage);
    server.start();

    loop.loop();
}
