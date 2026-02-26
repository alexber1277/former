#include "object.h"
#include <iostream>
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

std::vector<Object> Object::LoadObjs(const std::string& path) {
    std::vector<Object> objects;
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path,aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_GlobalScale);
    if (!scene || !scene->HasMeshes()) {
        return objects;
    }
    for (unsigned int m = 0; m < scene->mNumMeshes; ++m) {
        aiMesh* mesh = scene->mMeshes[m];
        Object obj;
        obj.vertices.reserve(mesh->mNumVertices);
        for (unsigned int v = 0; v < mesh->mNumVertices; ++v) {
            Vertex vert{};
            vert.pos = glm::vec4(
                mesh->mVertices[v].x,
                mesh->mVertices[v].y,
                mesh->mVertices[v].z,
                1.0f
            );
            if (mesh->HasNormals()) {
                vert.normal = glm::vec4(
                    mesh->mNormals[v].x,
                    mesh->mNormals[v].y,
                    mesh->mNormals[v].z,
                    0.0f
                );
            } else {
                vert.normal = glm::vec4(0, 0, 1, 0);
            }
            if (mesh->HasTextureCoords(0)) {
                vert.uv = glm::vec2(
                    mesh->mTextureCoords[0][v].x,
                    mesh->mTextureCoords[0][v].y
                );
            } else {
                vert.uv = glm::vec2(0.0f);
            }
            vert.pad = glm::vec2(0.0f);
            obj.vertices.push_back(vert);
        }
        obj.indices.reserve(mesh->mNumFaces * 3);
        for (unsigned int f = 0; f < mesh->mNumFaces; ++f) {
            const aiFace& face = mesh->mFaces[f];
            obj.indices.push_back(face.mIndices[0]);
            obj.indices.push_back(face.mIndices[1]);
            obj.indices.push_back(face.mIndices[2]);
        }
        obj.meshCount = static_cast<uint32_t>(obj.indices.size());
        obj.meshOffset = 0;
        obj.matBaseColor = glm::vec4(1.0f);
        obj.matRoughness = 0.5f;
        obj.matMetallic  = 0.0f;
        obj.matPad       = glm::vec2(0.0f);
        if (scene->HasMaterials()) {
            aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
            aiColor3D color(1.f, 1.f, 1.f);
            if (material->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS) {
                obj.matBaseColor = glm::vec4(color.r, color.g, color.b, 1.0f);
            }
        }
        Instance inst{};
        inst.model = glm::mat4(1.0f);
        obj.instances.push_back(inst);
        objects.push_back(std::move(obj));
    }
    return objects;
}
