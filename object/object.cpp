#include "object.h"

#include <iostream>

#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

glm::mat4 Object::GetModelMatrix() const {
    glm::mat4 T = glm::translate(glm::mat4(1.0f), Position);
    glm::mat4 Rx = glm::rotate(glm::mat4(1.0f),glm::radians(Rotation.x),glm::vec3(1,0,0));
    glm::mat4 Ry = glm::rotate(glm::mat4(1.0f),glm::radians(Rotation.y),glm::vec3(0,1,0));
    glm::mat4 Rz = glm::rotate(glm::mat4(1.0f),glm::radians(Rotation.z),glm::vec3(0,0,1));
    glm::mat4 S = glm::scale(glm::mat4(1.0f), Scale);
    return T * Rz * Ry * Rx * S;
}

std::vector<Object> LoadObjs(const std::string& path) {
    std::vector<Object> meshes;
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
        Object newMesh;
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