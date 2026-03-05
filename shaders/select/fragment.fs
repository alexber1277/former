#version 450 core

flat in uint v_InstanceID;
uniform int instanceIndex;

out vec4 FragColor;

void main() {
    if(v_InstanceID != uint(instanceIndex))
        discard;

    if(gl_FrontFacing)
        discard;

    FragColor = vec4(1.0, 0.6, 0.0, 1.0);
}