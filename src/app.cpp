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
    glfwWindowHint(GLFW_SAMPLES, 4);
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

    if (!LoadShaderItem("grid",shader->GridShader)) { LogErr("Failed to load grid shader"); }
    if (!LoadShaderItem("mesh",shader->MeshShader)) { LogErr("Failed to load mesh shader"); }
    shader->InitializeUniformsGrid();
    shader->InitializeUniformsMesh();

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
    glUseProgram(shader->GridShader);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glUniformMatrix4fv(shader->GridModel, 1, GL_FALSE, &shader->GridDefault[0][0]);
    glUniformMatrix4fv(shader->GridCameraProjection, 1, GL_FALSE, &camera->viewProjection[0][0]);
    glBindVertexArray(shader->GridVAO);
    glDrawArrays(GL_LINES, 0, shader->GridVertices.size());
    glBindVertexArray(0);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glUseProgram(0);
}

int App::PickInstance() {
    int selectedInstance = -1;


    return selectedInstance;
}

Mesh& App::GetMeshByIndex(size_t index) {
    return meshes[index];
}

void App::LoadMeshes() {
    meshes = Mesh::LoadMeshes("assets/cube.obj");
    for (auto& mesh : meshes) {
        mesh.SetCamPos(shader->MeshShader);
        mesh.AttachMaterial(shader->MeshShader);
        mesh.AddInstance(glm::vec3(-4, -4, 3));
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

void App::PreCalculate() {
    CalcFPS();
    std::string title = nameApp + " [BETA][" + fps+ " FPS]";
    glViewport(0, 0, ScreenWidth, ScreenHeight);
    glfwSetWindowTitle(window, title.c_str());
    input->Update();
    input->Handlers(camera);
    camera->CamCalculated(ScreenWidth,ScreenHeight);
    Cleared();
    DrawGrid();
}

void App::Cleared() {
    glClearColor(background.r, background.g, background.b, background.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

int App::StartedRT() {
    gui->SetPointer(this);
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        PreCalculate();
        // =================================

        for (auto& mesh : meshes) {
            mesh.Draw(shader->MeshShader,shader->MeshCameraProjection,camera);
        }

        //Log(std::to_string(PickInstance()));

        // =================================
        gui->RenderPanels();
        glfwSwapBuffers(window);
    }
    return 0;
}
