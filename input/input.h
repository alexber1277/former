#pragma once
#include <GLFW/glfw3.h>
#include "../camera/camera.h"

class Input {
public:
    Input(GLFWwindow* window) {
        m_Window = window;
    };
    void Update();
    void Handlers(Camera* camera);

    double GetMouseX() const;
    double GetMouseY() const;
    double GetDeltaX() const;
    double GetDeltaY() const;
    double GetScrollDelta() const;
    float ConsumeScrollDelta();

    void SetScroll(float val) {
        m_ScrollDelta += val;
    }

    bool IsMouseDown(int button) const {
        return glfwGetMouseButton(m_Window, button) == GLFW_PRESS;
    }

    bool IsKeyDown(int key) const {
        return glfwGetKey(m_Window, key) == GLFW_PRESS;
    }

private:

    GLFWwindow* m_Window;

    double m_MouseX = 0.0;
    double m_MouseY = 0.0;
    double m_LastMouseX = 0.0;
    double m_LastMouseY = 0.0;

    double m_DeltaX = 0.0;
    double m_DeltaY = 0.0;

    double m_ScrollDelta = 0.0;

    float lastFrame = 0.0;
    float deltaTime = 0.0;

};
