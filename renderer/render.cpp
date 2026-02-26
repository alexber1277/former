#include "render.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <stdexcept>
#include <vector>

#include "../camera/camera.h"
#include "glm/vec4.hpp"

GLuint Render::LoadCmpShader(const std::string& path) {
    GLuint shader = CompileComputeShader("shaders/"+path);
    GLuint program = glCreateProgram();
    glAttachShader(program, shader);
    glLinkProgram(program);
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        GLint logLength;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
        std::vector<char> log(logLength);
        glGetProgramInfoLog(program, logLength, nullptr, log.data());
        std::string error(log.begin(), log.end());
        glDeleteProgram(program);
        glDeleteShader(shader);
        throw std::runtime_error("Compute program link error:\n" + error);
    }
    glDetachShader(program, shader);
    glDeleteShader(shader);
    return program;
}

void Render::Resie() {
    glViewport(0, 0, ScreenWidth, ScreenHeight);
    glBindTexture(GL_TEXTURE_2D, TEXCompShader);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, ScreenWidth, ScreenHeight,0, GL_RGBA, GL_FLOAT, nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Render::Log(const std::string& msg) {
    std::cout << msg << std::endl;
}

std::string Render::LoadFile(const std::string& path) {
    std::ifstream file(path, std::ios::in);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open shader file: " + path);
    }
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

GLuint Render::CompileComputeShader(const std::string& path) {
    std::string source = LoadFile(path);
    const char* src = source.c_str();
    GLuint shader = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLint logLength;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
        std::vector<char> log(logLength);
        glGetShaderInfoLog(shader, logLength, nullptr, log.data());
        std::string error(log.begin(), log.end());
        glDeleteShader(shader);
        throw std::runtime_error("Compute shader compile error:\n" + error);
    }
    return shader;
}

bool Render::CreatePipeline() {
    ComputeShader = LoadCmpShader("base.comp");
    if (ComputeShader == 0 ) {
        Log("Failed to load compute shader");
        return false;
    }
    if (QuadShader == 0) {
        Log("Failed to load quad shader");
        return false;
    }

    UCamPosLoc = glGetUniformLocation(ComputeShader, "uCamPos");
    UCamTargLoc = glGetUniformLocation(ComputeShader, "uCamTarg");
    UCamUpLoc = glGetUniformLocation(ComputeShader, "uCamUp");
    UCamFovLoc = glGetUniformLocation(ComputeShader, "uCamFov");
    UBackground = glGetUniformLocation(ComputeShader, "uBackground");

    UResLoc = glGetUniformLocation(ComputeShader, "uResolution");
    if (UResLoc < 0) {
        Log("Uniform uResolution not found in compute shader");
        return false;
    }

    UScreenTex = glGetUniformLocation(QuadShader, "screenTex");
    if (UScreenTex < 0) {
        Log("Uniform screenTex not found in quad shader");
        return false;
    }
    UQuadResolute = glGetUniformLocation(QuadShader, "uResolution");
    if (UQuadResolute < 0) {
        Log("Uniform screenTex not found in quad shader");
    }

    glUseProgram(QuadShader);
    glUniform1i(UScreenTex, 0);
    glUseProgram(0);

    glGenVertexArrays(1, &QuadVAO);
    glBindVertexArray(QuadVAO);
    glBindVertexArray(0);

    glGenTextures(1, &TEXCompShader);
    glBindTexture(GL_TEXTURE_2D, TEXCompShader);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, ScreenWidth, ScreenHeight,0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    return true;
}

void Render::SetCamOrig(Camera* camera) {
    camPos = camera->position;
    camTarg = camera->target;
    camUp = camera->up;
    camFov = camera->fov;
}

void Render::RunCompute(glm::vec4 colorClear) {

    // clear buffer
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glClearColor(colorClear.r, colorClear.g, colorClear.b, colorClear.a);
    glClear(GL_COLOR_BUFFER_BIT);

    // run compute shader
    glUseProgram(ComputeShader);
    glUniform2f(UResLoc, ScreenWidth, ScreenHeight);

    // send camera settings
    glUniform3fv(UCamPosLoc,1, &camPos[0]);
    glUniform3fv(UCamTargLoc,1, &camTarg[0]);
    glUniform3fv(UCamUpLoc,1, &camUp[0]);
    glUniform3fv(UBackground,1, &colorClear[0]);
    glUniform1f(UCamFovLoc, camFov);

    glBindImageTexture(0, TEXCompShader, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glDispatchCompute((ScreenWidth + 15) / 16, (ScreenHeight + 15) / 16, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    // render fullscreen quad
    glUseProgram(QuadShader);
    glBindVertexArray(QuadVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, TEXCompShader);
    glUniform1i(UScreenTex, 0);
    glUniform2f(UQuadResolute, ScreenWidth, ScreenHeight);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);

    // unbind shader
    glUseProgram(0);
}