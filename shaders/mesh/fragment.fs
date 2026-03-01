#version 460 core

in vec3 v_Normal;
in vec3 v_WorldPos;
out vec4 FragColor;
uniform vec3 cameraPos;

void main() {
    vec3 N = normalize(v_Normal);
    vec3 L = normalize(cameraPos - v_WorldPos);

    float diff = max(dot(N, L), 0.0);

    vec3 baseColor = vec3(0.8, 0.8, 0.8);

    vec3 color = baseColor * (0.1 + diff * 0.9);

    FragColor = vec4(color, 1.0);
}