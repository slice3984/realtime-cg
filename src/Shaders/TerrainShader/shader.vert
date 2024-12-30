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

vec3 mod289(vec3 x) {
    return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec2 mod289(vec2 x) {
    return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec3 permute(vec3 x) {
    return mod289(((x*34.0)+1.0)*x);
}

float snoise(vec2 v)
{
    const vec4 C = vec4(0.211324865405187,  // (3.0-sqrt(3.0))/6.0
    0.366025403784439,  // 0.5*(sqrt(3.0)-1.0)
    -0.577350269189626,  // -1.0 + 2.0 * C.x
    0.024390243902439); // 1.0 / 41.0
    // First corner
    vec2 i  = floor(v + dot(v, C.yy) );
    vec2 x0 = v -   i + dot(i, C.xx);

    // Other corners
    vec2 i1;

    i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
    vec4 x12 = x0.xyxy + C.xxzz;
    x12.xy -= i1;

    // Permutations
    i = mod289(i); // Avoid truncation effects in permutation
    vec3 p = permute( permute( i.y + vec3(0.0, i1.y, 1.0 ))
                      + i.x + vec3(0.0, i1.x, 1.0 ));

    vec3 m = max(0.5 - vec3(dot(x0,x0), dot(x12.xy,x12.xy), dot(x12.zw,x12.zw)), 0.0);
    m = m*m ;
    m = m*m ;

    vec3 x = 2.0 * fract(p * C.www) - 1.0;
    vec3 h = abs(x) - 0.5;
    vec3 ox = floor(x + 0.5);
    vec3 a0 = x - ox;

    m *= 1.79284291400159 - 0.85373472095314 * ( a0*a0 + h*h );

    // Compute final noise value at P
    vec3 g;
    g.x  = a0.x  * x0.x  + h.x  * x0.y;
    g.yz = a0.yz * x12.xz + h.yz * x12.yw;
    return 130.0 * dot(m, g);
}

out float o_minHeight;
out float o_maxHeight;

void main() {
    vec3 position = data[gl_VertexID].pos;
    vec3 normal = data[gl_VertexID].normal;

    f_texCoord = position.xz;
    vec2 worldPosCam = position.xz;

    // Noise parameters
    float noiseHeight = 0.0f;
    float amplitude = 1.0f;
    float frequency = 1.0f;

    for (int i = 0; i < u_octaves; i++) {
        noiseHeight += amplitude * snoise((worldPosCam +  u_chunkOffset) / (u_scale * frequency));
        amplitude *= u_persistance;
        frequency *= u_lucunarity;
    }

    noiseHeight = u_terrainHeight * (noiseHeight + 1.0f) * 0.5f;

    o_minHeight = 0;
    o_maxHeight = u_terrainHeight;
    o_height = noiseHeight;

    vec4 worldPos = u_model * vec4(worldPosCam.x, noiseHeight, worldPosCam.y, 1.0f);
    f_worldPos = worldPos.xyz;

    gl_Position = u_projection * u_view * worldPos;
}
