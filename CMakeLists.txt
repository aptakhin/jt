cmake_minimum_required(VERSION 3.7)
project(jt)

set(CMAKE_CXX_STANDARD 14)

#set(CMAKE_CXX_COMPILER "/usr/bin/clang++")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -Wextra -pedantic -static -stdlib=libc++")

set(SOURCE_FILES
    src/gtest/gtest-all.cc
    src/gtest/gtest.h
    src/dev.cpp
    src/dev.h
    src/test.cpp
)

include_directories(src)

add_executable(jt ${SOURCE_FILES})