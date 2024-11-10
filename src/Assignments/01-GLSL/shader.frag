#version 420 core
in vec2 vUv;
out vec4 fragColor;

uniform float u_time;
uniform vec3 u_color;
uniform vec2 u_windowDimensions;
uniform int u_columnCount;
uniform float u_animSpeed;

const float PI = 3.14159265358979323846;

float rectSdf(vec2 origin, vec2 p, float halfWidth, float halfHeight) {
    vec2 d = abs(p - origin) - vec2(halfWidth, halfHeight);
    return max(d.x, d.y);
}

vec3 palette(in float t) {
    vec3 green = vec3(0.0, 1.0, 0.0);
    vec3 orange = vec3(1.0, 0.5, 0.0);
    vec3 red = vec3(1.0, 0.0, 0.0);

    return (t < 0.5) ? mix(green, orange, t * 2.0) : mix(orange, red, (t - 0.5) * 2.0);
}

const int nColumns =  u_columnCount;
int amountBlocks[50];

void main() {
    vec2 uv = vUv;

    uv.x *= float(nColumns);
    const float colIdx = floor(uv.x);
    uv.x = fract(uv.x);

    const float height = 0.3 / u_columnCount; // Block height
    const float blockSpacing = 1. / u_columnCount;

    // Max amount of blocks per column
    int currBlockCount = 3;
    for (int i = 0; i < nColumns; i++) {
        amountBlocks[i] = (i < nColumns / 2) ? currBlockCount++ : currBlockCount--;
    }

    vec3 color = vec3(0.0);

    for (int i = 0; i < nColumns; i++) {
        const float cosVal = max(cos(u_time * u_animSpeed * float(i) + PI) * 0.5 + 0.5, 0.05);
        int blocksToRender = max(1, int(round(float(amountBlocks[i]) * cosVal)));

        for (int j = 0; j < blocksToRender; j++) {
            float activation = 0.0;
            if (colIdx == float(i)) {
                activation = step(0.0, -rectSdf(vec2(0.5, float(j) * blockSpacing + 0.04), uv, 0.4, height));
            }

            color += activation * palette(uv.y * 1.25);
        }
    }

    fragColor = vec4(color, 1.0);
}
