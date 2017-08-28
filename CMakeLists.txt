cmake_minimum_required(VERSION 3.8)
project(AE)

set(CMAKE_C_STANDARD 99)

set(SOURCE_FILES main.c sha1.c sha1.h)
add_executable(AE ${SOURCE_FILES})