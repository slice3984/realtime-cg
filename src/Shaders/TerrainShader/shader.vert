#version 430
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

// Terrain uniforms
uniform vec2 u_chunkOffset;
uniform float u_terrainHeight;
uniform float u_scale;
uniform float u_persistance;
uniform float u_lucunarity;
uniform int u_octaves;

out float o_height;
out vec2 f_texCoord;
out vec3 f_worldPos;
out vec3 f_normal;

out float o_minHeight;
out float o_maxHeight;

void main() {
    vec3 position = data[gl_VertexID].pos;
    vec3 normal = data[gl_VertexID].normal;

    f_texCoord = position.xz;
    vec2 worldPosCam = position.xz;

    o_minHeight = 0;
    o_maxHeight = u_terrainHeight;
    o_height = position.y;

    vec4 worldPos = u_model * vec4(position, 1.0f);
    f_worldPos = worldPos.xyz;
    f_normal = normal;

    gl_Position = u_projection * u_view * worldPos;
}