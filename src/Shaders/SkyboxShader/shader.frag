#version 430
in vec3 f_texCoord;
out vec4 fragColor;
uniform samplerCube u_skybox;

void main()
{
    fragColor = texture(u_skybox, f_texCoord);
}