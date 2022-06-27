//
// Created by song on 6/27/2022.
//

#ifndef TINY_MUDUO_POLLER_H
#define TINY_MUDUO_POLLER_H

#include <map>
#include <vector>

#include "datetime/Timestamp.h"
#include "EventLoop.h"

struct pollfd;

//struct pollfd {
//    int fd;			/* File descriptor to poll.  */
//    short int events;		/* Types of events poller cares about.  */
//    short int revents;		/* Types of events that actually occurred.  */
//};

namespace muduo {
    class Channel;

    class Poller : boost::noncopyable {
    public:
        typedef std::vector<Channel*> ChannelList;
        Poller(EventLoop* loop);
        ~Poller();

        /// Polls the I/O events.
        /// Must be called in the loop thread.
        Timestamp poll(int timeoutMs, ChannelList* activateChannels);

        /// Changes the interested I/O events.
        /// Must be called in the loop thread.
        void updateChannel(Channel* channel);

        void assertInLoopThread() {
            ownerEventLoop->assertInLoopThread();
        }

    private:
        void fillActivateChannels(int numEvents, ChannelList* activateChannels) const;

        typedef std::vector<struct pollfd> PollFdList;
        typedef std::map<int, Channel*> ChannelMap;
        EventLoop* ownerEventLoop;
        PollFdList pollFdList;
        ChannelMap channelMap;
    };
}


#endif //TINY_MUDUO_POLLER_H
