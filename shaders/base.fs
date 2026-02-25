#version 460 core

uniform vec3 u_Color = vec3(0.7, 0.7, 0.7); // цвет линий сетки

out vec4 FragColor;

void main() {
    FragColor = vec4(u_Color, 0.2);
}