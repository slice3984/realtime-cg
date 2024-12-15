#version 420
out vec4 oFragColor;
in vec3 vColor;
in vec3 f_normal;
in vec3 f_worldPos;
in vec2 f_texCoord;

uniform vec3 u_lightDirection;
uniform vec3 u_cameraPos;
uniform float u_ambientIntensity;
uniform float u_specularIntensity;
uniform vec2 u_tilingFactor;
uniform sampler2D u_diffuseTex;

void main() {
    vec3 color = texture(u_diffuseTex, f_texCoord * u_tilingFactor).xyz;
    vec3 lightDir = normalize(u_lightDirection);
    vec3 viewDirection = normalize(u_cameraPos - f_worldPos);
    vec3 normal = normalize(f_normal);

    float ambient = u_ambientIntensity;
    float diffuse = max(0.0f, dot(normal, lightDir) * (1.0f - ambient));

    vec3 halfWay = normalize(viewDirection + lightDir);
    float specular = max(0.0, dot(normal, halfWay));
    specular = pow(specular, u_specularIntensity);

    float lightIntensity = ambient + diffuse + specular;
    color *= lightIntensity;
    oFragColor = vec4(color, 1.0f);
}