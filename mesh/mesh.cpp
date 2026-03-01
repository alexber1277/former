#include "mesh.h"
#include <iostream>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

std::vector<Mesh> Mesh::LoadMeshes(const std::string& path) {
    std::vector<Mesh> objects;
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
    for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
        const aiMesh* aMesh = scene->mMeshes[i];
        Mesh mesh;
        mesh.ID = i;
        mesh.vertices.reserve(aMesh->mNumVertices);
        for (unsigned int v = 0; v < aMesh->mNumVertices; ++v) {
            MeshVertex vertex;
            vertex.position = glm::vec3(aMesh->mVertices[v].x, aMesh->mVertices[v].y, aMesh->mVertices[v].z);
            if (aMesh->HasNormals()) {
                vertex.normal = glm::vec3(aMesh->mNormals[v].x, aMesh->mNormals[v].y, aMesh->mNormals[v].z);
            } else {
                vertex.normal = glm::vec3(0.0f);
            }
            if (aMesh->HasTextureCoords(0)) {
                vertex.uv = glm::vec2(aMesh->mTextureCoords[0][v].x, aMesh->mTextureCoords[0][v].y);
            } else {
                vertex.uv = glm::vec2(0.0f);
            }
            mesh.vertices.push_back(vertex);
        }
        mesh.indices.reserve(aMesh->mNumFaces);
        for (unsigned int f = 0; f < aMesh->mNumFaces; ++f) {
            const aiFace& face = aMesh->mFaces[f];
            if (face.mNumIndices != 3) continue;
            MeshIndex idx;
            idx.i1 = face.mIndices[0];
            idx.i2 = face.mIndices[1];
            idx.i3 = face.mIndices[2];
            mesh.indices.push_back(idx);
        }
        MeshInstance instance;
        instance.meshId = mesh.ID;
        instance.position = glm::vec3(0.0f);
        instance.rotation = glm::vec3(0.0f);
        instance.scale = glm::vec3(0.1f);
        instance.UpdateModel();
        mesh.instances.push_back(instance);
        objects.push_back(mesh);
    }
    return objects;
}
