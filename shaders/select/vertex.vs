#version 460 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 3) in mat4 a_Model;

uniform mat4 viewProjection;
uniform int instanceIndex;
flat out uint v_InstanceID;

const float outlineThickness = 1.5;

void main() {
    v_InstanceID = gl_InstanceID;
    vec3 normalApprox = normalize(a_Normal);
    vec3 inflatedPos = a_Position + normalApprox * outlineThickness;
    gl_Position = viewProjection * a_Model * vec4(inflatedPos, 1.0);
}