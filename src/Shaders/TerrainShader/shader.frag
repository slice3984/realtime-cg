#version 430

in float o_height;
in float o_minHeight;
in float o_maxHeight;
in vec2 f_texCoord;
in vec3 f_worldPos;
in vec3 f_normal;

out vec4 o_fragColor;

uniform sampler2D u_texLayerOne;
uniform sampler2D u_texLayerTwo;

// Lighting
uniform vec3 u_lightDirection;
uniform vec3 u_cameraPos;
uniform float u_ambientIntensity;
uniform float u_specularIntensity;

// Terrain
const float waterLevel = 0.1;
const float lowLevel = 0.35;
const float highLevel = 1.0;
const float blendRange = 0.3;

void main() {
    // Normalize terrain height
    float normalizedHeight = (o_height - o_minHeight) / (o_maxHeight - o_minHeight);

    // Discard fragments below water level
    if (normalizedHeight < waterLevel) {
        discard;
    }

    // Sample textures
    vec3 layerOneColor = texture(u_texLayerOne, f_texCoord / 32.0).xyz;
    vec3 layerTwoColor = texture(u_texLayerTwo, f_texCoord / 32.0).xyz;

    // Add a tint to the second texture layer
    vec3 deadGrassTint = vec3(0.45, 0.37, 0.25);
    layerTwoColor = mix(layerTwoColor, deadGrassTint, 0.1);

    // Blend the two texture layers based on terrain height
    float blendFactor = smoothstep(lowLevel - blendRange, lowLevel + blendRange, normalizedHeight) *
    (1.0 - smoothstep(highLevel - blendRange, highLevel + blendRange, normalizedHeight));
    vec3 color = mix(layerOneColor, layerTwoColor, blendFactor);

    // Lighting calculations
    vec3 normal = normalize(f_normal);
    vec3 lightDir = normalize(u_lightDirection);
    vec3 viewDir = normalize(u_cameraPos - f_worldPos);

    // Ambient lighting
    float ambient = u_ambientIntensity;

    // Diffuse lighting
    float diffuse = max(dot(normal, lightDir), 0.0);

    // Specular lighting
    vec3 halfWay = normalize(viewDir + lightDir);
    float specular = pow(max(dot(normal, halfWay), 0.0), u_specularIntensity);

    // Combine light components
    float lightIntensity = ambient + diffuse + specular;
    lightIntensity = clamp(lightIntensity, 0.0, 1.0);

    // Apply lighting to color
    color *= lightIntensity;

    o_fragColor = vec4(color, 1.0f);
    color = 0.5 * normalize(f_normal) + 0.5;
    float f = length(vec2(normal.x, normal.z));
   // o_fragColor = vec4(f,f,f,1.0);
}
