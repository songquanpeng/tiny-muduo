# Tiny Muduo

## Overview


### For User
```mermaid
sequenceDiagram
    actor User
    participant EventLoop
    participant TcpServer
    User->>EventLoop: call init()
    User->>TcpServer: call init(loop, addr)
    User->>TcpServer: call set*Connection()
    User->>TcpServer: call setThreadNum()
    User->>TcpServer: call start()
    User->>EventLoop: call loop()
```

### For TcpServer
```mermaid
sequenceDiagram
    participant TcpServer
    participant EventLoopThreadPool
    participant TcpConnection
    participant Acceptor
    participant Socket
    participant Channel
    participant EventLoop
    participant Poller
    participant OS
    
    TcpServer->>TcpServer: init()
    TcpServer->>Acceptor: call init(loop, addr)
    Acceptor->>Socket: call create()
    Socket-->>Acceptor: fd
    Acceptor->>Channel: call init(loop, fd)
    Acceptor->>Socket: call bind(addr)
    Acceptor->>Channel: call setReadCallback(handleRead)
    TcpServer->>EventLoopThreadPool: call init(loop)
    TcpServer->>Acceptor: call setNewConnectionCallback(newConnection)
    
    TcpServer->>TcpServer: start()
    TcpServer->>EventLoop: call runInLoop(Acceptor::listen)
    EventLoop->>EventLoop: queueInLoop()
    EventLoop->>EventLoop: wakeUp()
    EventLoop->>EventLoop: loop()
    EventLoop->>EventLoop: doPendingFunctors()
    EventLoop->>Acceptor: call listen()
    Acceptor->>Socket: call listen()
    
    Acceptor->>Channel: call enableReading()
    Channel->>EventLoop: call updateChannel()
    EventLoop->>Poller: call updateChannel()
    Poller->Poller: update pollFdList
    
    EventLoop->>EventLoop: loop()
    EventLoop->>Poller: call poll()
    Poller->>OS: call poll(pollFdList)
    OS-->>Poller: update pollFdList
    Poller->>Poller: fillActiveChannels(pollFdList)
    Poller-->>EventLoop: return activeChannels
    EventLoop->>Channel: call handleEvent()
    
    Channel->>Acceptor: call handleRead()
    Acceptor->>Socket: call accept()
    Socket-->>Acceptor: return connfd, peerAddr
    Acceptor->>TcpServer: call newConnectionCallback(connfd, peerAddr)
    
    TcpServer->>TcpServer: newConnection()
    TcpServer->>EventLoopThreadPool: call getNextLoop()
    EventLoopThreadPool-->>TcpServer: return ioLoop
    TcpServer->>TcpConnection: call init(ioLoop, connfd, peerAddr)
    TcpConnection->>Socket: call init(connfd)
    TcpConnection->>Channel: call init(ioLoop, connfd)
    TcpServer->>TcpConnection: call set*Callback
    TcpConnection->>Channel: call set*Callback(TcpConnection::handle*)
    TcpServer->>EventLoop: call runInLoop(TcpConnection::connectEstablished, conn)
    
```

### Timer
```mermaid
sequenceDiagram
    participant TcpServer
    participant Timer
    participant Timer Queue

```

## Setup
```shell
sudo apt install build-essential cmake libboost-dev
```

## Reference
https://github.com/chenshuo/recipes