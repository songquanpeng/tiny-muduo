//
// Created by song on 6/27/2022.
//

#ifndef TINY_MUDUO_CHANNEL_H
#define TINY_MUDUO_CHANNEL_H

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>

namespace muduo {
    class EventLoop;

    class Channel : boost::noncopyable {
    public:
        typedef boost::function<void()> EventCallback;

        Channel(EventLoop *loop, int fd);

        // Will be called by EventLoop::loop()
        // Call different user callback according to the value of revents
        void handleEvent();

        void setReadCallback(const EventCallback &cb) {
            readCallback = cb;
        }

        void setWriteCallback(const EventCallback &cb) {
            writeCallback = cb;
        }

        void setErrorCallback(const EventCallback &cb) {
            errorCallback = cb;
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

        EventCallback readCallback;
        EventCallback writeCallback;
        EventCallback errorCallback;


        EventLoop *ownerLoop;
        const int fd;
        int events;  /* Types of events poller cares about.  */
        int revents;  /* Types of events that actually occurred.  */
        int index; // Used by Poller
    };
}


#endif //TINY_MUDUO_CHANNEL_H
