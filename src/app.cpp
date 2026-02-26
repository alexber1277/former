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
    glfwSwapInterval(1);
    glfwMaximizeWindow(window);

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
    input = new Input(window);

    render = new Render(window, ScreenWidth, ScreenHeight);
    LoadShaderItem("quad", render->QuadShader);
    if (!render->CreatePipeline()) {
        throw std::runtime_error("Failed to create compute pipeline");
    }

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

void App::BaseCallback() {
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window,
        [](GLFWwindow* window, int width, int height){
            glViewport(0, 0, width, height);
            App* app = static_cast<App*>(glfwGetWindowUserPointer(window));
            if (app) {
                app->ScreenWidth = width;
                app->ScreenHeight = height;
                app->render->ScreenWidth = width;
                app->render->ScreenHeight = height;
                app->render->Resie();
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


void App::UploadMeshesToGPU() {
    std::vector<Vertex> globalVertices;
    std::vector<uint32_t> globalIndices;
    uint32_t vertexOffset = 0;
    uint32_t indexOffset  = 0;

    for (auto& obj : objects) {
        obj.meshOffset = indexOffset;
        glm::mat4 model = obj.GetModelMatrix();
        glm::mat3 normalMat = glm::transpose(glm::inverse(glm::mat3(model)));
        for (auto& v : obj.vertices) {
            Vertex vert = v;
            vert.pos = model * vert.pos;
            vert.normal = glm::vec4(normalMat * glm::vec3(v.normal), 0.0f);
            globalVertices.push_back(vert);
        }
        for (uint32_t idx : obj.indices) {
            globalIndices.push_back(idx + vertexOffset);
        }
        obj.meshCount = static_cast<uint32_t>(obj.indices.size());
        vertexOffset += static_cast<uint32_t>(obj.vertices.size());
        indexOffset  += static_cast<uint32_t>(obj.indices.size());
    }
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // for (auto& obj : objects) {
    //     obj.meshOffset = indexOffset;
    //     globalVertices.insert(globalVertices.end(),obj.vertices.begin(),obj.vertices.end());
    //     for (uint32_t idx : obj.indices) {
    //         globalIndices.push_back(idx + vertexOffset);
    //     }
    //     obj.meshCount = static_cast<uint32_t>(obj.indices.size());
    //     vertexOffset += static_cast<uint32_t>(obj.vertices.size());
    //     indexOffset  += static_cast<uint32_t>(obj.indices.size());
    // }
    glGenBuffers(1, &vertexSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, vertexSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER,globalVertices.size() * sizeof(Vertex),globalVertices.data(),GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, vertexSSBO);
    glGenBuffers(1, &indexSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, indexSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER,globalIndices.size() * sizeof(uint32_t),globalIndices.data(),GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, indexSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

bool App::LoadShaderItem(const std::string& name, GLuint& program) {
    std::string basePath = "shaders/";
    std::string vertPath = basePath + name + ".vs";
    std::string fragPath = basePath + name + ".fs";
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

void App::LoadMeshes() {
    objects = Object::LoadObjs("assets/cube.obj");
    UploadMeshesToGPU();
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
    std::string title = nameApp + " [" + fps+ "]";
    glfwSetWindowTitle(window, title.c_str());
    input->Handlers(camera);
    camera->CamCalculated(ScreenWidth, ScreenHeight);
    render->SetCamOrig(camera);
}

int App::StartedRT() {
    glViewport(0, 0, ScreenWidth, ScreenHeight);
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        PreCalculate();
        render->RunCompute(background);
        glfwSwapBuffers(window);
    }
    return 0;
}

