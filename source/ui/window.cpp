#include "ui/window.hpp"

#include "logger.hpp"

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_glfw.h"

namespace arm::ui {

    Window::Window(Board &board) : m_board(board) {


        if (!glfwInit())
            Logger::fatal("GLFW init failed!");

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
        this->m_window = glfwCreateWindow(640, 480, "My Title", nullptr, nullptr);
        if (this->m_window == nullptr)
            Logger::fatal("Window creation failed!");

        glfwMakeContextCurrent(this->m_window);
        glfwSwapInterval(1);

        if (!gladLoadGL())
            Logger::fatal("GLAD init failed!");

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;

        ImGui::StyleColorsDark();

        ImGui_ImplGlfw_InitForOpenGL(this->m_window, true);
        ImGui_ImplOpenGL3_Init("#version 130");
    }

    Window::~Window() {
        glfwDestroyWindow(this->m_window);
        glfwTerminate();
    }

    bool Window::update() {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        this->drawDebuggerWindow();

        this->drawRegisterWindow(0);

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(this->m_window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(this->m_window);

        return !glfwWindowShouldClose(this->m_window);
    }

    void Window::drawDebuggerWindow() {
        ImGui::Begin("Control");

        if (ImGui::Button("Run")) {
            for (u8 coreId = 0; coreId < this->m_board.CPU.getCoreCount(); coreId++)
                this->m_board.CPU.getCore(coreId).exitDebugMode();
        }


        ImGui::SameLine();
        if (ImGui::Button("Break")) {
            for (u8 coreId = 0; coreId < this->m_board.CPU.getCoreCount(); coreId++) {
                this->m_board.CPU.getCore(coreId).breakCore();
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Reset")) {
            for (u8 coreId = 0; coreId < this->m_board.CPU.getCoreCount(); coreId++) {
                this->m_board.CPU.getCore(coreId).reset();
            }
        }

        ImGui::NewLine();
        if (ImGui::Button("Step Instruction")) {
            for (u8 coreId = 0; coreId < this->m_board.CPU.getCoreCount(); coreId++)
                this->m_board.CPU.getCore(coreId).singleStep();
        }

        ImGui::End();
    }

    void Window::drawRegisterWindow(u8 coreId) {
        if (!this->m_board.CPU.getCore(coreId).m_broken)
            return;

        ImGui::Begin("Debug Info");

        if (auto currInst = this->m_board.CPU.getCore(0).m_currInstruction; currInst.name == nullptr)
            ImGui::Text("Current Instruction: %s", "NONE");
        else
            ImGui::Text("Current Instruction %s", currInst.name);

        ImGui::NewLine();

        ImGui::Text("PC  : 0x%016llx", this->m_board.CPU.getCore(0).PC.X);
        ImGui::Text("SP  : 0x%016llx", this->m_board.CPU.getCore(0).GPR[32].X);
        ImGui::Text("LR  : 0x%016llx", this->m_board.CPU.getCore(0).GPR[30].X);

        ImGui::NewLine();

        for (u8 i = 0; i < 30; i++)
            ImGui::Text("W%02d : 0x%016llx", i, this->m_board.CPU.getCore(0).GPR[i].X);

        ImGui::End();
    }

    void Window::drawMemoryWindow() {

    }

}