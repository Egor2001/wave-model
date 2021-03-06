cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

project(WaveModel VERSION 2.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Ofast")

    add_compile_options(-mavx -Wall -Wextra -pedantic)

elseif(CMAKE_CXX_COMPILER_ID MATCHES "Intel")
    message(AUTHOR_WARNING
            "Intel compiler is temporary generating invalid output 
            which is different from GNU and Clang results."
            "Intel compiler support work is in progress."
            )

    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g -debug all")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Ofast")

    add_compile_options(-Wall -w3 
                        -Warray-bounds 
                        -Wchar-subscripts 
                        -Wcomment 
                        -Wenum-compare 
                        -Wformat 
                        -Wuninitialized 
                        -Wmaybe-uninitialized 
                        -Wmain -Wnarrowing 
                        -Wnonnull -Wparentheses 
                        -Wpointer-sign 
                        -Wreorder 
                        -Wreturn-type 
                        -Wsign-compare 
                        -Wsequence-point 
                        -Wtrigraphs 
                        -Wunused-function 
                        -Wunused-but-set-variable 
                        -Wunused-variable 
                        -Wwrite-strings
                        )

elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR 
       CMAKE_CXX_COMPILER_ID MATCHES "AppleClang")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Ofast")

    # TODO: to fix all disabled warnings
    add_compile_options(-mavx -Weverything -pedantic
                        -Wno-sign-conversion
                        -Wno-global-constructors
                        -Wno-exit-time-destructors
                        -Wno-c++98-compat-pedantic
                        -Wno-padded
                        -Wno-weak-vtables
                        -Wno-documentation
                        )

else()
    message(FATAL_ERROR 
            "${CMAKE_CXX_COMPILER_ID} is not currently supported."
            "Use on of these instead:"
            "GNU gcc"
            "[Intel icc] - CURRENTLY UNAVAILABLE TO USE, FIX IS IN PROCESS"
            "LLVM Clang"
            "Apple Clang"
            )

endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

include_directories(PUBLIC src/)

add_subdirectory(docs)
add_subdirectory(src/plane)
