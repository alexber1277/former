#include "input.h"

#include <iostream>
#include <ostream>

void Input::Update() {
    m_LastMouseX = m_MouseX;
    m_LastMouseY = m_MouseY;
    glfwGetCursorPos(m_Window, &m_MouseX, &m_MouseY);
    m_DeltaX = m_MouseX - m_LastMouseX;
    m_DeltaY = m_MouseY - m_LastMouseY;
    double currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
}

double Input::GetMouseX() const { return m_MouseX; }
double Input::GetMouseY() const { return m_MouseY; }
double Input::GetDeltaX() const { return m_DeltaX; }
double Input::GetDeltaY() const { return m_DeltaY; }
double Input::GetScrollDelta() const { return m_ScrollDelta; }

float Input::ConsumeScrollDelta() {
    float value = (float)m_ScrollDelta;
    m_ScrollDelta = 0.0;
    return value;
}

void Input::Handlers(Camera* camera) {
    if (IsMouseDown(GLFW_MOUSE_BUTTON_MIDDLE)) {
        if (IsKeyDown(GLFW_KEY_LEFT_ALT)) {
            camera->Orbit((float)GetDeltaX(),(float)GetDeltaY());
        } else {
            camera->Pan((float)GetDeltaX(),(float)GetDeltaY());
        }
    }
    float scroll = ConsumeScrollDelta();
    if (scroll != 0.0f) {
        camera->Zoom(scroll);
    }

}