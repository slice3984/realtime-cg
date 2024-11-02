#version 420

out vec4 fragColor;

uniform float u_time;
uniform vec3 u_color;

void main() {
    vec2 coord = gl_FragCoord.xy;

    vec2 normalizedCoord = coord / vec2(1280.0, 960.0);

    fragColor = vec4(u_color.r, u_color.g, u_color.b, 1.0);
}
