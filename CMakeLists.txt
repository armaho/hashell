cmake_minimum_required(VERSION 3.29)
project(hashell C)

set(CMAKE_C_STANDARD 11)

include_directories(src)

add_executable(hashell
        src/main.c
        src/parser.c
        src/parser.h
        src/path.c
        src/path.h)
