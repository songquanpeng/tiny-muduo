//
// Created by song on 6/27/2022.
//

#include "Poller.h"
#include "Channel.h"
#include "logging/Logging.h"

#include <cassert>
#include <poll.h>

using namespace muduo;

Poller::Poller(EventLoop *loop) : ownerEventLoop(loop) {

}

Poller::~Poller() = default;

Timestamp Poller::poll(int timeoutMs, Poller::ChannelList *activeChannels) {
    int numEvents = ::poll(pollFdList.data(), pollFdList.size(), timeoutMs);
    Timestamp now(Timestamp::now());
    if (numEvents > 0) {
        LOG_TRACE << numEvents << " events happened";
        fillActiveChannels(numEvents, activeChannels);
    } else if (numEvents == 0) {
        LOG_TRACE << "nothing happened";
    } else {
        LOG_SYSERR << "Poller::poll()";
    }
    return now;
}


void Poller::fillActiveChannels(int numEvents, Poller::ChannelList *activeChannels) const {
    for (auto iter = pollFdList.begin(); iter != pollFdList.end() && numEvents > 0; ++iter) {
        if (iter->revents > 0) {  // Which means this pfd is active
            --numEvents;
            auto targetIter = channelMap.find(iter->fd);
            assert(targetIter != channelMap.end());
            Channel *channel = targetIter->second;
            assert(channel->getFd() == iter->fd);
            channel->setRevents(iter->revents);
            activeChannels->push_back(channel);
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
        pfd.events = (short)(channel->getEvents());
        pfd.revents = 0;
        if (channel->isNoneEvent()) {
            // Ignore this pollfd
            pfd.fd = -1;
        }
    }
}


