cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

set(CMAKE_THREAD_PREFER_PTHREAD ON)
find_package(Threads REQUIRED)
find_package(OpenMP REQUIRED)

if (OpenMP_CXX_FOUND)
    add_compile_options(${OpenMP_CXX_FLAGS})

    include_directories(PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
    include_directories(PUBLIC ${OpenMP_CXX_INCLUDE_DIRS})
endif()

add_executable(plain main.cpp)
target_link_libraries(plain Threads::Threads OpenMP::OpenMP_CXX)
