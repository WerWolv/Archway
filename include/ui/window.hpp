#pragma once

#include "board.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace arm::ui {

    class Window {
    public:
        Window(Board &board);
        ~Window();

        bool update();
    private:
        Board &m_board;

        GLFWwindow *m_window;

        void drawDebuggerWindow();
        void drawRegisterWindow(u8 coreId);
        void drawMemoryWindow();
    };

}