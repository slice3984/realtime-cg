#version 420

out vec4 oFragColor;

in vec3 vColor;
in vec3 f_normal;
in vec3 f_worldPos;
in vec2 TexCoord;

uniform vec3 u_lightDirection;
uniform vec3 u_cameraPos;
uniform float u_ambientIntensity;
uniform float u_specularIntensity;
uniform sampler2D u_texDiffuse;

void main() {
    vec4 textureColor = texture(u_texDiffuse, TexCoord);

    vec3 normal = normalize(f_normal);
    vec3 lightDir = normalize(u_lightDirection);
    vec3 viewDir = normalize(u_cameraPos - f_worldPos);

    float ambient = u_ambientIntensity;

    float diffuse = max(dot(normal, lightDir), 0.0f);

    vec3 halfDir = normalize(lightDir + viewDir);
    float specular = pow(max(dot(normal, halfDir), 0.0f), u_specularIntensity);

    float lightIntensity = ambient + diffuse + specular;

    vec4 finalColor = textureColor * lightIntensity;

    oFragColor = finalColor;
}
