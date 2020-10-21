cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

project(WaveModel VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Ofast")

    add_compile_options(-Wall -Wextra -pedantic)

elseif(CMAKE_CXX_COMPILER_ID MATCHES "Intel")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g -debug all")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Ofast")

    add_compile_options(-Wall -w3 
        -Warray-bounds -Wchar-subscripts -Wcomment -Wenum-compare 
        -Wformat -Wuninitialized -Wmaybe-uninitialized -Wmain -Wnarrowing 
        -Wnonnull -Wparentheses -Wpointer-sign -Wreorder -Wreturn-type 
        -Wsign-compare -Wsequence-point -Wtrigraphs -Wunused-function 
        -Wunused-but-set-variable -Wunused-variable -Wwrite-strings
        )

elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR 
       CMAKE_CXX_COMPILER_ID MATCHES "AppleClang")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Ofast")

    add_compile_options(-Weverything -pedantic)

else()
    message(FATAL_ERROR 
        "${CMAKE_CXX_COMPILER_ID} is not currently supported."
            "Use on of these instead:"
            "GNU gcc"
            "Intel icc"
            "LLVM Clang"
            "Apple Clang"
            )

endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

include_directories(PUBLIC src/)

add_subdirectory(src/plane)
