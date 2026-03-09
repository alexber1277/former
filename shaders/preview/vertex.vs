#version 460 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_UV;

uniform mat4 model;
uniform mat4 viewProjection;

out vec3 v_Normal;
out vec3 v_WorldPos;

void main()
{
    vec4 worldPos = model * vec4(a_Position, 1.0);

    v_WorldPos = worldPos.xyz;
    v_Normal = mat3(transpose(inverse(model))) * a_Normal;

    gl_Position = viewProjection * worldPos;
}