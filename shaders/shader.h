#pragma once
#include <iostream>
#include <ostream>
#include "../gl/glincludes.h"

class Shader {
public:

    // PICK =========================================
    GLuint PickFBO = 0;

    // GRID =========================================
    GLuint GridShader = 0;
    GLuint GridVAO = 0;
    GLuint GridVBO = 0;
    GLuint GridModel = 0;
    GLuint GridCameraProjection = 0;
    int GridHalf = 10;
    std::vector<glm::vec3> GridVertices;
    glm::mat4 GridDefault = glm::mat4(1.0f);

    // MESH =========================================
    GLuint MeshShader = 0;
    GLuint MeshCameraProjection = 0;

    // PBO ==========================================
    GLuint PickPBO;

    // Grid ========================================================================
    void InitializeUniformsGrid() {
        GenGridVerticles();
        GridCameraProjection = glGetUniformLocation(GridShader, "u_ViewProjection");
        GridModel = glGetUniformLocation(GridShader, "u_Model");
        glGenVertexArrays(1, &GridVAO);
        glGenBuffers(1, &GridVBO);
        glBindVertexArray(GridVAO);
        glBindBuffer(GL_ARRAY_BUFFER, GridVBO);
        glBufferData(GL_ARRAY_BUFFER,GridVertices.size() * sizeof(glm::vec3),GridVertices.data(),GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    };

    // Mesh ========================================================================
    void InitializeUniformsMesh() {
        MeshCameraProjection = glGetUniformLocation(MeshShader, "viewProjection");
    }

    // create grid
    void GenGridVerticles() {
        GridVertices.clear();
        for (int i = -GridHalf; i <= GridHalf; ++i) {
            GridVertices.push_back({ (float)i, -GridHalf, 0.0f });
            GridVertices.push_back({ (float)i,  GridHalf, 0.0f });
            GridVertices.push_back({ -GridHalf, (float)i, 0.0f });
            GridVertices.push_back({  GridHalf, (float)i, 0.0f });
        }
    }

    void CreatePickPBO() {
        glGenBuffers(1, &PickPBO);
        glBindBuffer(GL_PIXEL_PACK_BUFFER, PickPBO);
        glBufferData(GL_PIXEL_PACK_BUFFER, sizeof(GLuint) * 2, nullptr, GL_STREAM_READ);
        glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
    }

};