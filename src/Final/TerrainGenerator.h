//
// Created by slice on 12/21/24.
//

#ifndef TERRAINGENERATOR_H
#define TERRAINGENERATOR_H
#include <vector>
#include "../../Linking/include/glad/glad.h"

class TerrainGenerator {
public:
    // The generated terrains will be square
    TerrainGenerator(int gridSize, int gridSpacing)
        : m_gridSize(gridSize), m_gridSpacing(gridSpacing) {
        generateVertices();
        generateIndices();
        uploadToGPU();
    }

    [[nodiscard]] RenderCall getRenderCall() const {
        return {
            m_VAO, (GLuint)m_indices.size(), GL_UNSIGNED_INT
        };
    }

    [[nodiscard]] float getHeight(const glm::vec3 &pos, int octaves, float scale, float persistance, float lucunarity) const {
        float noiseHeight = 0.0f;
        float amplitude = 1.0f;
        float frequency = 1.0f;

        for (int i = 0; i < octaves; i++) {
            const glm::vec2 xzPos = glm::vec2{pos.x, pos.z};

            noiseHeight += amplitude * SimplexNoise::snoise(xzPos / (scale * frequency));
            amplitude *= persistance;
            frequency *= lucunarity;
        }

        return 15.0f * (noiseHeight + 1.0f) * 0.5f;
    }

private:
    int m_gridSize;
    int m_gridSpacing;
    std::vector<GLfloat> m_vertices;
    std::vector<GLuint> m_indices;
    GLuint m_VAO;

    // Generates all the vertices required for the grid
    // We do not need y (height) this will be generated with a noise function
    void generateVertices() {
        // Calculate the offset to center the grid
        float offset = (m_gridSize - 1) * m_gridSpacing / 2.0f;

        for (int row = 0; row < m_gridSize; row++) {
            for (int column = 0; column < m_gridSize; column++) {
                // Offset each vertex by the calculated amount
                float x = column * m_gridSpacing - offset;
                float z = row * m_gridSpacing - offset;

                m_vertices.push_back(x);
                m_vertices.push_back(z);
            }
        }
    }

    // Triangulate the base grid
    void generateIndices() {
        for (int row = 0; row < m_gridSize - 1; row++) {
            for (int column = 0; column < m_gridSize - 1; column++) {
                GLuint topLeft = row * m_gridSize + column;
                GLuint bottomLeft = (row + 1) * m_gridSize + column;
                GLuint topRight = topLeft + 1;
                GLuint bottomRight = bottomLeft + 1;

                // First triangle TL -> BL -> BR
                m_indices.push_back(topLeft);
                m_indices.push_back(bottomLeft);
                m_indices.push_back(bottomRight);

                // Second triangle TL -> BR -> TR
                m_indices.push_back(topLeft);
                m_indices.push_back(bottomRight);
                m_indices.push_back(topRight);
            }
        }
    }

    void uploadToGPU() {
        GLuint VBO, EBO;

        // VAO
        glGenVertexArrays(1, &m_VAO);
        glBindVertexArray(m_VAO);

        // VBO
        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * (2 * sizeof(GLfloat)), m_vertices.data(), GL_STATIC_DRAW);

        // EBO
        glGenBuffers(1, &EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(GLuint), m_indices.data(), GL_STATIC_DRAW);

        // Vertex attribs
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void*)nullptr);
        glEnableVertexAttribArray(0);

        glBindVertexArray(0);
    }
};

#endif //TERRAINGENERATOR_H
