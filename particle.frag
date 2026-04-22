#version 450 core

in vec3 vColor;
in float vAlpha;

out vec4 FragColor;

void main() {
    // 简单的方形粒子，便于调试
    FragColor = vec4(vColor, vAlpha);
}