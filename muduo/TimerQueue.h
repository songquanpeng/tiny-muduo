//
// Created by song on 6/28/2022.
//

#ifndef TINY_MUDUO_TIMERQUEUE_H
#define TINY_MUDUO_TIMERQUEUE_H

#include <set>
#include <vector>
#include <boost/noncopyable.hpp>
#include "datetime/Timestamp.h"
#include "thread/Mutex.h"
#include "Callbacks.h"
#include "Channel.h"

namespace muduo {
    class EventLoop;

    class Timer;

    class TimerId;

    class TimerQueue : boost::noncopyable {
    public:
        TimerQueue(EventLoop *loop);

        ~TimerQueue();

        /// Schedules the callback to be run at given time,
        /// repeats if @c interval > 0.0.
        ///
        /// Must be thread safe. Usually be called from other threads.
        TimerId addTimer(const TimerCallback &cb, Timestamp when, double interval);

        void cancel(TimerId timerId);

    private:
        // TODO: typedef std::pair<Timestamp, std::unique_ptr<Timer>> Entry;
        typedef std::pair<Timestamp, Timer*> Entry;
        typedef std::set<Entry> TimerList;

        void addTimerInLoop(Timer* timer);

        // Called when timerfd alarms
        void handleRead();

        // Remove all expired timers
        std::vector<Entry> getExpired(Timestamp now);

        void reset(const std::vector<Entry> &expired, Timestamp now);

        bool insert(Timer *timer);

        EventLoop *loop;
        const int timerfd;
        Channel timerfdChannel;
        // Sorted by expiration
        TimerList timerList;

    };
}


#endif //TINY_MUDUO_TIMERQUEUE_H
