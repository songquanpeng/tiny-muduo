cmake_minimum_required(VERSION 3.0)
project(tiny_muduo)

set(CMAKE_CXX_STANDARD 11)

include_directories(muduo)

set(SOURCES
        muduo/EventLoop.cpp
        muduo/logging/Logging.cc
        muduo/logging/LogStream.cc
        muduo/thread/Thread.cc
        muduo/datetime/Timestamp.cc
        muduo/Channel.cpp
        muduo/Poller.cpp
        muduo/TimerQueue.cpp
        muduo/Timer.cpp
        muduo/Timer.cpp
        muduo/EventLoopThread.cpp
        muduo/Acceptor.cpp
        muduo/Socket.cc
        muduo/SocketsOps.cc
        muduo/InetAddress.cc
        muduo/TcpServer.cpp
        muduo/TcpConnection.cpp
        muduo/Buffer.cpp
        muduo/EventLoopThreadPool.cpp)

add_executable(test1 test1.cpp ${SOURCES})
target_link_libraries(test1 pthread)
add_executable(test2 test2.cpp ${SOURCES})
target_link_libraries(test2 pthread)
add_executable(test3 test3.cpp ${SOURCES})
target_link_libraries(test3 pthread)
add_executable(test4 test4.cpp ${SOURCES})
target_link_libraries(test4 pthread)
add_executable(test5 test5.cpp ${SOURCES})
target_link_libraries(test5 pthread)
add_executable(test6 test6.cpp ${SOURCES})
target_link_libraries(test6 pthread)
add_executable(test7 test7.cpp ${SOURCES})
target_link_libraries(test7 pthread)
add_executable(test8 test8.cpp ${SOURCES})
target_link_libraries(test8 pthread)
add_executable(test9 test9.cpp ${SOURCES})
target_link_libraries(test9 pthread)
add_executable(test10 test10.cpp ${SOURCES})
target_link_libraries(test10 pthread)
add_executable(test11 test11.cpp ${SOURCES})
target_link_libraries(test11 pthread)