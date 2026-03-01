#include "gui.h"

#include "../src/app.h"

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
    Mesh& baseMesh = app->GetMeshByIndex(0);
    ImGui::Text("Adjust three float values:");
    ImGui::SliderFloat4("Color", &baseMesh.MatAlbedoVal[0], 0.0f, 1.0f);
    ImGui::SliderFloat("Roughness", &baseMesh.MatRoughnesVal, 0.0f, 1.0f);
    ImGui::SliderFloat("Metalic", &baseMesh.MatMetalicVal, 0.0f, 1.0f);
    ImGui::End();
}

