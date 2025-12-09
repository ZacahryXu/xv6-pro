cmake_minimum_required(VERSION 3.24)
project(xv6 C)

set(CMAKE_C_STANDARD 99)

include_directories(.)

add_executable(xv6
        kernel
        mkfs
        user
        kernel/main.c
        user/redirect.c
        user/sleep.c
        user/pingpong.c
        user/find.c
        user/xargs.c
        user/trace.c
)

