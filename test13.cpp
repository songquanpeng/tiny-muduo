#include "EventLoop.h"
#include "InetAddress.h"
#include "TcpClient.h"

#include "logging/Logging.h"

#include <boost/bind.hpp>

#include <utility>

#include <cstdio>
#include <unistd.h>

std::string message = "Hello\n";

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

void onMessage(const muduo::TcpConnectionPtr &conn,
               muduo::Buffer *buf,
               muduo::Timestamp receiveTime) {
    printf("onMessage(): received %zd bytes from connection [%s] at %s\n",
           buf->readableBytes(),
           conn->getName().c_str(),
           receiveTime.toFormattedString().c_str());

    printf("onMessage(): [%s]\n", buf->retrieveAsString().c_str());
}

int main() {
    muduo::EventLoop loop;
    muduo::InetAddress serverAddr("localhost", 9981);
    muduo::TcpClient client(&loop, serverAddr);

    client.setConnectionCallback(onConnection);
    client.setMessageCallback(onMessage);
    client.enableRetry();
    client.connect();
    loop.loop();
}

