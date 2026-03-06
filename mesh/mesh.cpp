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
        mesh.instances.push_back(instance);
        mesh.UploadToGPU();
        mesh.UpdateInstances();
        objects.push_back(mesh);
    }
    return objects;
}

void Mesh::SetCamPos(GLuint shader) {
    CameraPos = glGetUniformLocation(shader, "cameraPos");
}

void Mesh::DrawInstance(int instanceIndex, GLuint shader, GLint viewProjLoc, Camera* camera) {
    if (instanceIndex < 0 || instanceIndex >= instances.size()) {
        return;
    }
    glUseProgram(shader);
    glUniformMatrix4fv(viewProjLoc, 1, GL_FALSE, &camera->viewProjection[0][0]);
    glUniform3fv(CameraPos, 1, glm::value_ptr(camera->position));
    glUniform4fv(MatAlbedo, 1, &MatAlbedoVal[0]);
    glUniform1f(MatRoughnes, MatRoughnesVal);
    glUniform1f(MatMetalic, MatMetalicVal);
    GLint instanceIndexLoc = glGetUniformLocation(shader, "instanceIndex");
    glUniform1i(instanceIndexLoc, instanceIndex);
    glBindVertexArray(VAO);
    glDrawElementsInstanced(GL_TRIANGLES, indices.size() * 3, GL_UNSIGNED_INT, 0, instances.size());
    glBindVertexArray(0);
}

void Mesh::AttachMaterial(GLuint shader) {
    MatAlbedo = glGetUniformLocation(shader, "mat_albedo");
    MatRoughnes = glGetUniformLocation(shader, "mat_roughness");
    MatMetalic = glGetUniformLocation(shader, "mat_metalic");
}

void Mesh::UploadToGPU() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glGenBuffers(1, &InstanceVBO);
    glBindVertexArray(VAO);
    // ---- Vertex buffer ----
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER,vertices.size() * sizeof(MeshVertex),vertices.data(),GL_STATIC_DRAW);
    // ---- Index buffer ----
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    std::vector<uint32_t> flatIndices;
    flatIndices.reserve(indices.size() * 3);
    for (auto& tri : indices) {
        flatIndices.push_back(tri.i1);
        flatIndices.push_back(tri.i2);
        flatIndices.push_back(tri.i3);
    }
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, flatIndices.size() * sizeof(uint32_t), flatIndices.data(), GL_STATIC_DRAW);
    // ---- Vertex layout ----
    GLsizei stride = sizeof(MeshVertex);
    // position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(MeshVertex, position));
    glEnableVertexAttribArray(0);
    // normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(MeshVertex, normal)); glEnableVertexAttribArray(1);
    // uv
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(MeshVertex, uv));
    glEnableVertexAttribArray(2);
    // ---- Instance buffer (empty for now) ----
    glBindBuffer(GL_ARRAY_BUFFER, InstanceVBO);
    glBufferData(GL_ARRAY_BUFFER, MaxInstance * sizeof(MeshInstanceGPU),nullptr,GL_DYNAMIC_DRAW);
    // mat4 = 4 vec4
    std::size_t vec4Size = sizeof(glm::vec4);
    for (int i = 0; i < 4; ++i) {
        glVertexAttribPointer(3 + i,4,GL_FLOAT,GL_FALSE,sizeof(MeshInstanceGPU),(void*)(i * vec4Size));
        glEnableVertexAttribArray(3 + i);
        glVertexAttribDivisor(3 + i, 1);
    }

    // selected instance
    glVertexAttribPointer(7,1,GL_FLOAT,GL_FALSE,sizeof(MeshInstanceGPU),(void*)offsetof(MeshInstanceGPU, selected));
    glEnableVertexAttribArray(7);
    glVertexAttribDivisor(7, 1);

    glBindVertexArray(0);
}

void Mesh::UpdateInstances() {
    if (instances.empty()) { return; }
    std::vector<MeshInstanceGPU> models(instances.size());
    for (size_t i = 0; i < instances.size(); i++) {
        instances[i].UpdateModel();
        models[i].model = instances[i].model;
        models[i].selected = instances[i].selected;
    }
    glBindBuffer(GL_ARRAY_BUFFER, InstanceVBO);
    glBufferSubData(GL_ARRAY_BUFFER,0,models.size() * sizeof(MeshInstanceGPU),models.data());
}

void Mesh::UpdateInstance(size_t index) {
    if (index >= instances.size()){ return; }
    instances[index].UpdateModel();
    MeshInstanceGPU instGPU;
    instGPU.model = instances[index].model;
    instGPU.selected = instances[index].selected;
    glBindBuffer(GL_ARRAY_BUFFER, InstanceVBO);
    glBufferSubData(
        GL_ARRAY_BUFFER,
        index * sizeof(MeshInstanceGPU),
        sizeof(MeshInstanceGPU),
        &instGPU
    );
}

void Mesh::AddInstance(const glm::vec3& position,const glm::vec3& rotation,const glm::vec3& scale) {
    MeshInstance instance;
    instance.position = position;
    instance.rotation = rotation;
    instance.scale    = scale;
    instances.push_back(instance);
}

void Mesh::Draw(GLuint shader, GLint viewProjLoc, Camera* camera) {
    glUseProgram(shader);
    glUniformMatrix4fv(viewProjLoc, 1, GL_FALSE, &camera->viewProjection[0][0]);
    glUniform3fv(CameraPos, 1, glm::value_ptr(camera->position));
    glUniform4fv(MatAlbedo, 1, &MatAlbedoVal[0]);
    glUniform1f(MatRoughnes, MatRoughnesVal);
    glUniform1f(MatMetalic, MatMetalicVal);
    glBindVertexArray(VAO);
    glDrawElementsInstanced(GL_TRIANGLES,indices.size() * 3,GL_UNSIGNED_INT,0,instances.size());
    glBindVertexArray(0);
}