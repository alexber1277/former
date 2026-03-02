#version 460 core

in vec3 v_Normal;
in vec3 v_WorldPos;

layout(location = 0) out vec4  FragColor;
layout(location = 1) out uvec2 outID;

uniform vec3 cameraPos;

// Материал
uniform vec4 mat_albedo;
uniform float mat_metalic;
uniform float mat_roughness;

uniform uint meshID;
flat in uint v_InstanceID;

void main() {
    vec3 N = normalize(v_Normal);
    vec3 V = normalize(cameraPos - v_WorldPos);
    vec3 L = V;

    float diff = max(dot(N, L), 0.0);

    vec3 H = normalize(L + V);
    float spec = pow(max(dot(N, H), 0.0), mix(64.0, 8.0, mat_roughness));
    spec *= mix(0.04, 1.0, mat_metalic);

    vec3 ambient = 0.1 * mat_albedo.rgb;
    vec3 diffuse = diff * mat_albedo.rgb;

    vec3 color = ambient + diffuse + spec;

    outID = uvec2(meshID + 1, v_InstanceID + 1);
    FragColor = vec4(color, mat_albedo.a);
}