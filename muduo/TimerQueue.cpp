//
// Created by song on 6/28/2022.
//

#include "TimerQueue.h"
#include "logging/Logging.h"
#include "EventLoop.h"
#include "Timer.h"
#include "TimerId.h"

#include <boost/foreach.hpp>
#include <boost/bind.hpp>
#include <sys/timerfd.h>

namespace muduo {
    namespace detail {
        int createTimerfd() {
            int timerfd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
            if (timerfd < 0) {
                LOG_SYSFATAL << "Failed in timerfd_create";
            }
            return timerfd;
        }

        struct timespec howMuchTimeFromNow(Timestamp when) {
            int64_t ms = when.microSecondsSinceEpoch() - Timestamp::now().microSecondsSinceEpoch();
            if (ms < 100) {
                ms = 100;
            }
            struct timespec ts;
            ts.tv_sec = (time_t) (ms / Timestamp::kMicroSecondsPerSecond);
            ts.tv_nsec = (long) (ms % Timestamp::kMicroSecondsPerSecond * 1000);
            return ts;
        }

        void readTimerfd(int timerfd, Timestamp now) {
            uint64_t howmany;
            ssize_t n = ::read(timerfd, &howmany, sizeof howmany);
            LOG_TRACE << "TimerQueue::handleRead() " << howmany << " at " << now.toString();
            if (n != sizeof howmany) {
                LOG_ERROR << "TimerQueue::handleRead() reads " << n << " bytes instead of 8";
            }
        }

        void resetTimerfd(int timerfd, Timestamp expiration) {
            // wake up loop by timerfd_settime()
            struct itimerspec newValue;
            struct itimerspec oldValue;
            bzero(&newValue, sizeof newValue);
            bzero(&oldValue, sizeof oldValue);
            newValue.it_value = howMuchTimeFromNow(expiration);
            int ret = ::timerfd_settime(timerfd, 0, &newValue, &oldValue);
            if (ret) {
                LOG_SYSERR << "timerfd_settime()";
            }
        }
    }
}

using namespace muduo;
using namespace muduo::detail;

TimerQueue::TimerQueue(EventLoop *loop)
        : loop(loop), timerfd(createTimerfd()), timerfdChannel(loop, timerfd), timerList(),
          callingExpiredTimers(false) {
    timerfdChannel.setReadCallback(boost::bind(&TimerQueue::handleRead, this));
    timerfdChannel.enableReading();
}

TimerQueue::~TimerQueue() {
    ::close(timerfd);
    // do not remove channel, since we're in EventLoop::dtor();

    for (const auto &it: timerList) {
        delete it.second;
    }
}

TimerId TimerQueue::addTimer(const TimerCallback &cb, Timestamp when, double interval) {
    auto *timer = new Timer(cb, when, interval);
    loop->runInLoop(boost::bind(&TimerQueue::addTimerInLoop, this, timer));
    return TimerId(timer);
}

void TimerQueue::addTimerInLoop(Timer *timer) {
    loop->assertInLoopThread();
    bool earliestChanged = insert(timer);

    if (earliestChanged) {
        resetTimerfd(timerfd, timer->getExpirationTimestamp());
    }
}


void TimerQueue::handleRead() {
    loop->assertInLoopThread();
    Timestamp now(Timestamp::now());
    readTimerfd(timerfd, now);

    std::vector<Entry> expired = getExpired(now);
    callingExpiredTimers = true;
    cancelingTimerSet.clear();

    // safe to callback outside critical section
    for (auto &it: expired) {
        it.second->run();
    }
    callingExpiredTimers = false;

    reset(expired, now);
}

std::vector<TimerQueue::Entry> TimerQueue::getExpired(Timestamp now) {
    assert(timerList.size() == activeTimerSet.size());
    std::vector<Entry> expired;
    Entry sentry = std::make_pair(now, reinterpret_cast<Timer *>(UINTPTR_MAX));
    auto it = timerList.lower_bound(sentry);
    assert(it == timerList.end() || now < it->first);
    std::copy(timerList.begin(), it, back_inserter(expired));
    timerList.erase(timerList.begin(), it);

    BOOST_FOREACH(Entry entry, expired) {
                    ActiveTimer timer(entry.second, entry.second->getSequence());
                    auto n = activeTimerSet.erase(timer);
                    assert(n == 1);
                }
    assert(timerList.size() == activeTimerSet.size());
    return expired;
}

void TimerQueue::reset(const std::vector<Entry> &expired, Timestamp now) {
    Timestamp nextExpire;

    for (const auto &it: expired) {
        ActiveTimer timer(it.second, it.second->getSequence());
        if (it.second->isRepeat() && cancelingTimerSet.find(timer) == cancelingTimerSet.end()) {
            it.second->restart(now);
            insert(it.second);
        } else {
            // FIXME move to a free list
            delete it.second;
        }
    }

    if (!timerList.empty()) {
        nextExpire = timerList.begin()->second->getExpirationTimestamp();
    }

    if (nextExpire.valid()) {
        resetTimerfd(timerfd, nextExpire);
    }
}

bool TimerQueue::insert(Timer *timer) {
    loop->assertInLoopThread();
    assert(timerList.size() == activeTimerSet.size());
    bool earliestChanged = false;  // will the new added timer be the new first timer to alert?
    Timestamp when = timer->getExpirationTimestamp();
    auto it = timerList.begin();
    if (it == timerList.end() || when < it->first) {
        earliestChanged = true;
    }
    {
        std::pair<TimerList::iterator, bool> result
                = timerList.insert(Entry(when, timer));
        assert(result.second);
        (void) result;
    }
    {
        std::pair<ActiveTimerSet::iterator, bool> result
                = activeTimerSet.insert(ActiveTimer(timer, timer->getSequence()));
        assert(result.second);
        (void) result;
    }
    assert(timerList.size() == activeTimerSet.size());
    return earliestChanged;
}

void TimerQueue::cancel(TimerId timerId) {
    loop->runInLoop(boost::bind(&TimerQueue::cancelInLoop, this, timerId));
}

void TimerQueue::cancelInLoop(TimerId timerId) {
    loop->assertInLoopThread();
    assert(timerList.size() == activeTimerSet.size());
    ActiveTimer timer(timerId.timer, timerId.sequence);
    auto it = activeTimerSet.find(timer);
    if (it != activeTimerSet.end()) {
        auto n = timerList.erase(Entry(it->first->getExpirationTimestamp(), it->first));
        assert(n == 1);
        delete it->first;
        activeTimerSet.erase(it);
    } else if (callingExpiredTimers) {
        cancelingTimerSet.insert(timer);
    }
    assert(timerList.size() == activeTimerSet.size());
}

