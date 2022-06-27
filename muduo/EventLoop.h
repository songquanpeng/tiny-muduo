//
// Created by song on 6/25/2022.
//

#ifndef TINY_MUDUO_EVENTLOOP_H
#define TINY_MUDUO_EVENTLOOP_H


#include "thread/Thread.h"
#include <boost/scoped_ptr.hpp>
#include <vector>

namespace muduo {
    class Channel;

    class Poller;

    class EventLoop : boost::noncopyable {
    public:
        EventLoop();

        ~EventLoop();

        void loop();

        void quit();

        void updateChannel(Channel *channel);

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

        typedef std::vector<Channel *> ChannelList;

        bool isLooping;
        bool isQuited;
        const pid_t threadId;
        boost::scoped_ptr<Poller> poller;
        ChannelList activateChannels;
    };
}

#endif //TINY_MUDUO_EVENTLOOP_H
