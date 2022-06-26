//
// Created by song on 6/25/2022.
//

#include "EventLoop.h"
#include "logging/Logging.h"
#include <cassert>
#include <poll.h>

using namespace muduo;

__thread EventLoop *eventLoopOfThisThread = nullptr;

EventLoop::EventLoop() : isLooping(false), threadId(CurrentThread::tid()) {
    LOG_TRACE << "EventLoop " << this << " created in thread " << threadId;
    if (eventLoopOfThisThread) {
        LOG_FATAL << "Another EventLoop " << eventLoopOfThisThread << " already in this thread " << threadId;
    } else {
        eventLoopOfThisThread = this;
    }
}

EventLoop::~EventLoop() {
    assert(!isLooping);
    eventLoopOfThisThread = nullptr;
}

void EventLoop::loop() {
    assert(!isLooping);
    assertInLoopThread();
    isLooping = true;
    poll(nullptr, 0, 5 * 1000);
    LOG_TRACE << "EventLoop " << this << " stop looping";
    isLooping = false;
}

void EventLoop::abortNotInLoopThread() {
    LOG_FATAL << "EventLop::abortNotInLoopThread - EventLoop " << this << " was created in thread " << threadId
              << " but current thread is " << CurrentThread::tid();
}