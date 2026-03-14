#include "gui.h"

#include <filesystem>

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
    RightPanel();
    if (showAssets) {
        AssetWindow();
    }

}

void Gui::RightPanel() {
    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x-300, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(300, io.DisplaySize.y), ImGuiCond_Always);
    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoSavedSettings;
    ImGui::Begin("right_panel", nullptr, flags);
    if (ImGui::BeginTabBar("right_tabs")){
        if (ImGui::BeginTabItem("GEOMETRY")) {
            NewMesh();
            GeometryList();
            if (app->gizmoMesh) {
                ShowPropertyObject();
                ShowMaterialsObject();
            }
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("MATERIALS")) {
            MaterialList();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    ImGui::End();
}

void Gui::ShowMaterialsObject() {
    if (!app->gizmoMesh) {
        return;
    }
    if (app->gizmoInstance < 0 || (size_t)app->gizmoInstance >= app->gizmoMesh->instances.size()) {
        return;
    }
    ImGui::SeparatorText("Materials");
}

void Gui::ShowPropertyObject() {
    if (!app->gizmoMesh) {
        return;
    }
    if (app->gizmoInstance < 0 || (size_t)app->gizmoInstance >= app->gizmoMesh->instances.size()) {
        return;
    }
    auto& inst = app->gizmoMesh->instances[app->gizmoInstance];
    glm::vec3 pos   = inst.position;
    glm::vec3 rot   = inst.rotation;
    glm::vec3 scale = inst.scale;
    bool changed = false;
    ImGui::SeparatorText("Transform");
    changed |= ImGui::DragFloat3("Position", &pos.x, 0.1f);
    changed |= ImGui::DragFloat3("Rotation", &rot.x, 0.5f);
    changed |= ImGui::DragFloat3("Scale", &scale.x, 0.1f);
    if (ImGui::Button("Reset", ImVec2(-FLT_MIN, 0))) {
        pos = glm::vec3(0.0f);
        rot = glm::vec3(0.0f);
        scale = glm::vec3(1.0f);
        inst.position = pos;
        inst.rotation = rot;
        inst.scale    = scale;
        app->gizmoMesh->UpdateInstance(app->gizmoInstance);
    }
    if (changed) {
        inst.position = pos;
        inst.rotation = rot;
        inst.scale    = scale;
        app->gizmoMesh->UpdateInstance(app->gizmoInstance);
    }
}

void Gui::NewMesh() {
    if (ImGui::Button("Add geometry", ImVec2(-FLT_MIN, 0))) {
        showAssets = true;
    }
    SetHandCursorIfHovered();
}

void Gui::GeometryList() {
    ImGui::BeginChild("SceneList", ImVec2(0,200), false);
    ImGui::Dummy(ImVec2(0.0f, 0.0f));
    auto meshes = app->GetMeshes();
    for (size_t meshIdx = 0; meshIdx < meshes.size(); meshIdx++){
        auto& mesh = meshes[meshIdx];
        for (size_t i = 0; i < mesh.instances.size(); i++){
            auto& inst = mesh.instances[i];
            bool selected = (inst.selected >= 1);
            std::string label = " "+mesh.name + "[" +std::to_string(meshIdx) + "-" +std::to_string(i) + "]";
            if (ImGui::Selectable(label.c_str(), selected)) {
                app->SelectInstanceMesh(mesh.ID, i);
            }
        }
    }
    ImGui::EndChild();
}

void Gui::AssetWindow() {
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 center(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);
    ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(1200, 800), ImGuiCond_Always);
    if (ImGui::Begin("Assets", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse)) {
        if (!ImGui::IsWindowFocused()) { showAssets = false; }
        ImGui::Dummy(ImVec2(0.0f, 10.0f));
        ImGui::BeginChild("AssetGrid", ImVec2(0,0), false);
        float padding = 16.0f;
        float cellWidth  = iconSize + padding;
        float cellHeight = iconSize + 30.0f;
        float panelWidth = ImGui::GetContentRegionAvail().x;
        int columns = (int)(panelWidth / cellWidth);
        if (columns < 1) columns = 1;
        const int maxNameLen = 12;
        int columnIndex = 0;
        for (size_t i = 0; i < Assets.size(); i++) {
            auto& asset = Assets[i];
            ImGui::PushID((int)i);
            ImGui::BeginChild("cell", ImVec2(cellWidth, cellHeight), false);
            float iconOffset = (cellWidth - iconSize) * 0.5f;
            ImGui::SetCursorPosX(iconOffset);
            ImGui::Image((ImTextureID)(intptr_t)asset.thumbnailTex,ImVec2(iconSize, iconSize));
            if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
                app->LoadMeshAsset(asset.path);
                showAssets = false;
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
                float previewSize = 300.0f;
                ImGui::BeginTooltip();
                ImGui::Image((ImTextureID)(intptr_t)asset.thumbnailTex,ImVec2(previewSize, previewSize));
                std::string tooltipName =
                    std::filesystem::path(asset.name).filename().string();
                ImGui::Text("%s", tooltipName.c_str());
                ImGui::EndTooltip();
            }
            std::string displayName =
                std::filesystem::path(asset.name).filename().string();
            if (displayName.length() > maxNameLen)
                displayName = displayName.substr(0, maxNameLen - 3) + "...";
            float textWidth = ImGui::CalcTextSize(displayName.c_str()).x;
            float textOffset = (cellWidth - textWidth) * 0.5f;
            if (textOffset < 4.0f) textOffset = 4.0f;
            ImGui::SetCursorPosX(textOffset);
            ImGui::Text("%s", displayName.c_str());
            ImGui::EndChild();
            ImGui::PopID();
            columnIndex++;
            if (columnIndex < columns)
                ImGui::SameLine();
            else
                columnIndex = 0;
        }
        ImGui::EndChild();
    }
    ImGui::End();
}

void Gui::MaterialList() {
    ImGui::Text("Materials");
}

void Gui::SetHandCursorIfHovered() {
    if (ImGui::IsItemHovered())
        ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
}

void Gui::AssetsList() {
    ImGuiIO& io = ImGui::GetIO();
    float height = 200.0f;
    ImGui::SetNextWindowPos(ImVec2(0.0f, io.DisplaySize.y - height), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, height), ImGuiCond_Always);
    ImGui::Begin("assets", nullptr, baseFlags);
    float padding  = 10.0f;
    float cellSize = iconSize + padding;
    float panelWidth = ImGui::GetContentRegionAvail().x;
    int columns = (int)(panelWidth / cellSize);
    if (columns < 1) columns = 1;
    const int maxNameLen = 8;
    int columnIndex = 0;
    for (size_t i = 0; i < Assets.size(); i++) {
        auto& asset = Assets[i];
        ImGui::PushID((int)i);
        ImGui::BeginGroup();
        ImGui::Image((ImTextureID)(intptr_t)asset.thumbnailTex, ImVec2(iconSize, iconSize));
        if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
            app->LoadMeshAsset(asset.path);
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
            ImVec2 mouse = ImGui::GetMousePos();
            float previewSize = 300.0f;
            ImVec2 tooltipPos(mouse.x - previewSize * 0.5f, mouse.y - previewSize - 40.0f);
            ImGui::SetNextWindowPos(tooltipPos, ImGuiCond_Always);
            ImGui::BeginTooltip();
            ImGui::Image((ImTextureID)(intptr_t)asset.thumbnailTex, ImVec2(previewSize, previewSize));
            std::string tooltipName = std::filesystem::path(asset.name).filename().string();
            ImGui::Text("%s", tooltipName.c_str());
            ImGui::EndTooltip();
        }
        std::string displayName = std::filesystem::path(asset.name).filename().string();
        if (displayName.length() > maxNameLen) {
            displayName = displayName.substr(0, maxNameLen - 3) + "...";
        }
        ImGui::TextWrapped("%s", displayName.c_str());
        ImGui::EndGroup();
        ImGui::PopID();
        columnIndex++;
        if (columnIndex < columns)
            ImGui::SameLine();
        if (columnIndex >= columns)
            columnIndex = 0;
    }
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

