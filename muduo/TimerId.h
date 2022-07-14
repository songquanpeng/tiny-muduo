//
// Created by song on 6/28/2022.
//

#ifndef TINY_MUDUO_TIMERID_H
#define TINY_MUDUO_TIMERID_H

#include "datetime/copyable.h"

namespace muduo {
    class Timer;

    class TimerId : public muduo::copyable {
    public:
        TimerId(Timer *timer = nullptr, int64_t seq = 0) : timer(timer), sequence(seq) {
        }

        friend class TimerQueue;

    private:
        Timer *timer;
        int64_t sequence;
    };
}

#endif //TINY_MUDUO_TIMERID_H
