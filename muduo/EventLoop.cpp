//
// Created by song on 6/25/2022.
//

#include "EventLoop.h"
#include "Channel.h"
#include "Poller.h"
#include "logging/Logging.h"
#include "TimerQueue.h"
#include <cassert>
#include <poll.h>

using namespace muduo;

__thread EventLoop *eventLoopOfThisThread = nullptr;
const int kPollTimeMs = 10000;

EventLoop::EventLoop() : isLooping(false), isQuited(false), threadId(CurrentThread::tid()), poller(new Poller(this)),
                         timerQueue(new TimerQueue(this)) {
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
    isQuited = false;
    while (!isQuited) {
        activeChannels.clear();
        pollReturnTime = poller->poll(kPollTimeMs, &activeChannels);
        for (auto activeChannel: activeChannels) {
            activeChannel->handleEvent();
        }
    }
    LOG_TRACE << "EventLoop " << this << " stop looping";
    isLooping = false;
}

void EventLoop::quit() {
    isQuited = true;
}


void EventLoop::abortNotInLoopThread() {
    LOG_FATAL << "EventLop::abortNotInLoopThread - EventLoop " << this << " was created in thread " << threadId
              << " but current thread is " << CurrentThread::tid();
}

void EventLoop::updateChannel(Channel *channel) {
    assert(channel->getOwnerLoop() == this);
    assertInLoopThread();
    poller->updateChannel(channel);
}

TimerId EventLoop::runAt(const Timestamp &time, const TimerCallback &cb) {
    return timerQueue->addTimer(cb, time, 0);
}

TimerId EventLoop::runAfter(double delay, const TimerCallback &cb) {
    Timestamp time(addTime(Timestamp::now(), delay));
    return runAt(time, cb);
}

TimerId EventLoop::runEvery(double interval, const TimerCallback &cb) {
    Timestamp time(addTime(Timestamp::now(), interval));
    return timerQueue->addTimer(cb, time, interval);
}
