#version 460 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 3) in mat4 a_Model;

uniform mat4 viewProjection;
uniform int instanceIndex;
flat out uint v_InstanceID;

const float outlineThickness = 1.0;

void main() {
    // только выбранный инстанс
    v_InstanceID = gl_InstanceID;

    // смещаем вершины по нормали для контура
    vec3 normalApprox = normalize(a_Normal); // или normalize(a_Position) если нормалей нет
    vec3 inflatedPos = a_Position + normalApprox * outlineThickness;

    gl_Position = viewProjection * a_Model * vec4(inflatedPos, 1.0);
}