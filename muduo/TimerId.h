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
        explicit TimerId(Timer *timer) : timer(timer) {
        }

    private:
        Timer *timer;
    };
}

#endif //TINY_MUDUO_TIMERID_H
