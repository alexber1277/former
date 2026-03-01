#version 460 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_UV;
layout(location = 3) in mat4 a_Model;

uniform mat4 viewProjection;

out vec3 v_Normal;
out vec3 v_WorldPos;

void main() {
    vec4 worldPos = a_Model * vec4(a_Position, 1.0);

    v_WorldPos = worldPos.xyz;

    // нормаль трансформируем только матрицей модели
    v_Normal = mat3(transpose(inverse(a_Model))) * a_Normal;

    gl_Position = viewProjection * worldPos;
}