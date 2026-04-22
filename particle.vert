#version 450 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aVelocity;
layout(location = 2) in float aLife;
layout(location = 3) in float aInitialLife;
layout(location = 4) in float aSize;
layout(location = 5) in vec3 aColor;

uniform mat4 uModelViewProjection;

out vec3 vColor;
out float vAlpha;

void main() {
    vColor = aColor;
    vAlpha = 0.8;
    
    gl_PointSize = 10.0;  // 固定大小，便于调试
    gl_Position = uModelViewProjection * vec4(aPosition, 1.0);
}