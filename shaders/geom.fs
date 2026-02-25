#version 460 core

in vec3 v_Normal;
in vec3 v_FragPos;

uniform vec3 u_CamPos;
uniform vec3 u_BaseColor = vec3(0.8, 0.3, 0.2);
uniform float u_Metallic = 0.0;
uniform float u_Roughness = 0.5;

uniform float u_FadeNear = 0.5;
uniform float u_FadeFar  = 1000.0;

layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 NormalDepth;

void main() {

    vec3 N = normalize(v_Normal);
    vec3 V = normalize(u_CamPos - v_FragPos);
    vec3 L = V;
    vec3 H = normalize(L + V);

    vec3 F0 = mix(vec3(0.04), u_BaseColor, u_Metallic);
    float NdotH = max(dot(N, H), 0.0);
    vec3 F = F0 + (1.0 - F0) * pow(1.0 - NdotH, 5.0);

    float NdotL = max(dot(N, L), 0.0);
    vec3 diffuse = (1.0 - u_Metallic) * u_BaseColor / 3.14159265;
    vec3 ambient = 0.3 * u_BaseColor;

    vec3 color = ambient + diffuse * NdotL + F * NdotL;

    float depth = length(v_FragPos - u_CamPos);

    float alpha = clamp((u_FadeFar - depth) / (u_FadeFar - u_FadeNear), 0.0, 1.0);

    depth = length(v_FragPos - u_CamPos);
    float normalizedDepth = depth / u_FadeFar;

    FragColor   = vec4(color, 1.0);

    vec3 N_rgb = normalize(v_Normal) * 0.5 + 0.5;
    NormalDepth = vec4(N_rgb, normalizedDepth);
}