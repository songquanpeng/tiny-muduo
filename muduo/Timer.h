//
// Created by song on 6/28/2022.
//

#ifndef TINY_MUDUO_TIMER_H
#define TINY_MUDUO_TIMER_H

#include <boost/noncopyable.hpp>

#include "datetime/Timestamp.h"
#include "Callbacks.h"

namespace muduo {
    class Timer : boost::noncopyable {
    public:
        Timer(const TimerCallback &cb, Timestamp when, double interval)
                : callback(cb), expirationTimestamp(when), interval(interval), repeat(interval > 0.0) {
        }

        void run() const {
            callback();
        }

        Timestamp getExpirationTimestamp() const {
            return expirationTimestamp;
        }

        bool isRepeat() const {
            return repeat;
        }

        void restart(Timestamp now);

    private:
        const TimerCallback callback;
        Timestamp expirationTimestamp;
        const double interval;
        const bool repeat;
    };
}

#endif //TINY_MUDUO_TIMER_H
