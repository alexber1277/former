#include "render.h"
#include <fstream>
#include <sstream>
#include <string>
#include <stdexcept>
#include <vector>

GLuint Render::LoadCMShader(const std::string& path) {
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

void Render::ResizeRenderTexture(int width, int height) {
    glBindTexture(GL_TEXTURE_2D, BaseTexture);
    glTexImage2D(
        GL_TEXTURE_2D,0,GL_RGBA32F,
        width,height,0,GL_RGBA,
        GL_FLOAT,nullptr
    );
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Render::CreateRenderTexture(int ScreenWidth, int ScreenHeight) {
    glGenTextures(1, &BaseTexture);
    glBindTexture(GL_TEXTURE_2D, BaseTexture);
    glTexImage2D(
        GL_TEXTURE_2D,0,
        GL_RGBA32F,ScreenWidth,ScreenHeight,
        0,GL_RGBA,GL_FLOAT,nullptr
    );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(BaseQuadVAO);
    glBindVertexArray(0);
    BaseResolutLoc = glGetUniformLocation(BaseShader, "uResolution");
}

void Render::ClearCompute(int ScreenWidth, int ScreenHeight) {
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glViewport(0, 0, ScreenWidth, ScreenHeight);
    glClearColor(1.0f, 0.5f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

}

void Render::RunCompute(int ScreenWidth, int ScreenHeight) {
    glUseProgram(BaseShader);
    glUniform2f(BaseResolutLoc,ScreenWidth,ScreenHeight);
    glBindImageTexture(0,BaseTexture,0,GL_FALSE,0,GL_WRITE_ONLY,GL_RGBA32F);
    glDispatchCompute((ScreenWidth + 15) / 16,(ScreenHeight + 15) / 16,1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    ClearCompute(ScreenWidth, ScreenHeight);
    DrawFullscreen();
}

void Render::DrawFullscreen() {
    glUseProgram(BaseQuadShader);
    glBindVertexArray(BaseQuadVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, BaseTexture);
    glUniform1i(glGetUniformLocation(BaseQuadShader, "screenTex"),0);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);
}