//
// Created by song on 6/27/2022.
//

#ifndef TINY_MUDUO_CHANNEL_H
#define TINY_MUDUO_CHANNEL_H

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>

#include <datetime/Timestamp.h>

namespace muduo {
    class EventLoop;

    class Channel : boost::noncopyable {
    public:
        typedef boost::function<void()> EventCallback;
        typedef boost::function<void(Timestamp)> ReadEventCallback;

        Channel(EventLoop *loop, int fd);
        ~Channel();

        // Will be called by EventLoop::loop()
        // Call different user callback according to the value of revents
        void handleEvent(Timestamp receiveTimestamp);

        void setReadCallback(const ReadEventCallback &cb) {
            readCallback = cb;
        }

        void setWriteCallback(const EventCallback &cb) {
            writeCallback = cb;
        }

        void setErrorCallback(const EventCallback &cb) {
            errorCallback = cb;
        }

        void setCloseCallback(const EventCallback &cb) {
            closeCallback = cb;
        }

        int getFd() const {
            return fd;
        }

        int getEvents() const {
            return events;
        }

        void setRevents(int revents) {
            this->revents = revents;
        }

        bool isNoneEvent() const {
            return events == kNoneEvent;
        }

        void enableReading() {
            events |= kReadEvent;
            update();
        }

        void enableWriting() {
            events |= kWriteEvent;
            update();
        }

        void disableWriting() {
            events &= ~kWriteEvent;
            update();
        }

        void disableAll() {
            events = kNoneEvent;
            update();
        }

        bool isWriting() const {
            return events & kWriteEvent;
        }

        void setIndex(int idx) {
            index = idx;
        }

        int getIndex() const {
            return index;
        }

        EventLoop* getOwnerLoop() {
            return ownerLoop;
        }

    private:
        // This will call EventLoop::updateChannel()
        void update();

        static const int kNoneEvent;
        static const int kReadEvent;
        static const int kWriteEvent;

        ReadEventCallback readCallback;
        EventCallback writeCallback;
        EventCallback errorCallback;
        EventCallback closeCallback;

        bool eventHandling;

        EventLoop *ownerLoop;
        const int fd;
        int events;  /* Types of events poller cares about.  */
        int revents;  /* Types of events that actually occurred.  */
        int index; // Used by Poller
    };
}


#endif //TINY_MUDUO_CHANNEL_H
