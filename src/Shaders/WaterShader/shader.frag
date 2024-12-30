#version 430

in vec3 f_normal;
in vec3 f_worldPos;
out vec4 o_fragColor;
in float f_normalizedTerrainHeight;

uniform vec3 u_camPos;
uniform float u_ambientIntensity;
uniform float u_specularIntensity;
uniform vec3 u_lightDirection;

uniform samplerCube u_skybox;

void main() {
    if (f_normalizedTerrainHeight > 0.15) {
        discard;
    }

    vec3 lightDir = normalize(u_lightDirection);
    vec3 viewDirection = normalize(u_camPos - f_worldPos);
    vec3 normal = normalize(f_normal);

    float diffuse = max(0.0f, dot(normal, lightDir));

    vec3 halfWay = normalize(viewDirection + lightDir);
    float specular = max(0.0, dot(normal, halfWay));
    specular = pow(specular, u_specularIntensity);

    float ambient = u_ambientIntensity ;

    float lightIntensity = ambient + diffuse + specular;

    vec3 reflectedDir = reflect(viewDirection, normal);

    vec4 skyColor = texture(u_skybox, reflectedDir);

    float reflectionStrength = 0.5f;
    vec4 reflectionColor = skyColor * reflectionStrength;

    vec4 finalColor = mix(vec4(lightIntensity, lightIntensity, lightIntensity, 1.0), reflectionColor, reflectionStrength);

    finalColor.rgb *= vec3(0.6, 0.2, 0.0);

    o_fragColor = finalColor;
}
