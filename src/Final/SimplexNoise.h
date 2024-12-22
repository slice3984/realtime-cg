//
// Created by slice on 12/22/24.
//

#ifndef SIMPLEXNOISE_H
#define SIMPLEXNOISE_H


#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <cmath>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <cmath>

class SimplexNoise {
public:
    static glm::vec3 mod289(const glm::vec3& x) {
        return x - glm::floor(x * (1.0f / 289.0f)) * 289.0f;
    }

    static glm::vec2 mod289(const glm::vec2& x) {
        return x - glm::floor(x * (1.0f / 289.0f)) * 289.0f;
    }

    static glm::vec3 permute(const glm::vec3& x) {
        return mod289(((x * 34.0f) + 1.0f) * x);
    }

    static float snoise(const glm::vec2& v) {
        const glm::vec4 C = glm::vec4(0.211324865405187f,  // (3.0-sqrt(3.0))/6.0
                                      0.366025403784439f,  // 0.5*(sqrt(3.0)-1.0)
                                     -0.577350269189626f,  // -1.0 + 2.0 * C.x
                                      0.024390243902439f); // 1.0 / 41.0

        // First corner
        glm::vec2 i = glm::floor(v + glm::vec2(glm::dot(v, glm::vec2(C.y, C.y))));
        glm::vec2 x0 = v - i + glm::vec2(glm::dot(i, glm::vec2(C.x, C.x)));

        // Other corners
        glm::vec2 i1 = (x0.x > x0.y) ? glm::vec2(1.0f, 0.0f) : glm::vec2(0.0f, 1.0f);
        glm::vec4 x12 = glm::vec4(x0.x, x0.y, x0.x, x0.y) + glm::vec4(C.x, C.x, C.z, C.z);
        x12.x -= i1.x;
        x12.y -= i1.y;

        // Permutations
        i = mod289(i); // Avoid truncation effects in permutation
        glm::vec3 p = permute(permute(i.y + glm::vec3(0.0f, i1.y, 1.0f)) + i.x + glm::vec3(0.0f, i1.x, 1.0f));

        glm::vec3 m = glm::max(0.5f - glm::vec3(glm::dot(x0, x0), glm::dot(glm::vec2(x12.x, x12.y), glm::vec2(x12.x, x12.y)), glm::dot(glm::vec2(x12.z, x12.w), glm::vec2(x12.z, x12.w))), 0.0f);
        m = m * m;
        m = m * m;

        glm::vec3 x = 2.0f * glm::fract(p * C.w) - 1.0f;
        glm::vec3 h = glm::abs(x) - 0.5f;
        glm::vec3 ox = glm::floor(x + 0.5f);
        glm::vec3 a0 = x - ox;

        m *= 1.79284291400159f - 0.85373472095314f * (a0 * a0 + h * h);

        // Compute final noise value at P
        glm::vec3 g;
        g.x = a0.x * x0.x + h.x * x0.y;
        g.y = a0.y * x12.x + h.y * x12.y;
        g.z = a0.z * x12.z + h.z * x12.w;
        return 130.0f * glm::dot(m, g);
    }
};


#endif //SIMPLEXNOISE_H
