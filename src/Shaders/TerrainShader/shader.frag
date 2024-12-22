#version 420

in float o_height;
in float o_minHeight;
in float o_maxHeight;

out vec4 o_fragColor;
void main() {
    float normalizedHeight = (o_height - o_minHeight) / (o_maxHeight - o_minHeight);

    if (normalizedHeight < 0.2) {
        o_fragColor = vec4(0.0, 0.0, 0.8, 1.0);
    } else if (normalizedHeight < 0.4) {
        o_fragColor = vec4(0.8, 0.5, 0.2, 1.0);
    } else if (normalizedHeight < 0.6) {
        o_fragColor = vec4(0.0, 1.0, 0.0, 1.0);
    } else if (normalizedHeight < 0.8) {
        o_fragColor = vec4(0.3, 0.3, 0.3, 1.0);
    } else {
        o_fragColor = vec4(1.0, 1.0, 1.0, 1.0);
    }
}
