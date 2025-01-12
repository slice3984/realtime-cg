#version 430
out vec4 oFragColor;
in vec3 vColor;
in vec3 f_normal;
in vec3 f_worldPos;
in vec2 f_texCoord;
in float f_grassHeight;

uniform vec3 u_lightDirection;
uniform vec3 u_cameraPos;
uniform float u_ambientIntensity;
uniform float u_specularIntensity;
uniform sampler2D u_diffuseTex;

void main() {
    vec4 color = texture(u_diffuseTex, f_texCoord);
    if (color.a < 0.9)
        discard;

    vec3 lightDir = normalize(u_lightDirection);
    vec3 viewDirection = normalize(u_cameraPos - f_worldPos);
    vec3 normal = normalize(f_normal);

    float ambient = 0.5f;
    float diffuse = max(0.0f, dot(normal, lightDir));

    vec3 halfWay = normalize(viewDirection + lightDir);
    float specular = max(0.0, dot(normal, halfWay));
    specular = pow(specular, 64.0f);

    float lightIntensity = ambient +
    diffuse * (1.0f - ambient) +
    specular * 0.3;

    vec3 finalColor = color.rgb;

    finalColor *= lightIntensity;
    finalColor = mix(finalColor * 0.8, finalColor, smoothstep(0.0, 0.3, lightIntensity));
    oFragColor = vec4(finalColor * color.a, color.a);
}