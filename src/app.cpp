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
    library = new Library();
    library->InitPreviewBuffer();

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
    library->PreviewShader = shader->MeshShader;
    Material mat = Material("baseHeight");
    mat.CreateDefaultHeightMap(shader->MeshShader, 128,128);
    materials.push_back(mat);
}

void App::LoadMeshes() {
    auto newMeshes = Mesh::LoadMeshes("assets/floore001.obj", meshes.size());
    size_t baseIndex = meshes.size();
    meshes.insert(meshes.end(), newMeshes.begin(), newMeshes.end());
    for (size_t i = baseIndex; i < meshes.size(); ++i) {
        meshes[i].SetCamPos(shader->MeshShader);
        meshes[i].AttachMaterial(shader->MeshShader);
        meshes[i].UpdateInstances();
    }
}

void App::SelectInstanceMesh(int indexMesh, int indexInst) {
    for (auto& mesh : meshes) {
        for (auto& inst : mesh.instances) {
            inst.selected = 0.0f;
        }
    }
    gizmoMesh = nullptr;
    gizmoInstance = -1;
    if (indexMesh < 0 || indexMesh >= (int)meshes.size()) {
        return;
    }
    Mesh& mesh = meshes[indexMesh];
    if (indexInst < 0 || indexInst >= (int)mesh.instances.size()) {
        return;
    }
    mesh.instances[indexInst].selected = 1.0f;
    gizmoMesh = &mesh;
    gizmoInstance = indexInst;
    for (auto& m : meshes) {
        m.UpdateInstances();
    }
}

void App::SnapSelectedVertex(float eps) {
    if (!gizmoMesh) return;
    if (gizmoInstance < 0) return;
    if (gizmoInstance >= gizmoMesh->instances.size()) return;
    Mesh& mesh = *gizmoMesh;
    MeshInstance& inst = mesh.instances[gizmoInstance];
    const float borderEps = 0.0001f;
    glm::vec3 min = mesh.vertices[0].position;
    glm::vec3 max = mesh.vertices[0].position;
    for (auto& v : mesh.vertices) {
        min = glm::min(min, v.position);
        max = glm::max(max, v.position);
    }
    std::vector<glm::vec3> selectedBorder;
    for (auto& v : mesh.vertices) {
        const glm::vec3& p = v.position;
        bool border =
            fabs(p.x - min.x) < borderEps ||
            fabs(p.x - max.x) < borderEps ||
            fabs(p.y - min.y) < borderEps ||
            fabs(p.y - max.y) < borderEps ||
            fabs(p.z - min.z) < borderEps ||
            fabs(p.z - max.z) < borderEps;
        if (!border) continue;
        selectedBorder.push_back(glm::vec3(inst.model * glm::vec4(p,1)));
    }
    for (auto& m : meshes) {
        glm::vec3 min2 = m.vertices[0].position;
        glm::vec3 max2 = m.vertices[0].position;
        for (auto& v : m.vertices) {
            min2 = glm::min(min2, v.position);
            max2 = glm::max(max2, v.position);
        }
        for (auto& otherInst : m.instances) {
            if (&m == &mesh && &otherInst == &inst)
                continue;
            int matchCount = 0;
            glm::vec3 snapDelta(0);
            for (auto& ov : m.vertices) {
                const glm::vec3& p2 = ov.position;
                bool border2 =
                    fabs(p2.x - min2.x) < borderEps ||
                    fabs(p2.x - max2.x) < borderEps ||
                    fabs(p2.y - min2.y) < borderEps ||
                    fabs(p2.y - max2.y) < borderEps ||
                    fabs(p2.z - min2.z) < borderEps ||
                    fabs(p2.z - max2.z) < borderEps;
                if (!border2) continue;
                glm::vec3 worldB = glm::vec3(otherInst.model * glm::vec4(p2,1));
                for (auto& worldA : selectedBorder) {
                    glm::vec3 d = worldB - worldA;
                    if (fabs(d.x) < eps && fabs(d.y) < eps && fabs(d.z) < eps) {
                        matchCount++;
                        if (matchCount == 1)
                            snapDelta = d;
                        if (matchCount >= 2) {
                            inst.position += snapDelta;
                            inst.UpdateModel();
                            mesh.UpdateInstance(gizmoInstance);
                            return;
                        }
                    }
                }
            }
        }
    }
}

glm::vec3 App::GetSpawnPosition() {
    glm::vec3 rayOrigin = camera->position;
    glm::vec3 rayDir = glm::normalize(camera->target - camera->position);
    float bestDist = FLT_MAX;
    glm::vec3 hitPos;
    bool hit = false;
    for (auto& mesh : meshes) {
        for (auto& inst : mesh.instances) {
            glm::vec3 min(FLT_MAX);
            glm::vec3 max(-FLT_MAX);
            for (auto& v : mesh.vertices) {
                glm::vec3 w = glm::vec3(inst.model * glm::vec4(v.position,1));
                min = glm::min(min, w);
                max = glm::max(max, w);
            }
            float tmin = 0.0f;
            float tmax = 1000.0f;
            for (int i = 0; i < 3; i++) {
                float invD = 1.0f / rayDir[i];
                float t0 = (min[i] - rayOrigin[i]) * invD;
                float t1 = (max[i] - rayOrigin[i]) * invD;
                if (invD < 0.0f)
                    std::swap(t0, t1);
                tmin = t0 > tmin ? t0 : tmin;
                tmax = t1 < tmax ? t1 : tmax;
                if (tmax <= tmin)
                    goto nextInstance;
            }
            if (tmin < bestDist && tmin > 0.0f) {
                bestDist = tmin;
                hitPos = rayOrigin + rayDir * tmin;
                hit = true;
            }
            nextInstance:;
        }
    }
    if (hit)
        return hitPos;
    if (camera->position.z < 0.0f) {
        return rayOrigin + rayDir * 50.0f;
    } else {
        if (fabs(rayDir.z) < 0.00001f)
            return rayOrigin + rayDir * 50.0f;
        float t = -rayOrigin.z / rayDir.z;
        if (t > 0.0f)
            return rayOrigin + rayDir * t;
        return rayOrigin + rayDir * 50.0f;
    }
}

void App::LoadMeshAsset(const std::string& path) {
    auto newMeshes = Mesh::LoadMeshes(path, meshes.size());
    size_t baseIndex = meshes.size();
    meshes.insert(meshes.end(), newMeshes.begin(), newMeshes.end());
    glm::vec3 spawnPos = GetSpawnPosition();
    for (size_t i = baseIndex; i < meshes.size(); ++i) {
        meshes[i].SetCamPos(shader->MeshShader);
        meshes[i].AttachMaterial(shader->MeshShader);
        meshes[i].UpdateInstances();
    }
    meshes[meshes.size()-2].instances[0].position = spawnPos;
    meshes[meshes.size()-2].UpdateInstance(0);
    SelectInstanceMesh(meshes.size()-2, 0);
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
        mesh.mat = &materials[0];
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
    if (SelectedMesh == 0 || SelectedInstance == 0)
        return;
    glViewport(0, 0, ScreenWidth, ScreenHeight);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    glUseProgram(shader->SelectShader);
    meshes[SelectedMesh - 1].DrawInstance(SelectedInstance - 1,shader->SelectShader,shader->SelectCameraProjection,camera);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_BLEND);
}

void App::PostCalculate() {
    gui->EndFrame();
}

void App::PreCalculate() {
    CalcFPS();
    std::string title = nameApp + " [BETA][" + fps+ " FPS]";
    glfwSetWindowTitle(window, title.c_str());
    gui->StartFrame();
    if (!gui->io->WantCaptureMouse) {
        input->Update();
        ChangeSnap();
        input->Handlers(camera);
        camera->CamCalculated(ScreenWidth,ScreenHeight);
    }
    Cleared();
    DrawGrid();
}

bool App::DrawGizmo() {
    bool statMove = false;
    ImGuizmo::BeginFrame();
    ImGuizmo::SetRect(0, 0, ScreenWidth, ScreenHeight);
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
        if (Snapping) {
            SnapSelectedVertex(0.2);
        }
        statMove = true;
    } else {
        duplicated = false;
    }
    return statMove;
}

void App::CheckSelect() {
    if (!input->ClickLeftMouse() || gui->io->WantCaptureMouse) {
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
        for (auto& mesh : meshes) {
            mesh.UpdateInstances();
        }
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
    SelectedAdd = false;
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

void App::FormedLibrary() {
    library->LoadElementsLib();
    meshes.clear();
    meshes.shrink_to_fit();
    gui->Assets = library->assets;
}

std::vector<Mesh>& App::GetMeshes() {
    return meshes;
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

void App::FocusSelected() {
    static bool zPressedLastFrame = false;
    bool zPressedNow = input->IsKeyDown(GLFW_KEY_Z);
    if (!(zPressedNow && !zPressedLastFrame)) {
        zPressedLastFrame = zPressedNow;
        return;
    }
    glm::vec3 min(FLT_MAX);
    glm::vec3 max(-FLT_MAX);
    bool hasInstances = false;
    bool hasSelected  = false;
    for (auto& mesh : meshes) {
        for (auto& inst : mesh.instances) {
            hasInstances = true;
            if (inst.selected == 0)
                continue;
            hasSelected = true;
            for (auto& vertex : mesh.vertices) {
                glm::vec3 w = glm::vec3(inst.model * glm::vec4(vertex.position,1));
                min = glm::min(min,w);
                max = glm::max(max,w);
            }
        }
    }
    if (!hasSelected && hasInstances) {
        min = glm::vec3(FLT_MAX);
        max = glm::vec3(-FLT_MAX);
        for (auto& mesh : meshes) {
            for (auto& inst : mesh.instances) {
                for (auto& vertex : mesh.vertices) {
                    glm::vec3 w = glm::vec3(inst.model * glm::vec4(vertex.position,1));
                    min = glm::min(min,w);
                    max = glm::max(max,w);
                }
            }
        }
    }
    if (!hasInstances) {
        camera->position = {-130.0f, -130.0f, 90.0f};
        camera->target   = {0.0f, 0.0f, 0.0f};
        zPressedLastFrame = zPressedNow;
        return;
    }
    glm::vec3 center = (min + max) * 0.5f;
    glm::vec3 offset = camera->position - camera->target;
    float oldDistance = glm::length(offset);
    if (oldDistance < 0.001f)
        offset = glm::vec3(0,0,-1) * 5.0f;
    else
        offset = glm::normalize(offset) * oldDistance;
    camera->target = center;
    glm::vec3 size = max - min;
    float radius = glm::length(size) * 0.5f;
    if (radius > 0.001f) {
        float distance =
            radius / tan(glm::radians(camera->fov * 0.5f));
        distance = glm::max(distance, 0.1f);
        camera->position = center + glm::normalize(offset) * distance;
    }
    zPressedLastFrame = zPressedNow;
}

void App::ChangeSnap() {
    static bool duplicated = false;
    if (input->IsKeyDown(GLFW_KEY_A) && !duplicated) {
        Snapping = !Snapping;
        duplicated = true;
    }
    if (!input->IsKeyDown(GLFW_KEY_A) && duplicated) {
        duplicated = false;
    }
}

int App::StartedRT() {
    gui->SetPointer(this);
    glfwGetWindowSize(window, &ScreenWidth, &ScreenHeight);
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        PreCalculate();
        DrawMeshes();
        // ==========================================
        if (!DrawGizmo()) {
            DrawPicking();
            DrawHover();
            CheckSelect();
            DeleteSelected();
            MirrorSelect();
            FocusSelected();
        }
        // ==========================================
        gui->RenderPanels();
        PostCalculate();
        glfwSwapBuffers(window);
    }
    return 0;
}
