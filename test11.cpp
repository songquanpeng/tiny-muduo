#include "TcpServer.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include <cstdio>

std::string message;

void onConnection(const muduo::TcpConnectionPtr &conn) {
    if (conn->isConnected()) {
        printf("onConnection(): new connection [%s] from %s\n",
               conn->getName().c_str(),
               conn->getPeerAddress().toHostPort().c_str());
        conn->send(message);
    } else {
        printf("onConnection(): connection [%s] is down\n",
               conn->getName().c_str());
    }
}

void onWriteComplete(const muduo::TcpConnectionPtr &conn) {
    conn->send(message);
}

void onMessage(const muduo::TcpConnectionPtr &conn,
               muduo::Buffer *buf,
               muduo::Timestamp receiveTime) {
    printf("onMessage(): received %zd bytes from connection [%s] at %s\n",
           buf->readableBytes(),
           conn->getName().c_str(),
           receiveTime.toFormattedString().c_str());

    buf->retrieveAll();
}

int main() {
    printf("main(): pid = %d\n", getpid());

    std::string line;
    for (int i = 33; i < 127; ++i) {
        line.push_back(char(i));
    }
    line += line;

    for (size_t i = 0; i < 127 - 33; ++i) {
        message += line.substr(i, 72) + '\n';
    }

    muduo::InetAddress listenAddr(9981);
    muduo::EventLoop loop;

    muduo::TcpServer server(&loop, listenAddr);
    server.setConnectionCallback(onConnection);
    server.setMessageCallback(onMessage);
    server.setWriteCompleteCallback(onWriteComplete);
    server.start();

    loop.loop();
}
