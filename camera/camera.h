#pragma once
#include "../gl/glincludes.h"

class Camera {
public:
    glm::vec3 position = {-130.0f, -130.0f, 90.0f};
    glm::vec3 target   = {0.0f, 0.0f, 0.0f};
    glm::vec3 up       = {0.0f, 0.0f, 1.0f};

    glm::mat4 view;
    glm::mat4 projection;
    glm::mat4 viewProjection;

    float fov = 60.0f;
    float aspect = 0.0f;

    float nearPlane = 0.01f;
    float farPlane  = 1000.0f;

    float orbitSpeed = 0.5f;
    float panSpeed   = 0.5f;
    float zoomSpeed  = 0.5f;

    void Orbit(float dx, float dy);
    void Pan(float dx, float dy);
    void Zoom(float delta);

    glm::mat4 GetView() const {
        return glm::lookAt(position, target, up);
    }

    glm::mat4 GetProjection(float aspect) const {
        return glm::perspective(glm::radians(fov), aspect, nearPlane, farPlane);
    }

    void CamCalculated(int width, int height);

};
