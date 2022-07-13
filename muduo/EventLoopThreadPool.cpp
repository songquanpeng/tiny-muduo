//
// Created by song on 7/13/2022.
//

#include "EventLoopThreadPool.h"

#include "EventLoop.h"
#include "EventLoopThread.h"

#include <boost/bind.hpp>

using namespace muduo;

EventLoopThreadPool::EventLoopThreadPool(EventLoop *baseLoop)
        : baseLoop(baseLoop), isStarted(false), numThreads(0), next(0) {
}

EventLoopThreadPool::~EventLoopThreadPool() {

}

void EventLoopThreadPool::start() {
    assert(!isStarted);
    baseLoop->assertInLoopThread();
    isStarted = true;

    for (int i = 0; i < numThreads; i ++) {
        auto* t = new EventLoopThread;
        threads.push_back(t);
        loops.push_back(t->startLoop());
    }
}

EventLoop *EventLoopThreadPool::getNextLoop() {
    baseLoop->assertInLoopThread();
    EventLoop* loop = baseLoop;
    if (!loops.empty()) {
        loop = loops[next];
        next ++;
        if (next >= loops.size()) {
            next = 0;
        }
    }
    return loop;
}
