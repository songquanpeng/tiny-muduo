cmake_minimum_required(VERSION 3.0)
project(tiny_muduo)

set(CMAKE_CXX_STANDARD 11)

include_directories(muduo)

add_executable(server
        main.cpp
        muduo/EventLoop.cpp
        muduo/logging/Logging.cc
        muduo/logging/LogStream.cc
        muduo/thread/Thread.cc
        muduo/datetime/Timestamp.cc
)

target_link_libraries(server pthread)