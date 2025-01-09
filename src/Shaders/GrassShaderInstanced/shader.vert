#version 430
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aTangent;
layout (location = 3) in vec2 aTexCoord;
uniform vec2 u_windowDimensions;

uniform mat4 u_view;
uniform mat4 u_projection;
uniform float u_time;
uniform int u_baseInstance;

struct InstanceData {
    vec3 pos;
    float scaling;
};

layout(std430, binding = 1) buffer InstanceBuffer {
    InstanceData instances[];
};

out vec3 f_worldPos;
out vec3 vColor;
out vec3 f_normal;
out vec2 f_texCoord;

void main() {
    InstanceData instance = instances[gl_InstanceID + u_baseInstance];
    float scalingFactor = instance.scaling;
    vec3 worldPos = vec3(scalingFactor * aPos.x, 5 * aPos.y, scalingFactor * aPos.z) + instance.pos;

    float sway = sin(worldPos.x * 0.5 + worldPos.z * 0.5 + u_time + gl_InstanceID * 0.1);
    worldPos.x += sway * 0.1;

    f_worldPos = worldPos;
    f_normal = vec3(0.0f, 1.0f, 0.0f);
    f_texCoord = aTexCoord;
    vColor = vec3(aTexCoord, 0.0f);

    gl_Position = u_projection * u_view * vec4(worldPos, 1.0f);
}

