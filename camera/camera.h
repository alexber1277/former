#pragma once
#include "../gl/glincludes.h"

class Camera {
public:
    glm::vec3 position = {100.0f, 100.0f, 100.0f};
    glm::vec3 target   = {0.0f, 0.0f, 0.0f};
    glm::vec3 up       = {0.0f, 0.0f, 1.0f};

    glm::mat4 view;
    glm::mat4 projection;
    glm::mat4 viewProjection;

    float fov = 60.0f;

    float nearPlane = 0.5f;
    float farPlane  = 2000.0f;

    float orbitSpeed = 0.5f;
    float panSpeed   = 0.1f;
    float zoomSpeed  = 5.0f;

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
