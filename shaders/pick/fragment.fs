#version 460 core
layout(location=0) out uvec2 outID;

uniform uint meshID;          // сохраняем твой uniform
flat in uint v_InstanceID;    // приходит из vertex shader

void main() {
    outID = uvec2(meshID + 1u, v_InstanceID + 1u); // +1 чтобы 0 = "ничего"
}