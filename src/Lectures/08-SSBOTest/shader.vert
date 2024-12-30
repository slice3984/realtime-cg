#version 430
struct VertexData {
    vec3 pos;
    vec3 normal;
};

layout (std430, binding = 0) buffer VertexDataBuffer {
    VertexData data[];
}

out vec3 frag_normal;

void main() {
    vec3 position = data[gl_VertexID].pos;
    vec3 normal = data[gl_VertexID].normal;

    gl_Position = vec4(position, 1.0);
    frag_normal = normalize(normal);
}
