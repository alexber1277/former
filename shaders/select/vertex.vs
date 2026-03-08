#version 460 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 3) in mat4 a_Model;

uniform mat4 viewProjection;
flat out uint v_InstanceID;

void main() {
    // глобальный ID инстанса (учитываем baseInstance)
    v_InstanceID = gl_InstanceID + gl_BaseInstance;

    gl_Position = viewProjection * a_Model * vec4(a_Position, 1.0);
}