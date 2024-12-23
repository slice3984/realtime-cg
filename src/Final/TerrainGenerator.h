//
// Created by slice on 12/21/24.
//

#ifndef TERRAINGENERATOR_H
#define TERRAINGENERATOR_H
#include <vector>

#include "../GeometryUtils.h"
#include "../../Linking/include/glad/glad.h"

class TerrainGenerator {
public:
    // The generated terrains will be square
    TerrainGenerator(int gridSize, int gridSpacing)
        : m_gridSize(gridSize), m_gridSpacing(gridSpacing) {

        geometry_utils::TriangulatedPlaneMesh mesh = geometry_utils::generateTriangulatedPlaneMesh(gridSize, gridSpacing, true);
        m_VAO = geometry_utils::uploadTriangulatedPlaneMeshToGPU(mesh);
        m_vertices = mesh.vertices;
        m_indices = mesh.indices;
    }

    [[nodiscard]] RenderCall getRenderCall() const {
        return {
            m_VAO, (GLuint)m_indices.size(), GL_UNSIGNED_INT
        };
    }

    [[nodiscard]] float getHeight(const glm::vec3 &pos, float terrainHeight, int octaves, float scale, float persistance, float lucunarity) const {
        float noiseHeight = 0.0f;
        float amplitude = 1.0f;
        float frequency = 1.0f;

        for (int i = 0; i < octaves; i++) {
            const glm::vec2 xzPos = glm::vec2{pos.x, pos.z};

            noiseHeight += amplitude * SimplexNoise::snoise(xzPos / (scale * frequency));
            amplitude *= persistance;
            frequency *= lucunarity;
        }

        return terrainHeight * (noiseHeight + 1.0f) * 0.5f;
    }

private:
    int m_gridSize;
    int m_gridSpacing;
    std::vector<GLfloat> m_vertices;
    std::vector<GLuint> m_indices;
    GLuint m_VAO;
};

#endif //TERRAINGENERATOR_H
