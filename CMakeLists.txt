cmake_minimum_required(VERSION 3.16)
project(ARMv8Again)

set(CMAKE_CXX_STANDARD 20)

include_directories(include)

add_executable(ARMv8Again source/main.cpp source/cpu/core.cpp source/cpu/instruction_handlers.cpp)