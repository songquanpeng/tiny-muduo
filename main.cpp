#include "Acceptor.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include "SocketsOps.h"
#include <cstdio>

void newConnection1(int sockfd, const muduo::InetAddress &peerAddr) {
    printf("newConnection(): accepted a new connection from %s\n",
           peerAddr.toHostPort().c_str());
    char res[] = "HTTP/1.1 200 OK\n"
                 "\n"
                 "<html>\n"
                 "<head><title>newConnection1</title></head>\n"
                 "<body>\n"
                 "<center><h1>newConnection1</h1></center>\n"
                 "<hr><center>This is newConnection1!</center>\n"
                 "</body>\n"
                 "</html>";
    ::write(sockfd, res, sizeof res);
    muduo::sockets::close(sockfd);
}

void newConnection2(int sockfd, const muduo::InetAddress &peerAddr) {
    printf("newConnection(): accepted a new connection from %s\n",
           peerAddr.toHostPort().c_str());
    char res[] = "HTTP/1.1 200 OK\n"
                 "\n"
                 "<html>\n"
                 "<head><title>newConnection2</title></head>\n"
                 "<body>\n"
                 "<center><h1>newConnection2</h1></center>\n"
                 "<hr><center>This is newConnection2!</center>\n"
                 "</body>\n"
                 "</html>";
    ::write(sockfd, res, sizeof res);
    muduo::sockets::close(sockfd);
}

int main() {
    printf("main(): pid = %d\n", getpid());

    muduo::InetAddress listenAddr1(3000);
    muduo::InetAddress listenAddr2(3001);
    muduo::EventLoop loop;

    muduo::Acceptor acceptor1(&loop, listenAddr1);
    muduo::Acceptor acceptor2(&loop, listenAddr2);
    acceptor1.setNewConnectionCallback(newConnection1);
    acceptor1.listen();

    acceptor2.setNewConnectionCallback(newConnection2);
    acceptor2.listen();

    loop.loop();
}
