//
// Created by song on 6/27/2022.
//

#include "Channel.h"
#include "EventLoop.h"
#include "logging/Logging.h"
#include <sstream>
#include <poll.h>

using namespace muduo;

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = POLLIN | POLLPRI;

Channel::Channel(EventLoop *loop, int fd) : ownerLoop(loop), fd(fd), events(0), revents(0), index(-1) {

}

void Channel::update() {
    ownerLoop->updateChannel(this);
}

void Channel::handleEvent() {
    if (revents & POLLNVAL) {
        LOG_WARN << "Channel::handleEvent() POLLNVAL";
    }
    if (revents & (POLLERR | POLLNVAL)) {
        if (errorCallback) errorCallback();
    }

    if (revents & (POLLIN | POLLPRI | POLLRDHUP)) {
        if (readCallback) readCallback();
    }

    if (revents & POLLOUT) {
        if (writeCallback) writeCallback();
    }
}