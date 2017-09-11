cmake_minimum_required(VERSION 3.8)
project(AE)

set(CMAKE_C_STANDARD 99)

set(SOURCE_FILES main.c HashandStoreData/SHA1Hashing.c headers/SHA1Hashing.h)
add_executable(AE ${SOURCE_FILES})