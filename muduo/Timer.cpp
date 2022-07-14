//
// Created by song on 6/28/2022.
//

#include "Timer.h"

using namespace muduo;

AtomicInt64 Timer::s_numCreated;

void Timer::restart(Timestamp now) {
    if (repeat) {
        expirationTimestamp = addTime(now, interval);
    } else {
        expirationTimestamp = Timestamp::invalid();
    }
}
