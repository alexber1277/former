#pragma once
#include "../gl/glincludes.h"
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

class Gui {
    public:
        Gui(GLFWwindow* window) {
            m_Window = window;
            ImGui::CreateContext();
            ImGui_ImplGlfw_InitForOpenGL(m_Window, true);
            ImGui_ImplOpenGL3_Init("#version 460 core");
        };
        void StartFrame();
        void EndFrame();
        void RightPanel();
        void RenderPanels();

        float value1;
        float value2;
        float value3;

    private:
        GLFWwindow* m_Window;

};