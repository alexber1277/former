#include "gui.h"

void Gui::StartFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void Gui::EndFrame() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Gui::RenderPanels() {
    StartFrame();

    // Right
    RightPanel();

    EndFrame();
}

void Gui::RightPanel() {
    ImGui::Begin("RTNV Controls");

    ImGui::Text("Adjust three float values:");
    ImGui::SliderFloat("Value 1", &value1, 0.0f, 10.0f);
    ImGui::SliderFloat("Value 2", &value2, -5.0f, 5.0f);
    ImGui::SliderFloat("Value 3", &value3, 0.0f, 1.0f);

    ImGui::End();
}

