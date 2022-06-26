#include <iostream>
#include "thread/Thread.h"
#include "EventLoop.h"


void threadFunc() {
    printf("threadFunc(): pid = %d, tid = %d\n", getpid(), muduo::CurrentThread::tid());
    muduo::EventLoop loop;
    loop.loop();
}

int main() {
    printf("main(): pid = %d, tid = %d\n", getpid(), muduo::CurrentThread::tid());
    muduo::EventLoop loop;
    muduo::Thread thread(threadFunc);
    thread.start();
    loop.loop();
    return 0;
}
