#include "object.h"
#include <iostream>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
std::vector<Object> Object::LoadObjs(const std::string& path) {
    std::vector<Object> objects;
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(
        path,
        aiProcess_Triangulate |
        aiProcess_JoinIdenticalVertices |
        aiProcess_GlobalScale
    );
    if (!scene || !scene->HasMeshes()) {
        return objects;
    }
    objects.reserve(scene->mNumMeshes);
    for (unsigned int m = 0; m < scene->mNumMeshes; ++m) {
        aiMesh* mesh = scene->mMeshes[m];
        Object obj{};
        obj.ID = m;
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
                vert.normal = glm::vec4(0,0,1,0);
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
        objects.push_back(std::move(obj));
    }
    return objects;
}