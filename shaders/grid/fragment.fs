#version 460 core

in vec3 linePos;
out vec4 FragColor;

void main() {
    vec3 color = vec3(0.3);

    // Оси X и Y красные
    if (abs(linePos.x) < 0.001 || abs(linePos.y) < 0.001) {
        color = vec3(1.0, 0.0, 0.0);
    }

    FragColor = vec4(color, 1.0);
}