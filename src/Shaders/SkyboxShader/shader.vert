#version 430
layout (location = 0) in vec3 aPos;

uniform mat4 u_view;
uniform mat4 u_projection;

out vec3 f_texCoord;

void main() {
    f_texCoord = aPos;
    vec4 ndcCoord = u_projection * u_view * vec4(aPos, 0.0f);
    gl_Position = ndcCoord.xyww;
}