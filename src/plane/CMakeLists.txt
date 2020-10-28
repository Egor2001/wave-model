cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

set(CMAKE_THREAD_PREFER_PTHREAD ON)
find_package(Threads REQUIRED)

include_directories(PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(plain main.cpp)
target_link_libraries(plain Threads::Threads)
