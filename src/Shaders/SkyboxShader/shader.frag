#version 430
in vec3 f_texCoord;
out vec4 fragColor;
uniform samplerCube u_skybox;

void main()
{
    fragColor = mix(texture(u_skybox, f_texCoord), vec4(0.0f, 0.0f, 0.0f, 1.0f), 0.6);
}