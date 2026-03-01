#version 460 core

in vec2 uv;
out vec4 FragColor;

layout(binding = 0) uniform sampler2D screenTex;

void main(){
    vec3 color = texture(screenTex, uv).rgb;
    FragColor = vec4(color, 1.0);
}