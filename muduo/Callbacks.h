//
// Created by song on 6/28/2022.
//

#ifndef TINY_MUDUO_CALLBACKS_H
#define TINY_MUDUO_CALLBACKS_H

#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>

#include "datetime/Timestamp.h"

namespace muduo {
    class Buffer;
    class TcpConnection;

    typedef boost::shared_ptr<TcpConnection> TcpConnectionPtr;

    typedef boost::function<void()> TimerCallback;
    typedef boost::function<void(const TcpConnectionPtr &)> ConnectionCallback;
    typedef boost::function<void(const TcpConnectionPtr &, Buffer* buf, Timestamp)> MessageCallback;
    typedef boost::function<void (const TcpConnectionPtr&)> CloseCallback;
    typedef boost::function<void (const TcpConnectionPtr&)> WriteCompleteCallback;
    typedef std::function<void (const TcpConnectionPtr&, size_t)> HighWaterLevelCallback;
}

#endif //TINY_MUDUO_CALLBACKS_H
