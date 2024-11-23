#version 420
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
uniform vec2 u_windowDimensions;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

out vec4 vColor;

void main() {
    gl_Position = u_projection * u_view * u_model * vec4(aPos, 1.0);
    vColor = vec4(aColor, 1.0);
}