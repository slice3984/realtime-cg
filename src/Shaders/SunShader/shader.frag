#version 430
out vec4 oFragColor;
in vec2 f_texCoord;

uniform sampler2D u_diffuseTex;

void main() {
    vec3 color = texture(u_diffuseTex, f_texCoord).xyz;
    oFragColor = vec4(color, 1.0f);
}
