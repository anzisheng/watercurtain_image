#ifndef SHADERS_H
#define SHADERS_H

// 顶点着色器源码 - 粒子系统核心
// 每个粒子作为独立点，支持大小变化和颜色渐变
static const char* particleVertexShader = R"(
#version 450 core

layout(location = 0) in vec3 aPosition;    // 粒子位置
layout(location = 1) in vec3 aVelocity;    // 粒子速度（用于动态计算）
layout(location = 2) in float aLife;       // 当前生命值 [0,1]
layout(location = 3) in float aInitialLife;// 初始生命值
layout(location = 4) in float aSize;       // 粒子大小
layout(location = 5) in vec3 aColor;       // 粒子颜色

uniform mat4 uModelViewProjection;
uniform float uTime;                       // 全局时间
uniform vec3 uCameraPosition;

out float vLife;
out vec3 vColor;
out float vAlpha;

void main() {
    vLife = aLife;
    
    // 生命值衰减影响透明度
    vAlpha = smoothstep(0.0, 1.0, aLife) * 0.8;
    
    // 颜色基于生命值渐变
    float lifeFactor = aLife;
    vec3 startColor = aColor;
    vec3 endColor = vec3(0.4, 0.6, 1.0);  // 浅蓝色终点
    vColor = mix(endColor, startColor, lifeFactor);
    
    // 粒子大小随生命值变化（喷出时大，落下时小）
    float sizeFactor = 0.5 + sin(aLife * 3.14159) * 0.5;
    float finalSize = aSize * (0.3 + sizeFactor * 0.7);
    
    gl_PointSize = finalSize;
    gl_Position = uModelViewProjection * vec4(aPosition, 1.0);
}
)";

// 片元着色器 - 圆形粒子渲染
// 使用圆形渐变和边缘羽化，模拟水珠效果
static const char* particleFragmentShader = R"(
#version 450 core

in float vLife;
in vec3 vColor;
in float vAlpha;

out vec4 FragColor;

uniform float uTime;

void main() {
    // 绘制圆形粒子
    vec2 coord = gl_PointCoord * 2.0 - 1.0;
    float r = length(coord);
    if (r > 1.0) discard;
    
    // 边缘羽化 - 让粒子看起来更柔和
    float alpha = (1.0 - smoothstep(0.7, 1.0, r)) * vAlpha;
    
    // 高光效果（模拟水珠反光）
    float specular = pow(1.0 - r, 2.0) * 0.5;
    vec3 finalColor = vColor + vec3(specular);
    
    // 生命末期略微发白
    if (vLife < 0.2) {
        finalColor = mix(finalColor, vec3(0.9, 0.95, 1.0), 1.0 - vLife * 5.0);
    }
    
    FragColor = vec4(finalColor, alpha);
}
)";

#endif // SHADERS_H