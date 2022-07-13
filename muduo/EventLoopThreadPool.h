//
// Created by song on 7/13/2022.
//

#ifndef TINY_MUDUO_EVENTLOOPTHREADPOOL_H
#define TINY_MUDUO_EVENTLOOPTHREADPOOL_H

#include "thread/Condition.h"
#include "thread/Mutex.h"
#include "thread/Thread.h"


#include <vector>
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

namespace muduo {
    class EventLoop;

    class EventLoopThread;

    class EventLoopThreadPool : boost::noncopyable {
    public:
        EventLoopThreadPool(EventLoop *baseLoop);

        ~EventLoopThreadPool();

        void setThreadNum(int numThreads) {
            this->numThreads = numThreads;
        }
        void start();
        EventLoop* getNextLoop();

    private:
        EventLoop *baseLoop;
        bool isStarted;
        int numThreads;
        int next;
        boost::ptr_vector<EventLoopThread> threads;
        std::vector<EventLoop *> loops;
    };
}

#endif //TINY_MUDUO_EVENTLOOPTHREADPOOL_H
