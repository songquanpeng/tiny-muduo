//
// Created by song on 6/25/2022.
//

#ifndef TINY_MUDUO_EVENTLOOP_H
#define TINY_MUDUO_EVENTLOOP_H


#include "thread/Thread.h"

namespace muduo {
    class EventLoop : boost::noncopyable {
    public:
        EventLoop();

        ~EventLoop();

        void loop();

        bool isInLoopThread() const {
            return threadId == CurrentThread::tid();
        }

        void assertInLoopThread() {
            if (!isInLoopThread()) {
                abortNotInLoopThread();
            }
        }

    private:
        void abortNotInLoopThread();

        bool isLooping;
        const pid_t threadId;
    };
}

#endif //TINY_MUDUO_EVENTLOOP_H
