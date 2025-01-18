#version 430
layout (location = 0) in vec2 aPos;

struct VertexData {
    vec3 pos;
    vec3 normal;
};

layout (std430, binding = 0) buffer VertexDataBuffer {
    VertexData data[];
};

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;
uniform vec3 u_camPos;
uniform float u_time;

uniform float u_terrainHeight;
uniform float u_scale;
uniform float u_persistance;
uniform float u_lucunarity;
uniform int u_octaves;

out vec3 f_normal;
out vec3 f_worldPos;
out float f_normalizedTerrainHeight;

const float pi = 3.14159265358979323846;

void main() {
    vec2 worldPosCam = aPos + u_camPos.xz;

    float height = 0.0f;
    float dx = 0.0f;
    float dy = 0.0f;

    // Combine multiple sine waves with different frequencies
    for (int i = 0; i < 6; i++) {
        float waveFreq = 1.25f * pi / (30.0f - (i * 4.0));
        float waveAmplitude = pow(0.6f, i);
        float phaseShift = u_time * 0.5f * float(i);

        float f = waveFreq * (aPos.x + aPos.y) + phaseShift;

        height += waveAmplitude * (sin(f) + 0.5f);

        float derivative = waveAmplitude * waveFreq * cos(f);
        dx += derivative;
        dy += derivative;
    }

    vec3 gradient = vec3(dx, dy, -1.0f);
    f_normal = normalize(gradient);

    // Use the known height to determine if the waves should be cut off
    float normalizedHeight =  data[gl_VertexID].pos.y / u_terrainHeight;
    f_normalizedTerrainHeight = normalizedHeight;

    float noiseInfluence = clamp((0.1 - normalizedHeight) / 0.1, 0.0, 1.0);

    height *= noiseInfluence;

    vec4 worldPos = u_model * vec4(worldPosCam.x, abs(data[gl_VertexID].pos.y) + height, worldPosCam.y, 1.0f);
    f_worldPos = worldPos.xyz;
    gl_Position = u_projection * u_view * worldPos;
}
