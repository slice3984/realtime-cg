#version 430
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aTangent;
layout (location = 3) in vec2 aTexCoord;
uniform vec2 u_windowDimensions;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;
uniform int u_baseInstance;

out vec3 f_worldPos;
out vec3 f_normal;
out vec2 f_texCoord;

struct InstanceData {
    vec3 pos;
    float scaling;
};

layout(std430, binding = 1) buffer InstanceBuffer {
    InstanceData instances[];
};

void main() {
    InstanceData instance = instances[gl_InstanceID + u_baseInstance];
    float scalingFactor = instance.scaling;
    vec3 worldPos = scalingFactor * aPos + instance.pos;

    vec3 transformedNormal = normalize(aNormal / scalingFactor);

    f_worldPos = worldPos;
    f_normal = transformedNormal;
    f_texCoord = aTexCoord;

    gl_Position = u_projection * u_view * vec4(worldPos, 1.0f);
}