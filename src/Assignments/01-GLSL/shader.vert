#version 420
layout (location = 0) in vec3 aPos;

out vec2 vUv;

void main()
{
    gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
    vUv = aPos.xy * 0.5 + 0.5;
}