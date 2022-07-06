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
const int Channel::kWriteEvent = POLLOUT;

Channel::Channel(EventLoop *loop, int fd) : ownerLoop(loop), fd(fd), events(0), revents(0), index(-1), eventHandling(
        false) {

}

Channel::~Channel() {
    assert(!eventHandling);
}

void Channel::update() {
    ownerLoop->updateChannel(this);
}

void Channel::handleEvent(Timestamp receiveTimestamp) {
    eventHandling = true;
    if (revents & POLLNVAL) {
        LOG_WARN << "Channel::handleEvent() POLLNVAL";
    }
    if ((revents & POLLHUP) && !(revents & POLLIN)) {
        LOG_WARN << "Channel::handle_event() POLLHUP";
        if (closeCallback) closeCallback();
    }
    if (revents & (POLLERR | POLLNVAL)) {
        if (errorCallback) errorCallback();
    }

    if (revents & (POLLIN | POLLPRI | POLLRDHUP)) {
        if (readCallback) readCallback(receiveTimestamp);
    }

    if (revents & POLLOUT) {
        if (writeCallback) writeCallback();
    }
    eventHandling = false;
}


