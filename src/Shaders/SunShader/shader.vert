#version 430
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

const float u_sunSize = 0.05;

uniform mat4 u_view;
uniform mat4 u_projection;
uniform vec3 u_sunPosition;
out vec3 f_worldPos;
out vec3 vColor;
out vec3 f_normal;
out vec2 f_texCoord;

void main() {
    vec4 sunWorldPos = vec4(u_sunPosition, 1.0f);
    vec4 viewSunPos = u_view * sunWorldPos;
    vec4 clipSunPos = u_projection * viewSunPos;

    clipSunPos.xy += aPos.xy * u_sunSize * clipSunPos.w;
    f_texCoord = aTexCoord;

    gl_Position = clipSunPos;
}
