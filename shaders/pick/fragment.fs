#version 460 core
layout(location=0) out uvec2 outID;

uniform uint meshID;
flat in uint v_InstanceID;

void main() {
    outID = uvec2(meshID + 1u, v_InstanceID + 1u);
}