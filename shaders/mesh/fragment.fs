#version 460 core

in vec3 v_Normal;
in vec3 v_WorldPos;
out vec4 FragColor;

uniform vec3 cameraPos;

// Материал
uniform vec4 mat_albedo;
uniform float mat_metalic;
uniform float mat_roughness;

void main() {
    vec3 N = normalize(v_Normal);
    vec3 V = normalize(cameraPos - v_WorldPos);
    vec3 L = V; // свет от камеры

    // Lambert diffuse
    float diff = max(dot(N, L), 0.0);

    // Простое specular отражение по Blinn-Phong
    vec3 H = normalize(L + V);
    float spec = pow(max(dot(N, H), 0.0), mix(64.0, 8.0, mat_roughness));
    spec *= mix(0.04, 1.0, mat_metalic);

    // Ambient + diffuse
    vec3 ambient = 0.1 * mat_albedo.rgb;
    vec3 diffuse = diff * mat_albedo.rgb;

    vec3 color = ambient + diffuse + spec;

    FragColor = vec4(color, mat_albedo.a);
}