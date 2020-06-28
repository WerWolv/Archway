cmake_minimum_required(VERSION 3.16)
project(ARMv8)

set(CMAKE_CXX_STANDARD 20)

add_executable(ARMv8
        libs/glad/source/glad.c
        libs/imgui/source/imgui.cpp
        libs/imgui/source/imgui_demo.cpp
        libs/imgui/source/imgui_draw.cpp
        libs/imgui/source/imgui_widgets.cpp
        libs/imgui/source/imgui_impl_glfw.cpp
        libs/imgui/source/imgui_impl_opengl3.cpp

        source/main.cpp
        source/core.cpp
        source/logger.cpp
        source/address_space.cpp
        source/board.cpp
        source/cpu.cpp
        source/devices/memory.cpp
        source/ui/window.cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fconcepts")

include_directories(ARMv8 include libs/glad/include libs/glfw/include libs/imgui/include)

target_link_directories(ARMv8 PUBLIC libs/glfw/lib)
target_link_libraries(ARMv8 PUBLIC glfw3)