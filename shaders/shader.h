#pragma once
#include <iostream>
#include <ostream>
#include "../gl/glincludes.h"

class Shader {
public:

    // SELECT =======================================
    GLuint SelectShader = 0;
    GLuint SelectCameraProjection = 0;
    GLuint SelectIndexInstance = 0;

    // PICK =========================================
    GLuint PickShader = 0;
    GLuint PickingFBO = 0;
    GLuint PickingIDTex = 0;
    GLuint PickingDepth = 0;
    GLuint PickCameraProjection = 0;
    GLuint PickMeshIDProjection = 0;

    // MAIN =========================================
    GLuint MainFBO = 0;
    GLuint MainColor = 0;
    GLuint MainDepth = 0;
    GLuint MainID = 0;
    GLuint ResolveFBO = 0;
    GLuint ResolveTex = 0;

    // GRID =========================================
    GLuint GridShader = 0;
    GLuint GridVAO = 0;
    GLuint GridVBO = 0;
    GLuint GridModel = 0;
    GLuint GridCameraProjection = 0;
    int GridHalf = 10;
    float CellSize = 5.0f;
    std::vector<glm::vec3> GridVertices;
    glm::mat4 GridDefault = glm::mat4(1.0f);

    // MESH =========================================
    GLuint MeshShader = 0;
    GLuint MeshCameraProjection = 0;

    // PREVIEW ======================================
    GLuint PreviewShader = 0;

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

    // Pick ========================================================================
    void InitializeUniformsPick() {
        PickCameraProjection = glGetUniformLocation(PickShader, "viewProjection");
        PickMeshIDProjection = glGetUniformLocation(PickShader, "meshID");
    }

    // Select ======================================================================
    void InitializeUniformsSelect() {
        SelectCameraProjection = glGetUniformLocation(SelectShader, "viewProjection");
        SelectIndexInstance = glGetUniformLocation(SelectShader, "indexInstance");
    }

    // create grid
    void GenGridVerticles() {
        GridVertices.clear();
        for (int i = -GridHalf; i <= GridHalf; ++i) {
            float offset = i * CellSize;
            GridVertices.push_back({ offset, -GridHalf * CellSize, 0.0f });
            GridVertices.push_back({ offset,  GridHalf * CellSize, 0.0f });
            GridVertices.push_back({ -GridHalf * CellSize, offset, 0.0f });
            GridVertices.push_back({  GridHalf * CellSize, offset, 0.0f });
        }
    }

    void CreatePickingFramebuffer(int width, int height) {
        glGenFramebuffers(1, &PickingFBO);
        glBindFramebuffer(GL_FRAMEBUFFER, PickingFBO);
        glGenTextures(1, &PickingIDTex);
        glBindTexture(GL_TEXTURE_2D, PickingIDTex);
        glTexImage2D(GL_TEXTURE_2D,0,GL_RG32UI,width,height,0,GL_RG_INTEGER,GL_UNSIGNED_INT,nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,PickingIDTex,0);
        glGenRenderbuffers(1, &PickingDepth);
        glBindRenderbuffer(GL_RENDERBUFFER, PickingDepth);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
        glFramebufferRenderbuffer(
            GL_FRAMEBUFFER,
            GL_DEPTH_ATTACHMENT,
            GL_RENDERBUFFER,
            PickingDepth
        );
        GLenum attachments[1] = { GL_COLOR_ATTACHMENT0 };
        glDrawBuffers(1, attachments);
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cout << "PickingFBO not complete\n";
            std::abort();
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

};