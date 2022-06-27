//
// Created by song on 6/27/2022.
//

#include "Poller.h"
#include "Channel.h"
#include "logging/Logging.h"

#include <assert.h>
#include <poll.h>

using namespace muduo;

Poller::Poller(EventLoop *loop) : ownerEventLoop(loop) {

}

Poller::~Poller() {

}

Timestamp Poller::poll(int timeoutMs, Poller::ChannelList *activateChannels) {
    int numEvents = ::poll(pollFdList.data(), pollFdList.size(), timeoutMs);
    Timestamp now(Timestamp::now());
    if (numEvents > 0) {
        LOG_TRACE << numEvents << " events happened";
        fillActivateChannels(numEvents, activateChannels);
    } else if (numEvents == 0) {
        LOG_TRACE << "nothing happened";
    } else {
        LOG_SYSERR << "Poller::poll()";
    }
    return now;
}


void Poller::fillActivateChannels(int numEvents, Poller::ChannelList *activateChannels) const {
    for (auto iter = pollFdList.begin(); iter != pollFdList.end() && numEvents > 0; ++iter) {
        if (iter->revents > 0) {
            --numEvents;
            auto findIter = channelMap.find(iter->fd);
            assert(findIter != channelMap.end());
            Channel *channel = findIter->second;
            assert(channel->getFd() == iter->fd);
            channel->setRevents(iter->revents);
            activateChannels->push_back(channel);
        }
    }
}

void Poller::updateChannel(Channel *channel) {
    assertInLoopThread();
    LOG_TRACE << "fd = " << channel->getFd() << " events = " << channel->getEvents();
    if (channel->getIndex() < 0) {
        // Add a new channel
        assert(channelMap.find(channel->getFd()) == channelMap.end());
        struct pollfd pfd{};
        pfd.fd = channel->getFd();
        pfd.events = (short)(channel->getEvents());
        pfd.revents = 0;
        pollFdList.push_back(pfd);
        int idx = (int)(pollFdList.size()) - 1;
        channel->setIndex(idx);
        channelMap[pfd.fd] = channel;
    } else {
        // Update existing channel
        assert(channelMap.find(channel->getFd()) != channelMap.end());
        assert(channelMap[channel->getFd()] == channel);
        int idx = channel->getIndex();
        assert(0 <= idx && idx < (int)(pollFdList.size()));
        auto& pfd = pollFdList[idx];  // Notice this is a reference
        assert(pfd.fd == channel->getFd() || pfd.fd == -1);
        pfd.events == (short)(channel->getEvents());
        pfd.revents = 0;
        if (channel->isNoneEvent()) {
            // Ignore this pollfd
            pfd.fd = -1;
        }
    }
}


