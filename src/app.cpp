#include <GL/glew.h>
#include "app.h"
#include <fstream>
#include <iostream>
#include <ostream>
#include <sstream>

App::App(std::string name) {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 8);
    glfwWindowHint(GLFW_DEPTH_BITS, 32);
    glfwWindowHint(GLFW_STENCIL_BITS, 8);
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

    window = glfwCreateWindow(800, 600, name.c_str(), nullptr, nullptr);
    if (!window) {
        Log("Failed to create GLFW window");
        glfwTerminate();
        return;
    }

    glfwMakeContextCurrent(window);
    glewExperimental = GL_TRUE;
    glfwSetFramebufferSizeCallback(window, App::Resize);
    glfwSwapInterval(0);
    glfwMaximizeWindow(window);
    glfwGetFramebufferSize(window,&ScreenWidth,&ScreenHeight);

    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::string errorStr(reinterpret_cast<const char*>(glewGetErrorString(err)));
        Log("Failed to initialize GLEW: " + errorStr);
        return;
    }

    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);

    ScreenWidth = mode->width;
    ScreenHeight = mode->height;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_MULTISAMPLE);
    glDepthFunc(GL_LESS);

    camera = new Camera();
    camera->CamCalculated(ScreenWidth, ScreenHeight);
    glViewport(0, 0, ScreenWidth, ScreenHeight);

    input = new Input(window);
    gui = new Gui(window);
    shader = new Shader();

    BaseCallback();

    std::cout << "========================================================" << std::endl;
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLFW Version: " << glfwGetVersionString() << std::endl;
    std::cout << "========================================================" << std::endl;

}

App::~App() {
    glfwDestroyWindow(window);
    glfwTerminate();
}

void App::Resize(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void App::Log(const std::string& msg) {
    std::cout << msg << std::endl;
}

void App::LogErr(const std::string& msg) {
    throw std::runtime_error(msg);
}

void App::BaseCallback() {
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window,
        [](GLFWwindow* window, int width, int height){
            glViewport(0, 0, width, height);
            App* app = static_cast<App*>(glfwGetWindowUserPointer(window));
            if (app) {
                app->ScreenWidth = width;
                app->ScreenHeight = height;
                app->camera->CamCalculated(width, height);
            }
        }
    );
    glfwSetKeyCallback(window,
        [](GLFWwindow* window, int key, int scancode, int action, int mods) {
            (void)scancode;(void)mods;
            App* app = static_cast<App*>(glfwGetWindowUserPointer(window));
            if (!app) return;
            if (key == GLFW_KEY_G && action == GLFW_PRESS) {
                app->showGrid = !app->showGrid;
            }
            if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
                glfwSetWindowShouldClose(window, true);
            }
        }
    );
    glfwSetScrollCallback(window, [](GLFWwindow* window, double xoffset, double yoffset) {
            App* app = static_cast<App*>(glfwGetWindowUserPointer(window));
            if (app) {
                app->input->SetScroll(yoffset);
            }
        }
    );
}

std::string App::LoadFile(std::string path) {
    std::ifstream file(path, std::ios::in);
    if (!file.is_open()) {
        Log(std::string("Failed to open file: ") + path);
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();

}

bool App::LoadShaderItem(const std::string& name, GLuint& program) {
    std::string basePath = "shaders/";
    std::string vertPath = basePath + name + "/vertex.vs";
    std::string fragPath = basePath + name + "/fragment.fs";
    std::string vertCode = LoadFile(vertPath);
    std::string fragCode = LoadFile(fragPath);
    if (vertCode.empty() || fragCode.empty()) {
        Log("Failed to load shader files for: " + name);
        return false;
    }
    GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
    const char* vertSrc = vertCode.c_str();
    glShaderSource(vertShader, 1, &vertSrc, nullptr);
    glCompileShader(vertShader);
    GLint success;
    glGetShaderiv(vertShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(vertShader, 512, nullptr, infoLog);
        Log("Vertex shader compilation failed for " + name + ":");
        Log(infoLog);
        return false;
    }
    GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    const char* fragSrc = fragCode.c_str();
    glShaderSource(fragShader, 1, &fragSrc, nullptr);
    glCompileShader(fragShader);
    glGetShaderiv(fragShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(fragShader, 512, nullptr, infoLog);
        Log("Fragment shader compilation failed for " + name + ":");
        Log(infoLog);
        return false;
    }
    program = glCreateProgram();
    glAttachShader(program, vertShader);
    glAttachShader(program, fragShader);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        Log("Program linking failed for " + name + ":");
        Log(infoLog);
        return false;
    }
    glDeleteShader(vertShader);
    glDeleteShader(fragShader);
    Log("Shader loaded successfully: " + name);
    return true;
}

void App::DrawGrid() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(shader->GridShader);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glUniformMatrix4fv(shader->GridModel, 1, GL_FALSE, &shader->GridDefault[0][0]);
    glUniformMatrix4fv(shader->GridCameraProjection, 1, GL_FALSE, &camera->viewProjection[0][0]);
    glBindVertexArray(shader->GridVAO);
    glDrawArrays(GL_LINES, 0, shader->GridVertices.size());
    glBindVertexArray(0);
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glUseProgram(0);
}

Mesh& App::GetMeshByIndex(size_t index) {
    return meshes[index];
}

void App::LoadShaders() {
    if (!LoadShaderItem("grid",shader->GridShader)) { LogErr("Failed to load grid shader"); }
    if (!LoadShaderItem("mesh",shader->MeshShader)) { LogErr("Failed to load mesh shader"); }
    if (!LoadShaderItem("pick",shader->PickShader)) { LogErr("Failed to load pick shader"); }
    if (!LoadShaderItem("select",shader->SelectShader)) { LogErr("Failed to load select shader"); }
    shader->InitializeUniformsGrid();
    shader->InitializeUniformsMesh();
    shader->InitializeUniformsPick();
    shader->InitializeUniformsSelect();
    shader->CreatePickingFramebuffer(ScreenWidth, ScreenHeight);
}

void App::LoadMeshes() {
    meshes = Mesh::LoadMeshes("assets/floore.obj");
    for (auto& mesh : meshes) {
        mesh.SetCamPos(shader->MeshShader);
        mesh.AttachMaterial(shader->MeshShader);
        mesh.UpdateInstances();
    }
}

void App::CalcFPS() {
    double now = glfwGetTime();
    double delta = now - previousFrameTime;
    previousFrameTime = now;
    if (now - lastTitleUpdate >= 0.3) {
        if (delta > 0.0) {
            fps = std::to_string(int(1.0 / delta));
        }
        lastTitleUpdate = now;
    }
}

void App::Cleared() {

    // clear main buffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, ScreenWidth, ScreenHeight);
    glClearColor(background.r, background.g, background.b, background.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // clear pick buffer
    glBindFramebuffer(GL_FRAMEBUFFER, shader->PickingFBO);
    glClearColor(0,0,0,0);
    glViewport(0, 0, ScreenWidth, ScreenHeight);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

}

void App::DrawMeshes() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    for (auto& mesh : meshes) {
        mesh.Draw(shader->MeshShader,shader->MeshCameraProjection, camera);
    }
}

void App::DrawPicking() {
    glBindFramebuffer(GL_FRAMEBUFFER, shader->PickingFBO);
    glViewport(0,0,ScreenWidth,ScreenHeight);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glUseProgram(shader->PickShader);
    for (auto& mesh : meshes) {
        glUniform1ui(shader->PickMeshIDProjection, mesh.ID);
        mesh.Draw(shader->PickShader, shader->PickCameraProjection, camera);
    }
    unsigned int data[2] = {0,0};
    glReadPixels(input->GetMouseX(),ScreenHeight - input->GetMouseY(),1,1,GL_RG_INTEGER,GL_UNSIGNED_INT,data);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    SelectedMesh = data[0];
    SelectedInstance = data[1];
}

void App::DrawHover() {
    if (SelectedMesh == 0 || SelectedInstance == 0) {
        return;
    }
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glViewport(0, 0, ScreenWidth, ScreenHeight);
    glEnable(GL_DEPTH_TEST);
    glUseProgram(shader->SelectShader);
    meshes[SelectedMesh - 1].DrawInstance(SelectedInstance - 1, shader->SelectShader, shader->SelectCameraProjection, camera);
    glCullFace(GL_BACK);
    //Log(std::to_string(SelectedMesh) + " - " + std::to_string(SelectedInstance));
}

void App::PostCalculate() {
    gui->EndFrame();
}

void App::PreCalculate() {
    CalcFPS();
    std::string title = nameApp + " [BETA][" + fps+ " FPS]";
    glfwSetWindowTitle(window, title.c_str());
    input->Update();
    input->Handlers(camera);
    camera->CamCalculated(ScreenWidth,ScreenHeight);
    gui->StartFrame();
    Cleared();
    DrawGrid();
}

bool App::DrawGizmo() {
    bool statMove = false;
    ImGuizmo::BeginFrame();
    ImGuiIO& io = ImGui::GetIO();
    ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
    if (input->IsKeyDown(GLFW_KEY_W)) operation = ImGuizmo::TRANSLATE;
    if (input->IsKeyDown(GLFW_KEY_E)) operation = ImGuizmo::ROTATE;
    if (input->IsKeyDown(GLFW_KEY_R)) operation = ImGuizmo::SCALE;
    float snapTranslate[3] = {0.01f,0.01f,0.01f};
    float snapScale[3]     = {0.01f,0.01f,0.01f};
    float snapRotate       = 5.0f;
    float* snapPtr = nullptr;
    if (operation == ImGuizmo::TRANSLATE) snapPtr = snapTranslate;
    if (operation == ImGuizmo::ROTATE)    snapPtr = &snapRotate;
    if (operation == ImGuizmo::SCALE)     snapPtr = snapScale;
    if (!gizmoMesh) return false;
    if (gizmoInstance < 0) return false;
    if (gizmoInstance >= gizmoMesh->instances.size()) return false;
    if (gizmoMesh->instances[gizmoInstance].selected < 1.0f) return false;
    glm::mat4 gizmoMatrix = gizmoMesh->instances[gizmoInstance].model;
    glm::mat4 oldMatrix   = gizmoMatrix;
    ImGuizmo::Manipulate(
        &camera->view[0][0],
        &camera->projection[0][0],
        operation,
        ImGuizmo::WORLD,
        &gizmoMatrix[0][0],
        nullptr,
        snapPtr
    );
    static bool duplicated = false;
    if (ImGuizmo::IsUsing()) {
        if (input->IsKeyDown(GLFW_KEY_LEFT_SHIFT) && !duplicated) {
            std::vector<std::pair<Mesh*,int>> newInstances;
            for (auto& mesh : meshes) {
                std::vector<size_t> selected;
                for (size_t i = 0; i < mesh.instances.size(); i++) {
                    if (mesh.instances[i].selected >= 1.0f)
                        selected.push_back(i);
                }
                for (size_t idx : selected) {
                    MeshInstance copy = mesh.instances[idx];
                    copy.selected = 1.0f;
                    mesh.instances[idx].selected = 0.0f;
                    mesh.instances.push_back(copy);
                    int newIndex = (int)mesh.instances.size() - 1;
                    newInstances.push_back({&mesh,newIndex});
                }
                if (!selected.empty())
                    mesh.UpdateInstances();
            }
            if (!newInstances.empty()) {
                gizmoMesh     = newInstances[0].first;
                gizmoInstance = newInstances[0].second;
            }
            duplicated = true;
        }
        glm::mat4 delta = gizmoMatrix * glm::inverse(oldMatrix);
        for (auto& mesh : meshes) {
            for (size_t i = 0; i < mesh.instances.size(); i++) {
                auto& inst = mesh.instances[i];
                if (inst.selected >= 1.0f) {
                    inst.model = delta * inst.model;
                    float t[3], r[3], s[3];
                    ImGuizmo::DecomposeMatrixToComponents(
                        &inst.model[0][0],
                        t, r, s
                    );
                    inst.SetGizmoModel(t, r, s);
                    mesh.UpdateInstance(i);
                }
            }
        }
        statMove = true;
    } else {
        duplicated = false;
    }
    return statMove;
}

void App::CheckSelect() {
    if (!input->ClickLeftMouse()) {
        return;
    }
    bool ctrl = input->IsPressCtrl();
    if (!ctrl) {
        for (auto& mesh : meshes) {
            for (auto& inst : mesh.instances) {
                inst.selected = 0.0f;
            }
        }
    }
    if (SelectedMesh <= 0 || SelectedInstance <= 0) {
        gizmoMesh = nullptr;
        gizmoInstance = -1;
        for (auto& mesh : meshes)
            mesh.UpdateInstances();

        return;
    }
    int meshIndex = SelectedMesh - 1;
    int instIndex = SelectedInstance - 1;
    if (meshIndex >= meshes.size()) {
        return;
    }
    Mesh& mesh = meshes[meshIndex];
    if (instIndex >= mesh.instances.size()) {
        return;
    }
    mesh.instances[instIndex].selected = 1.0f;
    gizmoMesh = &mesh;
    gizmoInstance = instIndex;
    for (auto& m : meshes) {
        m.UpdateInstances();
    }
}

void App::DeleteSelected() {
    if (!input->IsKeyDown(GLFW_KEY_DELETE)) {
        return;
    }
    gizmoMesh = nullptr;
    gizmoInstance = -1;
    for (auto& mesh : meshes) {
        for (int i = (int)mesh.instances.size() - 1; i >= 0; i--) {
            if (mesh.instances[i].selected >= 1.0f) {
                mesh.instances.erase(mesh.instances.begin() + i);
            }
        }
        mesh.UpdateInstances();
    }
}

void App::MirrorSelect() {
    static bool mHeld = false;
    bool pressed = input->IsKeyDown(GLFW_KEY_M);
    if (!pressed) {
        mHeld = false;
        return;
    }
    if (mHeld) return;
    mHeld = true;
    for (size_t m = 0; m < meshes.size(); ++m) {
        Mesh& srcMesh = meshes[m];
        Mesh& dstMesh = meshes[srcMesh.mirrorID];
        for (int i = (int)srcMesh.instances.size() - 1; i >= 0; --i) {
            MeshInstance& inst = srcMesh.instances[i];
            if (inst.selected > 0.5f) {
                MeshInstance moved = inst;
                moved.meshId = dstMesh.ID;
                moved.selected = 0.0f;
                dstMesh.instances.push_back(moved);
                srcMesh.instances.erase(srcMesh.instances.begin() + i);
            }
        }
    }
    for (auto& mesh : meshes) {
        mesh.UpdateInstances();
    }
}

int App::StartedRT() {
    gui->SetPointer(this);
    glfwGetWindowSize(window, &ScreenWidth, &ScreenHeight);
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        PreCalculate();
        // ==========================================

        DrawMeshes();

        if (!DrawGizmo()) {
            DrawPicking();
            DrawHover();
            CheckSelect();
            DeleteSelected();
            MirrorSelect();
        }

        // ==========================================
        PostCalculate();
        glfwSwapBuffers(window);
    }

    return 0;
}
