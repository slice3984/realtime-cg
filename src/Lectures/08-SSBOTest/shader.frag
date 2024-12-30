#version 430

in vec3 frag_normal;

out vec4 FragColor;

void main() {
    vec3 lightDir = normalize(vec3(0.0, 0.0, 1.0));
    float diff = max(dot(frag_normal, lightDir), 0.0);

    FragColor = vec4(diff, diff, diff, 1.0);
}