#version 460 core
layout(location=0) in vec3 aPos;

uniform mat4 model;
uniform mat4 viewProjection;

flat out uint v_InstanceID;

void main() {
    gl_Position = viewProjection * model * vec4(aPos, 1.0);
    v_InstanceID = uint(gl_InstanceID);
}