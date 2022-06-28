//
// Created by song on 6/28/2022.
//

#include "Timer.h"

using namespace muduo;

void Timer::restart(Timestamp now) {
    if (repeat) {
        expirationTimestamp = addTime(now, interval);
    } else {
        expirationTimestamp = Timestamp::invalid();
    }
}
