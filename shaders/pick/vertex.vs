#version 460 core
layout(location=0) in vec3 aPos;

uniform mat4 model;           // сохраняем твой uniform
uniform mat4 viewProjection;  // сохраняем твой uniform

flat out uint v_InstanceID;   // передаем в фрагментный shader

void main() {
    gl_Position = viewProjection * model * vec4(aPos, 1.0);
    v_InstanceID = uint(gl_InstanceID); // используем gl_InstanceID только здесь
}