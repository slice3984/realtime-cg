#version 420
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec3 aNormal;
uniform vec2 u_windowDimensions;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

out vec3 f_worldPos;
out vec3 vColor;
out vec3 f_normal;

void main() {
    vec4 worldPos = u_model * vec4(aPos, 1.0f);
    f_worldPos = worldPos.xyz;
    f_normal = (u_model * vec4(aNormal, 0.0f)).xyz;
    vColor = aColor;

    gl_Position = u_projection * u_view * worldPos;
}