cmake_minimum_required(VERSION 3.31)
project(hashtest)

set(CMAKE_CXX_STANDARD 23)

add_executable(hashtest main.cpp sha25602/sha256.c
        verVersion.cpp)
#target_link_libraries(hashtest public stdc++exp)

