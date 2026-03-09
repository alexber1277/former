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
    AssetsList();
    Explorer();
}

void Gui::AssetsList() {
    ImGuiIO& io = ImGui::GetIO();
    float height = 200.0f;
    ImGui::SetNextWindowPos(ImVec2(0.0f, io.DisplaySize.y - height),ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, height),ImGuiCond_Always);
    ImGui::Begin("assets", nullptr, baseFlags);
    float padding  = 10.0f;
    float cellSize = iconSize + padding;
    float panelWidth = ImGui::GetContentRegionAvail().x;
    int columns = (int)(panelWidth / cellSize);
    if (columns < 1) columns = 1;
    ImGui::Columns(columns, 0, false);
    for (size_t i = 0; i < Assets.size(); i++) {
        auto& asset = Assets[i];
        ImGui::PushID((int)i);
        ImGui::Image((ImTextureID)(intptr_t)asset.thumbnailTex,ImVec2(iconSize, iconSize));
        if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
            app->LoadMeshAsset(Assets[i].path);
        }
        if (ImGui::IsItemHovered()){
            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
            ImGui::BeginTooltip();
            float previewSize = 300.0f;
            ImGui::Image((ImTextureID)(intptr_t)asset.thumbnailTex,ImVec2(previewSize, previewSize));
            ImGui::Text("%s", asset.name.c_str());
            ImGui::EndTooltip();
        }
        ImGui::TextWrapped("%s", asset.name.c_str());
        ImGui::NextColumn();
        ImGui::PopID();
    }
    ImGui::Columns(1);
    ImGui::End();
}

void Gui::Explorer() {
    float width = 250.0f;
    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - width, 0));
    ImGui::SetNextWindowSize(ImVec2(width, io.DisplaySize.y));
    ImGui::Begin("explorer", nullptr, baseFlags);

    ImGui::Text("EXPLORER");
    ImGui::Separator();

    // list meshes =============
    auto& meshes = app->GetMeshes();
    for (size_t m = 0; m < meshes.size(); m++) {
        auto& mesh = meshes[m];
        std::string meshLabel = mesh.name +  " [" + std::to_string(mesh.instances.size()) + "]["+std::to_string(m)+"]";
        if (ImGui::TreeNode(meshLabel.c_str())) {
            for (size_t i = 0; i < mesh.instances.size(); i++) {
                auto& inst = mesh.instances[i];
                std::string instLabel = "inst [" + std::to_string(m) + "][" + std::to_string(i)+"]";
                bool selected = inst.selected >= 1.0f;
                if (ImGui::Selectable(instLabel.c_str(), selected)) {
                    inst.selected = selected ? 0.0f : 1.0f;
                    mesh.UpdateInstance(i);
                    app->SelectedMesh = mesh.ID+1;
                    app->SelectedInstance = i+1;
                }
            }
            if (ImGui::Selectable("+ add inst", 0.0f)) {
                glm::vec3 p = glm::vec3(0.0f);
                glm::vec3 r = glm::vec3(0.0f);
                glm::vec3 s = glm::vec3(1.0f);
                mesh.AddInstance(p,r,s);
                mesh.UpdateInstances();
            }
            ImGui::TreePop();
        }
    }
    ImGui::End();
}

