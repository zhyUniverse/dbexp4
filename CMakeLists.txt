cmake_minimum_required(VERSION 3.14)
project(dbexp4 C)

set(CMAKE_C_STANDARD 11)

add_executable(dbexp4 main.c extmem.h extmem.c utils.c utils.h task.c task.h)