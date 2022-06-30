//
// Created by song on 6/30/2022.
//

#include "EventLoopThread.h"
#include "EventLoop.h"
#include <boost/bind.hpp>

using namespace muduo;

EventLoopThread::EventLoopThread()
        : loop(nullptr),
          exiting(false),
          thread(boost::bind(&EventLoopThread::threadFunc, this)),
          mutex(),
          cond(mutex) {

}

EventLoopThread::~EventLoopThread() {
    exiting = true;
    loop->quit();
    // Blocked here to wait thread end.
    thread.join();
}

EventLoop *EventLoopThread::startLoop() {
    assert(!thread.started());
    thread.start();
    {
        MutexLockGuard lock(mutex);
        // Actually this "while" is very interesting, but it's not necessary here.
        // Cause there is no other competitor. No need to worry Spurious wakeup.
        while (loop == nullptr) {
            // Block current thread, wait other thread to call pthread_cond_signal() or pthread_cond_broadcast().
            cond.wait();  // This will temporarily release unlock the mutex. After that it will lock it again.
        }
    }
    return loop;
}

void EventLoopThread::threadFunc() {
    EventLoop localLoop;
    {
        MutexLockGuard lock(mutex);
        loop = &localLoop;
        cond.notify();
    }
    localLoop.loop();
}
