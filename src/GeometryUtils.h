//
// Created by slice on 12/22/24.
//

#ifndef GEOMETRYUTILS_H
#define GEOMETRYUTILS_H

#include <vector>
#include <glad/glad.h>

namespace geometry_utils {
    struct TriangulatedPlaneMesh {
        int gridSize;
        int gridSpacing;
        std::vector<GLfloat> vertices;
        std::vector<GLuint> indices;
    };

    TriangulatedPlaneMesh generateTriangulatedPlaneMesh(const int size, const int spacing, const bool center) {
        std::vector<GLfloat> vertices;
        std::vector<GLuint> indices;
        std::vector<GLfloat> texCoords;

        // Generate vertices
        float offset = center ? (size - 1) * spacing / 2.0f : 0.0f;

        for (int row = 0; row < size; row++) {
            for (int column = 0; column < size; column++) {
                // Offset each vertex by the calculated amount
                float x = column * spacing - offset;
                float z = row * spacing - offset;

                vertices.push_back(x);
                vertices.push_back(z);

                vertices.push_back((x + offset) / size);
                vertices.push_back((z + offset) / size);
            }
        }

        // Triangulate base grid
        for (int row = 0; row < size - 1; row++) {
            for (int column = 0; column < size - 1; column++) {
                GLuint topLeft = row * size + column;
                GLuint bottomLeft = (row + 1) * size + column;
                GLuint topRight = topLeft + 1;
                GLuint bottomRight = bottomLeft + 1;

                // First triangle TL -> BL -> BR
                indices.push_back(topLeft);
                indices.push_back(bottomLeft);
                indices.push_back(bottomRight);

                // Second triangle TL -> BR -> TR
                indices.push_back(topLeft);
                indices.push_back(bottomRight);
                indices.push_back(topRight);
            }
        }

        return {
            size,
            spacing,
            std::move(vertices),
            std::move(indices)
        };
    }

    GLuint uploadTriangulatedPlaneMeshToGPU(const TriangulatedPlaneMesh &mesh) {
        GLuint VAO, VBO, EBO;

        // VAO
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);

        // VBO
        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);

        // XZ Coord, TexCoord
        glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(GLfloat), mesh.vertices.data(), GL_STATIC_DRAW);

        // EBO
        glGenBuffers(1, &EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(GLuint), mesh.indices.data(), GL_STATIC_DRAW);

        // Vertex attribs
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)nullptr);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);

        return VAO;
    }
}

#endif // GEOMETRYUTILS_H
