//
// Created by slice on 12/23/24.
//

#ifndef TERRAINPATCHLODGENERATOR_H
#define TERRAINPATCHLODGENERATOR_H
#include <iostream>
#include <valarray>
#include <vector>

#include "../../Linking/include/glad/glad.h"

enum class STITCHED_EDGE {
    LEFT = 0,
    TOP = 1,
    RIGHT = 2,
    BOTTOM = 3,
};

struct Vertex {
    GLfloat x;
    GLfloat z;
};

struct TerrainPatch {
    std::vector<std::vector<Vertex> > grid;
    int basePatchSize;
    int lod;
    int stepSize;
    bool stitchedLeftRight = false;
    bool stitchedTopButtom = false;
    bool stitchedLeft = false;
    bool stitchedTop = false;
    bool stitchedRight = false;
    bool stitchedBottom = false;
};

struct TerrainPatchHandle {
    GLuint VAO;
    GLuint elementCount;
    int basePatchSize;
    int lod;
};

class TerrainPatchLODGenerator {
public:
    static TerrainPatch generateBasePatch(const int basePatchSize, const int lodLevel) {
        TerrainPatch patch;
        patch.basePatchSize = basePatchSize;
        patch.lod = lodLevel;
        int stepSize = lodLevel + 1;
        patch.stepSize = stepSize;


        for (int row = 0; row <= basePatchSize; row += stepSize) {
            std::vector<Vertex> gridRow;
            for (int column = 0; column <= basePatchSize; column += stepSize) {
                gridRow.push_back({(GLfloat) column, (GLfloat) row});
            }

            // In case the base patch size is not evenly divisible by the stepsize
            if (gridRow[gridRow.size() - 1].x < basePatchSize) {
                gridRow.push_back({(GLfloat) basePatchSize, (GLfloat) row});
            }

            patch.grid.push_back(std::move(gridRow));
        }


        if (patch.grid[patch.grid.size() - 1][0].z < basePatchSize) {
            std::vector<Vertex> extraRow;
            for (int column = 0; column <= basePatchSize; column += stepSize) {
                extraRow.push_back({(GLfloat) column, (GLfloat) basePatchSize});
            }

            // Add a vertex for the edge case where step size doesn't align perfectly
            if (extraRow[extraRow.size() - 1].x < basePatchSize) {
                extraRow.push_back({(GLfloat) basePatchSize, (GLfloat) basePatchSize});
            }

            patch.grid.push_back(std::move(extraRow));
        }

        return std::move(patch);
    }

    // Inserts extra vertices as midpoints between original vertices at edges for stitching
    // Required for transitions between LODs
    static void stitchPatchEdge(TerrainPatch &patch, STITCHED_EDGE edge) {
        if (edge == STITCHED_EDGE::LEFT || edge == STITCHED_EDGE::RIGHT) {
            if (!patch.stitchedLeftRight) {
                // Insert new rows made up of 1 column as vertices for stitching
                float height = (patch.grid[1][0].z - patch.grid[0][0].z);

                // Iterate through the grid to insert rows at the correct position
                for (int row = 0; row < patch.grid.size() - 1;) {
                    Vertex vertex = {0.0f, patch.grid[row][0].z + height / 2.0f};

                    if (edge == STITCHED_EDGE::RIGHT) {
                        vertex.x = patch.basePatchSize;
                    }

                    patch.grid.insert(patch.grid.begin() + row + 1, std::vector<Vertex>{vertex});
                    row += 2;
                }

                // Mark as stitched, so for the next call no additional rows are inserted
                patch.stitchedLeftRight = true;
            } else {
                // Left or right side already stitched, no need to insert new rows
                for (int row = 0; row < patch.grid.size(); row++) {
                    if (row % 2 == 0) {
                        continue;
                    }

                    Vertex vertex = {0.0f, patch.grid[row][0].z};
                    if (edge == STITCHED_EDGE::LEFT) {
                        patch.grid[row].insert(patch.grid[row].begin(), vertex);
                    } else {
                        vertex.x = patch.basePatchSize;
                        patch.grid[row].push_back(vertex);
                    }
                }
            }

            if (edge == STITCHED_EDGE::LEFT) {
                patch.stitchedLeft = true;
            } else {
                patch.stitchedRight = true;
            }
        } else if (edge == STITCHED_EDGE::TOP || edge == STITCHED_EDGE::BOTTOM) {
            if (edge == STITCHED_EDGE::TOP) {
                for (int column = 0; column < patch.grid[0].size() - 1;) {
                    std::vector<Vertex> &row = patch.grid[0];
                    Vertex vertex = {row[column].x + (patch.stepSize / 2.0f), 0};
                    row.insert(row.begin() + column + 1, vertex);
                    column += 2;
                }
            } else {
                for (int column = 0; column < patch.grid[patch.grid.size() - 1].size() - 1;) {
                    std::vector<Vertex> &row = patch.grid[patch.grid.size() - 1];
                    Vertex vertex = {row[column].x + (patch.stepSize / 2.0f), row[0].z};
                    row.insert(row.begin() + column + 1, vertex);
                    column += 2; // Skip the newly inserted vertex
                }
            }

            if (edge == STITCHED_EDGE::TOP) {
                patch.stitchedTop = true;
            } else {
                patch.stitchedBottom = true;
            }

            patch.stitchedTopButtom = true;
        }
    }

    static std::vector<GLuint> triangulatePatch(const TerrainPatch &patch) {
        std::vector<GLuint> indices;

        // The grid is not interleaved, calculate the final index
        auto getInterleavedIndex = [&](int row, int column) -> int {
            // Just loop and add up lengths, the index could be further apart due to stitching
            int finalIndex = 0;
            for (int i = 0; i < row; i++) {
                finalIndex += patch.grid[i].size();
            }

            return finalIndex + column;
        };

        for (int row = 0; row < patch.grid.size() - 1; row++) {
            int stitchOffset = 0;

            for (int column = 0; column < patch.grid[row].size() - 1; column++) {
                int a = patch.grid[row].size();
                const std::vector<Vertex> &rows = patch.grid[row];
                // The mesh got stitched at the top
                if (patch.stitchedTop && row == 0) {
                    const int indexTopLeft = getInterleavedIndex(row, column);
                    const int indexTopRight = getInterleavedIndex(row, column + 1);
                    column++;
                    const int indexBottomLeft = getInterleavedIndex(row + 1, column / 2);
                    const int indexBottomRight = getInterleavedIndex(row + 1, (column + 1) / 2);

                    indices.insert(indices.end(), {indexBottomLeft, indexBottomRight, indexTopRight});
                    indices.insert(indices.end(), {indexBottomLeft, indexTopRight, indexTopLeft});

                    const int indexTopRightShifted = getInterleavedIndex(row, column + (1));
                    indices.insert(indices.end(), {indexTopRight, indexBottomRight, indexTopRightShifted});

                    // The mesh got stitched at the bottom
                } else if (patch.stitchedBottom && row == patch.grid.size() - 2) {
                    const int indexTopLeft = getInterleavedIndex(row, column);
                    const int indexTopRight = getInterleavedIndex(row, column + 1);
                    const int indexBottomLeft = getInterleavedIndex(row + 1, column + stitchOffset);
                    const int indexBottomRight = getInterleavedIndex(row + 1, column + 1 + stitchOffset);

                    indices.insert(indices.end(), {indexTopLeft, indexBottomRight, indexTopRight});
                    indices.insert(indices.end(), {indexTopLeft, indexBottomLeft, indexBottomRight});

                    const int indexBottomRightShifted = getInterleavedIndex(row + 1, column + 2 + stitchOffset);

                    stitchOffset++;

                    indices.insert(indices.end(), {indexTopRight, indexBottomRight, indexBottomRightShifted});
                    // The mesh got stitched on the left
                } else if (patch.stitchedLeft && column == 0) {
                    const int indexTopLeft = getInterleavedIndex(row, column);
                    const int indexTopRight = getInterleavedIndex(row, column + 1);
                    const int indexBottomLeft = getInterleavedIndex(row + 1, column);
                    const int indexBottomRight = getInterleavedIndex(row + 2, column + 1);

                    indices.insert(indices.end(), {indexBottomLeft, indexBottomRight, indexTopRight});
                    indices.insert(indices.end(), {indexBottomLeft, indexTopRight, indexTopLeft});

                    const int indexBottomLeftShifted = getInterleavedIndex(row + 2, column);
                    indices.insert(indices.end(), {indexBottomLeft, indexBottomLeftShifted, indexBottomRight});
                    // The mesh got stitched on the right
                } else if (patch.stitchedRight && column == patch.grid[row].size() - 2) {
                    const int indexTopLeft = getInterleavedIndex(row, column);
                    const int indexTopRight = getInterleavedIndex(row, column + 1);
                    const int indexBottomLeft = getInterleavedIndex(row + 2, column);
                    const int indexBottomRight = getInterleavedIndex(row + 1, 0);

                    indices.insert(indices.end(), {indexTopLeft, indexBottomLeft, indexBottomRight});
                    indices.insert(indices.end(), {indexTopLeft, indexBottomRight, indexTopRight});

                    const int indexBottomRightShifted = getInterleavedIndex(row + 2, column + 1);
                    indices.insert(indices.end(), {indexBottomLeft, indexBottomRightShifted, indexBottomRight});
                    // Base case, no stitching, basic 2 triangle quads
                } else {
                    const int indexTopLeft = getInterleavedIndex(row, column);
                    const int indexTopRight = getInterleavedIndex(row, column + 1);
                    // Skip rows in case of left or right stitching
                    const int indexBottomLeft = getInterleavedIndex(row + (patch.stitchedLeftRight ? 2 : 1), column);
                    const int indexBottomRight = getInterleavedIndex(row + (patch.stitchedLeftRight ? 2 : 1), column + 1);

                    // Triangle one and two, same for every cell
                    indices.insert(indices.end(), {indexBottomLeft, indexBottomRight, indexTopRight});
                    indices.insert(indices.end(), {indexBottomLeft, indexTopRight, indexTopLeft});
                }
            }

            if (patch.stitchedLeftRight) {
                row++;
            }
        }

        return std::move(indices);
    }

    static std::vector<GLfloat> generateVertexBuffer(const TerrainPatch &patch) {
        std::vector<GLfloat> buffer;

        // Get total amount of vertices
        std::size_t vertexCount = 0;
        for (const auto &row: patch.grid) {
            vertexCount += row.size();
        }

        vertexCount *= 2; // 2 GLfloats per vertex
        buffer.reserve(vertexCount);

        for (const auto &row: patch.grid) {
            for (const Vertex &vertex: row) {
                buffer.push_back(vertex.x);
                buffer.push_back(vertex.z);
            }
        }

        return std::move(buffer);
    }

    static TerrainPatchHandle generateTerrainPatchHandle(const TerrainPatch &patch) {
        GLuint VAO, VBO, EBO;

        // VAO
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);

        // VBO
        std::vector<GLfloat> vertexBuffer = generateVertexBuffer(patch);
        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertexBuffer.size() * sizeof(GLfloat), vertexBuffer.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid *) 0);
        glEnableVertexAttribArray(0); // Enable the attribute

        // EBO
        std::vector<GLuint> ebo = triangulatePatch(patch);
        glGenBuffers(1, &EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, ebo.size() * sizeof(GLuint), ebo.data(), GL_STATIC_DRAW);

        glBindVertexArray(0);

        return {
            VAO,
            (GLuint) ebo.size(),
            patch.basePatchSize,
            patch.lod
        };
    }
};


#endif //TERRAINPATCHLODGENERATOR_H