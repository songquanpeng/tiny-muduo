//
// Created by song on 6/28/2022.
//

#ifndef TINY_MUDUO_CALLBACKS_H
#define TINY_MUDUO_CALLBACKS_H

#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>

#include "datetime/Timestamp.h"

namespace muduo {
    typedef boost::function<void()> TimerCallback;
}

#endif //TINY_MUDUO_CALLBACKS_H
