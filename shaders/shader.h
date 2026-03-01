#pragma once
#include <iostream>
#include <ostream>
#include "../gl/glincludes.h"

class Shader {
public:

    GLuint Grid = 0;
    GLuint GridVAO = 0;
    GLuint GridVBO = 0;
    GLuint GridCameraProjection = 0;

    GLuint Model = 0;
    GLuint ModelCameraProjection = 0;

    void InitializeUniforms() {

        GridCameraProjection = glGetUniformLocation(Grid, "viewProjection");
        ModelCameraProjection = glGetUniformLocation(Model, "viewProjection");
        glGenVertexArrays(1, &GridVAO);
        glGenBuffers(1, &GridVBO);
    };

};