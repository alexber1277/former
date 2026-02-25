#include <GL/glew.h>
#include "app.h"
#include <fstream>
#include <iostream>
#include <ostream>
#include <sstream>

#include "glm/gtc/random.hpp"
#include "glm/gtc/type_ptr.inl"

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
    CreateGrid(10,10.0);

    BaseCallback();

    std::cout << "========================================================" << std::endl;
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLFW Version: " << glfwGetVersionString() << std::endl;
    std::cout << "========================================================" << std::endl;

}

App::~App() {
    glDeleteProgram(baseShader);
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
                app->CreateOptBuffers();
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

void App::CalcFPS() {
    currentFrameTime = glfwGetTime();
    double currDelta = currentFrameTime - previousFrameTime;
    if (currentFrameTime - lastTitleUpdate >= 0.5) {
        if (currDelta > 0) {
            fps = std::to_string((int)(1.0 / currDelta));
            lastTitleUpdate = currentFrameTime;
        }
    }
    previousFrameTime = currentFrameTime;
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
    auto loaded = Mesh::LoadMesh("assets/cube.obj");
    if (loaded.empty()) {
        Log("failed to load meshes");
        return;
    }
    meshes.insert(meshes.end(),loaded.begin(),loaded.end());
    meshes.insert(meshes.end(),loaded.begin(),loaded.end());
    for (auto& mesh : meshes) {
        mesh.UploadToGPU();
        mesh.Shader = geomShader;
    }
    meshes[meshes.size() - 1].Position = glm::vec3(2.0f, 2.0f, 2.0f);
}

void App::ShowGrid(GLuint shader, int lineCount) {
    glUseProgram(shader);
    glm::mat4 model = glm::mat4(1.0f);
    GLint loc = glGetUniformLocation(shader, "u_Model");
    glUniformMatrix4fv(loc, 1, GL_FALSE, &model[0][0]);
    loc = glGetUniformLocation(shader, "u_View");
    glUniformMatrix4fv(loc, 1, GL_FALSE, &camera->view[0][0]);
    loc = glGetUniformLocation(shader, "u_Proj");
    glUniformMatrix4fv(loc, 1, GL_FALSE, &camera->projection[0][0]);
    glBindVertexArray(gridVao);
    glDrawArrays(GL_LINES, 0, lineCount * 4);
    glBindVertexArray(0);
}

GLuint App::CreateGrid(int size, float step) {
    std::vector<glm::vec3> vertices;
    for (int i = -size; i <= size; ++i) {
        vertices.push_back({float(i) * step, -size * step, 0.0f});
        vertices.push_back({float(i) * step, size * step, 0.0f});
    }
    for (int i = -size; i <= size; ++i) {
        vertices.push_back({-size * step, float(i) * step, 0.0f});
        vertices.push_back({size * step, float(i) * step, 0.0f});
    }
    GLuint vao, vbo;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0); // позиция
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glBindVertexArray(0);
    gridVao = vao;
    return vao;
}

void App::Calculate() {
    CalcFPS();
    std::string name = nameApp + " [" +fps.c_str() + " FPS]";
    glfwSetWindowTitle(window, name.c_str());
    glBindFramebuffer(GL_FRAMEBUFFER, depthFBO);
    glClearColor(0,0,0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(background.x, background.y, background.z, background.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    input->Update();
    input->Handlers(camera);
    camera->CamCalculated(ScreenWidth,ScreenHeight);
}


void App::CreateOptBuffers() {
    glGenFramebuffers(1, &depthFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, depthFBO);

    glGenTextures(1, &colorTEX);
    glBindTexture(GL_TEXTURE_2D, colorTEX);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F,ScreenWidth, ScreenHeight,0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,colorTEX, 0);

    glGenTextures(1, &depthTEX);
    glBindTexture(GL_TEXTURE_2D, depthTEX);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F,ScreenWidth, ScreenHeight,0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT1,GL_TEXTURE_2D,depthTEX, 0);

    glGenRenderbuffers(1, &depthRBT);
    glBindRenderbuffer(GL_RENDERBUFFER, depthRBT);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, ScreenWidth, ScreenHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRBT);

    GLenum drawBuffers[2] = {
        GL_COLOR_ATTACHMENT0,
        GL_COLOR_ATTACHMENT1
    };

    glDrawBuffers(2, drawBuffers);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "FBO not complete!" << std::endl;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

void App::RenderFullscreenQuad() {
    if (quadVAO < 1 || quadVAO > 1000000) {
        float quadVertices[] = {
            -1.0f,  1.0f,  0.0f, 1.0f,
            -1.0f, -1.0f,  0.0f, 0.0f,
             1.0f, -1.0f,  1.0f, 0.0f,

            -1.0f,  1.0f,  0.0f, 1.0f,
             1.0f, -1.0f,  1.0f, 0.0f,
             1.0f,  1.0f,  1.0f, 1.0f
        };
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        glBindVertexArray(0);
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

int App::Started() {
    if (!GLEW_NV_mesh_shader) {
        Log("Mesh shaders not supported on this GPU");
        return 1;
    }
    glViewport(0, 0, ScreenWidth, ScreenHeight);
    CreateOptBuffers();
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        Calculate();
        ShowGrid(baseShader, 100);

        glDisable(GL_BLEND);
        for (auto mesh : meshes) {
            mesh.Draw(camera);
        }
        glEnable(GL_BLEND);

        glfwSwapBuffers(window);
    }
    return 0;
}

int App::StartedRT() {
    glViewport(0, 0, ScreenWidth, ScreenHeight);
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();



        glfwSwapBuffers(window);
    }
    return 0;
}



