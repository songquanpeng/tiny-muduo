//
// Created by song on 6/25/2022.
//

#ifndef TINY_MUDUO_EVENTLOOP_H
#define TINY_MUDUO_EVENTLOOP_H


#include "datetime/Timestamp.h"
#include "TimerId.h"
#include "Callbacks.h"
#include "thread/Thread.h"
#include "thread/Mutex.h"
#include <boost/scoped_ptr.hpp>
#include <vector>

namespace muduo {
    class Channel;

    class Poller;

    class TimerQueue;

    class EventLoop : boost::noncopyable {
    public:
        EventLoop();

        ~EventLoop();

        void loop();

        void quit();

        Timestamp getPollReturnTime() const { return pollReturnTime; }

        void updateChannel(Channel *channel);
        void removeChannel(Channel *channel);

        bool isInLoopThread() const {
            return threadId == CurrentThread::tid();
        }

        void assertInLoopThread() {
            if (!isInLoopThread()) {
                abortNotInLoopThread();
            }
        }

        typedef boost::function<void()> Functor;

        /// Runs callback immediately in the loop thread.
        /// It wakes up the loop, and run the cb.
        /// If in the same loop thread, cb is run within the function.
        /// Safe to call from other threads.
        void runInLoop(const Functor& cb);
        /// Queues callback in the loop thread.
        /// Runs after finish pooling.
        /// Safe to call from other threads.
        void queueInLoop(const Functor& cb);
        void wakeUp();

        ///
        /// Runs callback at 'time'.
        ///
        TimerId runAt(const Timestamp &time, const TimerCallback &cb);

        ///
        /// Runs callback after @c delay seconds.
        ///
        TimerId runAfter(double delay, const TimerCallback &cb);

        ///
        /// Runs callback every @c interval seconds.
        ///
        TimerId runEvery(double interval, const TimerCallback &cb);

        void cancel(TimerId timerId);

    private:
        void abortNotInLoopThread();
        void handleRead();  // waked up
        void doPendingFunctors();

        typedef std::vector<Channel *> ChannelList;

        bool isLooping;
        bool isQuited;
        const pid_t threadId;
        Timestamp pollReturnTime;
        boost::scoped_ptr<Poller> poller;
        boost::scoped_ptr<TimerQueue> timerQueue;
        ChannelList activeChannels;
        bool callingPendingFunctors;
        int wakeupFd;
        boost::scoped_ptr<Channel> wakeupChannel;
        MutexLock mutex;
        std::vector<Functor> pendingFunctors;
    };
}

#endif //TINY_MUDUO_EVENTLOOP_H
