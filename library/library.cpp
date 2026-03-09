#include "library.h"
#include <filesystem>
#include <iostream>

#include "../camera/camera.h"
#include "../mesh/mesh.h"

void Library::InitPreviewBuffer() {
    glGenFramebuffers(1, &PreviewFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, PreviewFBO);
    glGenTextures(1, &PreviewTex);
    glBindTexture(GL_TEXTURE_2D, PreviewTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, SizePreview, SizePreview, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, PreviewTex, 0);
    glGenRenderbuffers(1, &PreviewDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, PreviewDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SizePreview, SizePreview);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER,GL_DEPTH_STENCIL_ATTACHMENT,GL_RENDERBUFFER,PreviewDepth);
    GLenum drawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, drawBuffers);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "Preview FBO ERROR\n";
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Library::LoadElementsLib() {
    assets.clear();
    std::string meshFolder  = "assets/";
    std::string thumbFolder = "assets/.thumbs/";
    std::filesystem::create_directories(thumbFolder);
    for (auto& entry : std::filesystem::directory_iterator(meshFolder)) {
        if (!entry.is_regular_file()) {
            continue;
        }
        auto ext = entry.path().extension().string();
        if (ext != ".obj" && ext != ".fbx") {
            continue;
        }
        std::string path  = entry.path().string();
        std::string name  = entry.path().stem().string();
        std::string thumb = thumbFolder + name + ".png";
        if (!std::filesystem::exists(thumb)) {
            GeneratePreview(path, thumb);
        }
        LibraryAsset asset;
        asset.name = name;
        asset.path = path;
        asset.thumbnail = thumb;
        asset.thumbnailTex = LoadTexture(thumb);
        assets.push_back(asset);
    }
}

void Library::GeneratePreview(const std::string& meshPath, const std::string& thumbPath) {
    auto meshes = Mesh::LoadMeshes(meshPath, 0);
    if (meshes.empty()) return;
    Mesh& mesh = meshes[0];
    GLint oldViewport[4];
    glGetIntegerv(GL_VIEWPORT, oldViewport);
    glBindFramebuffer(GL_FRAMEBUFFER, PreviewFBO);
    glViewport(0, 0, SizePreview, SizePreview);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.18f, 0.18f, 0.18f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glm::vec3 min(FLT_MAX);
    glm::vec3 max(-FLT_MAX);
    for (auto& v : mesh.vertices) {
        min = glm::min(min, v.position);
        max = glm::max(max, v.position);
    }
    glm::vec3 center = (min + max) * 0.5f;
    float size = glm::length(max - min);
    if (size < 0.001f) size = 1.0f;
    Camera previewCam;
    glm::vec3 objDir = glm::normalize(glm::vec3(3, 3, 3));
    float distance = (size / (2.0f * tan(glm::radians(40.0f * 0.5f)))) * 1.1;
    previewCam.position = center + objDir * distance;
    previewCam.target   = center;
    glm::vec3 worldUp(0,0,1);
    if (glm::abs(glm::dot(objDir, worldUp)) > 0.99f) {
        worldUp = glm::vec3(0,1,0);
    }
    previewCam.up = -worldUp;
    previewCam.aspect   = 1.0f;
    previewCam.fov      = 40.0f;
    previewCam.nearPlane = distance * 0.01f;
    previewCam.farPlane  = distance * 10.0f;
    previewCam.CamCalculated(SizePreview,SizePreview);
    MeshInstance tempInstance;
    tempInstance.model = glm::translate(glm::mat4(1.0f), -center);
    tempInstance.selected = 0.0f;
    mesh.instances.clear();
    mesh.instances.push_back(tempInstance);
    mesh.UpdateInstances();
    GLuint shader = PreviewShader;
    glUseProgram(shader);
    GLuint locViewProj = glGetUniformLocation(shader, "viewProjection");
    glUniformMatrix4fv(locViewProj, 1, GL_FALSE, &previewCam.viewProjection[0][0]);
    GLuint locCam = glGetUniformLocation(shader, "cameraPos");
    glUniform3fv(locCam, 1, glm::value_ptr(previewCam.position));
    GLuint locAlbedo = glGetUniformLocation(shader, "mat_albedo");
    glUniform4fv(locAlbedo, 1, glm::value_ptr(mesh.MatAlbedoVal));
    GLuint locRough = glGetUniformLocation(shader, "mat_roughness");
    glUniform1f(locRough, mesh.MatRoughnesVal);
    GLuint locMetal = glGetUniformLocation(shader, "mat_metalic");
    glUniform1f(locMetal, mesh.MatMetalicVal);
    mesh.Draw(shader, locViewProj, &previewCam);
    std::vector<unsigned char> pixels(SizePreview*SizePreview*4);
    glReadPixels(0,0,SizePreview,SizePreview,GL_RGBA,GL_UNSIGNED_BYTE,pixels.data());
    stbi_write_png(thumbPath.c_str(),SizePreview,SizePreview,4,pixels.data(),SizePreview*4);
    glBindFramebuffer(GL_FRAMEBUFFER,0);
    glViewport(oldViewport[0], oldViewport[1], oldViewport[2], oldViewport[3]);
}

GLuint Library::LoadTexture(const std::string& path) {
    int w, h, c;
    unsigned char* data = stbi_load(path.c_str(), &w, &h, &c, 4);
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,w,h,0,GL_RGBA,GL_UNSIGNED_BYTE,data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(data);
    return tex;
}