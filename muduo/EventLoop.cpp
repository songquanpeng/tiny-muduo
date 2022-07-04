//
// Created by song on 6/25/2022.
//

#include "EventLoop.h"
#include "Channel.h"
#include "Poller.h"
#include "logging/Logging.h"
#include "TimerQueue.h"
#include "Channel.h"
#include <cassert>
#include <poll.h>
#include <sys/eventfd.h>
#include <boost/bind.hpp>

using namespace muduo;

__thread EventLoop *eventLoopOfThisThread = nullptr;
const int kPollTimeMs = 10000;

static int createEventfd() {
    int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (evtfd < 0) {
        LOG_SYSERR << "Failed in eventfd";
        abort();
    }
    return evtfd;
}

EventLoop::EventLoop() :
        isLooping(false),
        isQuited(false),
        threadId(CurrentThread::tid()),
        poller(new Poller(this)),
        timerQueue(new TimerQueue(this)),
        wakeupFd(createEventfd()),
        wakeupChannel(new Channel(this, wakeupFd)) {
    LOG_TRACE << "EventLoop " << this << " created in thread " << threadId;
    if (eventLoopOfThisThread) {
        LOG_FATAL << "Another EventLoop " << eventLoopOfThisThread << " already in this thread " << threadId;
    } else {
        eventLoopOfThisThread = this;
    }

    wakeupChannel->setReadCallback(boost::bind(&EventLoop::handleRead, this));
    wakeupChannel->enableReading();

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
        doPendingFunctors();
    }
    LOG_TRACE << "EventLoop " << this << " stop looping";
    isLooping = false;
}

void EventLoop::quit() {
    isQuited = true;
    if (!isInLoopThread()) {
        wakeUp();
    }
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

void EventLoop::removeChannel(Channel *channel) {
    assert(channel->getOwnerLoop() == this);
    assertInLoopThread();
    poller->removeChannel(channel);
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

void EventLoop::runInLoop(const Functor &cb) {
    if (isInLoopThread()) {
        cb();
    } else {
        queueInLoop(cb);
    }
}

void EventLoop::queueInLoop(const EventLoop::Functor &cb) {
    {
        MutexLockGuard lock(mutex);
        pendingFunctors.push_back(cb);
    }
    // If not in IO thread, we have to wake up it
    // If we are calling pending functor, we have to wake up it.
    // See P295 for reason.
    // This is because, when we are callingPendingFunctors, those called functors may add new functor
    // to the pendingFunctors. So we should call wakeUp() to make the next poll() return ASAP.
    // Otherwise, the new added factors will have to wait other event happened or poll timeout.
    if (!isInLoopThread() || callingPendingFunctors) {
        wakeUp();
    }
}

void EventLoop::wakeUp() {
    uint64_t one = 1;
    auto n = write(wakeupFd, &one, sizeof one);
    if (n != sizeof one) {
        LOG_ERROR << "EventLoop::wakeup() writes " << n << " bytes instead of 8";
    }
}

void EventLoop::handleRead() {
    uint64_t one = 1;
    auto n = ::read(wakeupFd, &one, sizeof one);
    if (n != sizeof one) {
        LOG_ERROR << "EventLoop::handleRead() reads " << n << " bytes instead of 8";
    }
}


void EventLoop::doPendingFunctors() {
    std::vector<Functor> functors;
    callingPendingFunctors = true;
    {
        MutexLockGuard lock(mutex);
        functors.swap(pendingFunctors);
    }
    for (auto &functor: functors) {
        functor();
    }
    callingPendingFunctors = false;
}
