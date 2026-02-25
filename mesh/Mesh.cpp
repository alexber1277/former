#include "Mesh.h"
#include <iostream>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "../camera/camera.h"

std::vector<Mesh> Mesh::LoadMesh(const std::string& path) {
    std::vector<Mesh> meshes;
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path,
        aiProcess_Triangulate |
        aiProcess_JoinIdenticalVertices |
        aiProcess_GlobalScale
    );
    if (!scene || !scene->HasMeshes()) {
        std::cerr << "Failed to load: " << path << std::endl;
        return meshes;
    }
    for (unsigned int m = 0; m < scene->mNumMeshes; ++m) {
        aiMesh* mesh = scene->mMeshes[m];
        Mesh newMesh;
        for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
            const aiVector3D& pos = mesh->mVertices[i];
            newMesh.m_Positions.emplace_back(pos.x, pos.y, pos.z, 1.0f);
            if (mesh->HasNormals()) {
                const aiVector3D& norm = mesh->mNormals[i];
                newMesh.m_Normals.emplace_back(norm.x, norm.y, norm.z);
            } else {
                newMesh.m_Normals.emplace_back(0.0f, 0.0f, 1.0f);
            }
        }
        for (unsigned int f = 0; f < mesh->mNumFaces; ++f) {
            const aiFace& face = mesh->mFaces[f];
            if (face.mNumIndices == 3) {
                newMesh.m_Indices.emplace_back(
                    face.mIndices[0],
                    face.mIndices[1],
                    face.mIndices[2]
                );
            }
        }
        newMesh.m_VertexCount = (uint32_t)newMesh.m_Positions.size();
        newMesh.m_PrimitiveCount = (uint32_t)newMesh.m_Indices.size();
        newMesh.m_NormalCount = (uint32_t)newMesh.m_Normals.size();
        meshes.push_back(std::move(newMesh));
    }
    return meshes;
}

glm::mat4 Mesh::GetModelMatrix() const {
    glm::mat4 T = glm::translate(glm::mat4(1.0f), Position);
    glm::mat4 Rx = glm::rotate(glm::mat4(1.0f),glm::radians(Rotation.x),glm::vec3(1,0,0));
    glm::mat4 Ry = glm::rotate(glm::mat4(1.0f),glm::radians(Rotation.y),glm::vec3(0,1,0));
    glm::mat4 Rz = glm::rotate(glm::mat4(1.0f),glm::radians(Rotation.z),glm::vec3(0,0,1));
    glm::mat4 S = glm::scale(glm::mat4(1.0f), Scale);
    return T * Rz * Ry * Rx * S;
}

void Mesh::UploadToGPU() {
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    std::vector<float> vertexData;
    vertexData.reserve(m_Positions.size() * 6);
    for (size_t i = 0; i < m_Positions.size(); ++i) {
        vertexData.push_back(m_Positions[i].x);
        vertexData.push_back(m_Positions[i].y);
        vertexData.push_back(m_Positions[i].z);

        vertexData.push_back(m_Normals[i].x);
        vertexData.push_back(m_Normals[i].y);
        vertexData.push_back(m_Normals[i].z);
    }
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float), vertexData.data(), GL_STATIC_DRAW);
    std::vector<unsigned int> indices;
    indices.reserve(m_Indices.size() * 3);
    for (auto& tri : m_Indices) {
        indices.push_back(tri.x);
        indices.push_back(tri.y);
        indices.push_back(tri.z);
    }
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glBindVertexArray(0);
}

void Mesh::Draw(Camera* camera) {
    glUseProgram(Shader);
    GLint loc = glGetUniformLocation(Shader, "u_Model");
    glm::mat4 model = GetModelMatrix();
    glUniformMatrix4fv(loc, 1, GL_FALSE, &model[0][0]);
    loc = glGetUniformLocation(Shader, "u_View");
    glUniformMatrix4fv(loc, 1, GL_FALSE, &camera->view[0][0]);
    loc = glGetUniformLocation(Shader, "u_Proj");
    glUniformMatrix4fv(loc, 1, GL_FALSE, &camera->projection[0][0]);
    glUniform3fv(glGetUniformLocation(Shader, "u_CamPos"), 1, &camera->position[0]);

    glUniform3f(glGetUniformLocation(Shader, "u_BaseColor"), 0.9f, 0.9f, 0.9f);
    glUniform1f(glGetUniformLocation(Shader, "u_Metallic"), 0.3f);
    glUniform1f(glGetUniformLocation(Shader, "u_Roughness"), 0.1f);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, m_PrimitiveCount * 3, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}
