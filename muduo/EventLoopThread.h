//
// Created by song on 6/30/2022.
//

#ifndef TINY_MUDUO_EVENTLOOPTHREAD_H
#define TINY_MUDUO_EVENTLOOPTHREAD_H

#include "thread/Condition.h"
#include "thread/Mutex.h"
#include "thread/Thread.h"

#include <boost/noncopyable.hpp>

namespace muduo {
    class EventLoop;

    class EventLoopThread : boost::noncopyable {
    public:
        EventLoopThread();
        ~EventLoopThread();
        EventLoop* startLoop();

    private:
        void threadFunc();
        EventLoop* loop;
        bool exiting;
        Thread thread;
        MutexLock mutex;
        Condition cond;
    };
}



#endif //TINY_MUDUO_EVENTLOOPTHREAD_H
