#pragma once
#include <string>
#include <vector>
#include "../gl/glincludes.h"
#include "../camera/camera.h"

struct MeshIndex;
struct MeshVertex;
struct MeshInstance;

class Mesh {
    public:

        // original ID
        uint32_t ID;

        // loc material
        GLuint MatAlbedo;
        GLuint MatRoughnes;
        GLuint MatMetalic;

        // val materal
        glm::vec4 MatAlbedoVal = glm::vec4(0.75,0.75,0.75,1.0);
        float MatRoughnesVal = 0.9;
        float MatMetalicVal = 0.2;

        // binding GPU data
        GLuint VBO;
        GLuint VAO;
        GLuint EBO;

        // binding GPU data
        GLuint InstanceVBO;

        // camera position
        GLuint CameraPos;

        // max instance
        GLsizei MaxInstance = 10000;

        // load mesh default
        static std::vector<Mesh> LoadMeshes(const std::string& path);

        // mesh data
        std::vector<MeshVertex> vertices;
        std::vector<MeshIndex> indices;

        // instance list
        std::vector<MeshInstance> instances;

        // gpu upload
        void UploadToGPU();

        // update all instances
        void UpdateInstances();

        void UpdateInstance(size_t index);

        // set camera position loc
        void SetCamPos(GLuint shader);

        // attach material
        void AttachMaterial(GLuint shader);

        // draw instances
        void Draw(GLuint shader, GLint viewProjLoc, Camera* camera);

        // add instance
        void AddInstance(const glm::vec3& position,const glm::vec3& rotation = glm::vec3(0.0f),const glm::vec3& scale = glm::vec3(0.1f));

};

struct MeshInstance {

    uint32_t meshId;
    glm::mat4 model;

    glm::vec3 position{0.0f,0.0f,0.0f};
    glm::vec3 rotation{0.0f,0.0f,0.0f};;
    glm::vec3 scale{1.0f,1.0f,1.0f};;

    void UpdateModel() {
        glm::mat4 t = glm::translate(glm::mat4(1.0f), position);
        glm::mat4 rx = glm::rotate(glm::mat4(1.0f), rotation.x, glm::vec3(1,0,0));
        glm::mat4 ry = glm::rotate(glm::mat4(1.0f), rotation.y, glm::vec3(0,1,0));
        glm::mat4 rz = glm::rotate(glm::mat4(1.0f), rotation.z, glm::vec3(0,0,1));
        glm::mat4 r = rz * ry * rx; // порядок ZYX
        glm::mat4 s = glm::scale(glm::mat4(1.0f), scale);
        model = t * r * s;
    }

};

struct MeshVertex {

    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;

};

struct MeshIndex {

    uint32_t i1, i2, i3;

};