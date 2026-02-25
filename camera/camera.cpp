#include "camera.h"

void Camera::CamCalculated(int width, int height){
    float aspect = height > 0 ? (float)width / height : 1.0f;
    view = glm::lookAt(position, target, up);
    projection = glm::perspective(glm::radians(fov),aspect,nearPlane,farPlane);
    viewProjection = projection * view;
}

void Camera::Orbit(float dx, float dy) {
    glm::vec3 dir = position - target;
    float radius = glm::length(dir);

    if (radius < 0.0001f)
        return;

    float yaw   = atan2(dir.y, dir.x);
    float pitch = atan2(dir.z, sqrt(dir.x * dir.x + dir.y * dir.y));

    yaw   -= dx * orbitSpeed * 0.01f;
    pitch += dy * orbitSpeed * 0.01f;

    float limit = glm::radians(89.0f);
    pitch = glm::clamp(pitch, -limit, limit);

    dir.x = radius * cos(pitch) * cos(yaw);
    dir.y = radius * cos(pitch) * sin(yaw);
    dir.z = radius * sin(pitch);

    position = target + dir;
}

void Camera::Pan(float dx, float dy) {
    glm::vec3 forward = glm::normalize(target - position);
    glm::vec3 right   = glm::normalize(glm::cross(forward, up));
    glm::vec3 camUp   = glm::normalize(glm::cross(right, forward));

    glm::vec3 move = (-right * dx + camUp * dy) * panSpeed;

    position += move;
    target   += move;
}

void Camera::Zoom(float delta) {
    glm::vec3 forward = glm::normalize(target - position);
    float distance = glm::length(target - position);

    float newDistance = distance - delta * zoomSpeed;
    newDistance = glm::max(newDistance, 0.1f);

    position = target - forward * newDistance;
}