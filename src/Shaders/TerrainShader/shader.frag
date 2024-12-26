#version 420

in float o_height;
in float o_minHeight;
in float o_maxHeight;
in vec2 f_texCoord;

out vec4 o_fragColor;

uniform sampler2D u_texDiffuse;
void main() {
    float normalizedHeight = (o_height - o_minHeight) / (o_maxHeight - o_minHeight);

    if (normalizedHeight < 0.2) {
        discard;
        //o_fragColor = vec4(1.0f, 1.0f, 0.0f, 1.0f);
    } else if (normalizedHeight < 0.3) {
        o_fragColor = texture(u_texDiffuse, f_texCoord * 20.0);
    } else if (normalizedHeight < 0.6) {
        o_fragColor = vec4(0.0, 1.0, 0.0, 1.0);
    } else if (normalizedHeight < 0.8) {
        o_fragColor = vec4(0.3, 0.3, 0.3, 1.0);
    } else {
        o_fragColor = vec4(1.0, 1.0, 1.0, 1.0);
    }
}
