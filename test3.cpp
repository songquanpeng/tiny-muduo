#include <cstdio>
#include "Channel.h"
#include <sys/timerfd.h>
#include "EventLoop.h"

muduo::EventLoop *globalLoop;

void timeout() {
    printf("Timeout!");
    globalLoop->quit();
}

int main() {
    muduo::EventLoop loop;
    globalLoop = &loop;
    int timerfd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    muduo::Channel channel(&loop, timerfd);
    channel.setReadCallback(timeout);
    channel.enableReading();

    struct itimerspec howLong{};
    bzero(&howLong, sizeof howLong);
    howLong.it_value.tv_sec = 5;
    timerfd_settime(timerfd, 0, &howLong, nullptr);
    loop.loop();
    return 0;
}