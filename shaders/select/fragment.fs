#version 450 core

flat in uint v_InstanceID;
uniform int instanceIndex;

out vec4 FragColor;

void main() {
    if(v_InstanceID != uint(instanceIndex))
        discard;

    vec3 color = vec3(1.0, 1.0, 1.0);
    color *= 0.1;
    color = min(color, 1.0);
    FragColor = vec4(color, 1.0);
}