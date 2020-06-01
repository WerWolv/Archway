cmake_minimum_required(VERSION 3.16)
project(ARMv8)

set(CMAKE_CXX_STANDARD 20)

add_executable(ARMv8 source/main.cpp source/core.cpp source/logger.cpp source/address_space.cpp source/board.cpp source/cpu.cpp source/devices/memory.cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fconcepts")

include_directories(ARMv8 include)