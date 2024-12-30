//
// Created by slice on 12/26/24.
//

#ifndef TERRAINMANAGER_H
#define TERRAINMANAGER_H
#include <vector>
#include <glm/glm.hpp>
#include "TerrainChunk.h"
#include "TerrainPatchLODGenerator.h"
#include "../Shaders/TerrainShader/TerrainShaderProgram.h"


class TerrainManager {
public:
    TerrainManager(const int chunkSize, TerrainShaderProgram &terrainShader, const float &terrainHeight,
                   const int &octaves, const float &scale, const float &persistance,
                   const float &lucunarity) : m_chunkSize(chunkSize),
                                              m_terrainShader(terrainShader),
                                              m_terrainGrid(
                                                  5, std::vector<TerrainChunk>(5)),
                                              m_terrainHeight(terrainHeight),
                                              m_octaves(octaves), m_scale(scale), m_persistance(persistance),
                                              m_lucunarity(lucunarity) {
        generateChunkMeshes();
        uploadTextures();
        recalculateChunks(glm::vec3{0.0f});
        renderGrid();
    }

    // Regenerate chunks if required
    void update(const glm::vec3 &camPos) {
        // Determine the grid position of the current center chunk (LOD 0)
        glm::vec2 currentCenterChunkPos = {
            std::floor(camPos.x / m_chunkSize),
            std::floor(camPos.z / m_chunkSize)
        };

        // Determine the grid position of the last known center chunk (LOD 0)
        glm::vec2 lastCenterChunkPos = {
            std::floor(m_terrainGrid[2][2].pos.x / m_chunkSize),
            std::floor(m_terrainGrid[2][2].pos.y / m_chunkSize)
        };

        // If the camera has moved into a new chunk, recalculate the terrain grid
        if (currentCenterChunkPos != lastCenterChunkPos) {
            recalculateChunks(camPos);
        }

        renderGrid();
    }

    [[nodiscard]] float getHeight(const glm::vec3 &pos) const {
        float noiseHeight = 0.0f;
        float amplitude = 1.0f;
        float frequency = 1.0f;

        for (int i = 0; i < m_octaves; i++) {
            const glm::vec2 xzPos = glm::vec2{pos.x, pos.z};

            noiseHeight += amplitude * SimplexNoise::snoise(xzPos / (m_scale * frequency));
            amplitude *= m_persistance;
            frequency *= m_lucunarity;
        }

        return m_terrainHeight * (noiseHeight + 1.0f) * 0.5f;
    }

    glm::vec3 calculateNormal(const glm::vec3 &localPos, const TerrainChunk &chunk) const {
        glm::vec3 worldPos = glm::vec3{chunk.pos.x, 0.0f, chunk.pos.y} + localPos;

        float hL = getHeight(worldPos - glm::vec3(1.0f, 0.0f, 0.0f)); // Left
        float hR = getHeight(worldPos + glm::vec3(1.0f, 0.0f, 0.0f)); // Right
        float hD = getHeight(worldPos - glm::vec3(0.0f, 0.0f, 1.0f)); // Down
        float hU = getHeight(worldPos + glm::vec3(0.0f, 0.0f, 1.0f)); // Up

        glm::vec3 dx = glm::vec3(2.0f, hR - hL, 0.0f);
        glm::vec3 dz = glm::vec3(0.0f, hU - hD, 2.0f);

        glm::vec3 normal = glm::normalize(glm::cross(dz, dx));

        return normal;
    }

    const TerrainChunk& getTerrainChunk(const int row, const int column) const {
        return m_terrainGrid[row][column];
    }

    glm::vec3 getWorldSpacePositionInChunk(const glm::vec2 &xzPos, const TerrainChunk &chunk) {
        glm::vec2 worldPosXZ = chunk.pos + xzPos;
        return glm::vec3{worldPosXZ.x, getHeight(glm::vec3{worldPosXZ.x, 0.0f, worldPosXZ.y}), worldPosXZ.y};
    }

private:
    int m_chunkSize;
    LODBufferInfo m_meshBufferPositions; // Contains buffer positions of all LODs and Meshes
    std::vector<std::vector<TerrainChunk> > m_terrainGrid;
    TerrainShaderProgram m_terrainShader;
    TerrainBufferHandles m_terrainBufferHandles;

    // Textures
    TextureHandle m_texLayerOne;
    TextureHandle m_texLayerTwo;
    TextureHandle m_texLayerThree;

    // Terrain noise parameters
    // Get as reference so it can live update
    const float &m_terrainHeight;
    const int &m_octaves;
    const float &m_scale;
    const float &m_persistance;
    const float &m_lucunarity;

    void generateChunkMeshes() {

        m_meshBufferPositions = TerrainPatchLODGenerator::generateMultiLODBuffers(m_chunkSize, 2);

        // Generate and set up VAO/EBO/SSBO
        m_terrainBufferHandles = TerrainPatchLODGenerator::generateTerrainBufferHandles(m_meshBufferPositions);

        // Cleanup
        glBindVertexArray(0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    void uploadTextures() {
        ImageData imgTexLayerOne = opengl_utils::loadImage("../assets/textures/terrain/rock.png");
        m_texLayerOne = opengl_utils::createTexture(imgTexLayerOne, true);
        opengl_utils::updateTextureData(m_texLayerOne, imgTexLayerOne);

        ImageData imgTexLayerTwo = opengl_utils::loadImage("../assets/textures/terrain/rocky_terrain.png");
        m_texLayerTwo = opengl_utils::createTexture(imgTexLayerTwo, true);
        opengl_utils::updateTextureData(m_texLayerTwo, imgTexLayerTwo);
    }

    void recalculateChunks(const glm::vec3 &currPos) {
        glm::vec2 gridChunk = {std::floor(currPos.x / m_chunkSize), std::floor(currPos.z / m_chunkSize)};
        glm::vec2 gridStartPos{
            (gridChunk.x * m_chunkSize) - (2 * m_chunkSize), (gridChunk.y * m_chunkSize) - (2 * m_chunkSize)
        };

        auto calculateLod = [](const int row, const int column) -> int {
            return std::max(std::abs(row - 2), std::abs(column - 2));
        };

        // Generate a 5x5 terrain grid
        // LOD decreases in the outer areas
        for (int row = 0; row < 5; row++) {
            for (int column = 0; column < 5; column++) {
                // 2 at outer boundaries, 0 at center
                int lod = calculateLod(row, column);

                TerrainChunk chunk;
                chunk.pos = {
                    gridStartPos.x + column * m_chunkSize,
                    gridStartPos.y + row * m_chunkSize
                };

                chunk.lod = lod;

                // Check if and where stitching is required
                LODBufferPos meshPos = m_meshBufferPositions.lodMeshes[lod];

                if (row > 0 && calculateLod(row - 1, column) < lod) {
                    // Top
                    chunk.indexBufferOffset = meshPos.stitchedMeshes[STITCHED_EDGE::TOP].indexOffset;
                    chunk.drawCount = meshPos.stitchedMeshes[STITCHED_EDGE::TOP].indexCount;
                } else if (column < 4 && calculateLod(row, column + 1) < lod) {
                    // Right
                    chunk.indexBufferOffset = meshPos.stitchedMeshes[STITCHED_EDGE::RIGHT].indexOffset;
                    chunk.drawCount = meshPos.stitchedMeshes[STITCHED_EDGE::RIGHT].indexCount;
                } else if (row < 4 && calculateLod(row + 1, column) < lod) {
                    // Bottom
                    chunk.indexBufferOffset = meshPos.stitchedMeshes[STITCHED_EDGE::BOTTOM].indexOffset;
                    chunk.drawCount = meshPos.stitchedMeshes[STITCHED_EDGE::BOTTOM].indexCount;
                } else if (column > 0 && calculateLod(row, column - 1) < lod) {
                    // Left
                    chunk.indexBufferOffset = meshPos.stitchedMeshes[STITCHED_EDGE::LEFT].indexOffset;
                    chunk.drawCount = meshPos.stitchedMeshes[STITCHED_EDGE::LEFT].indexCount;
                } else {
                    // No stitching
                    chunk.indexBufferOffset = meshPos.baseMesh.indexOffset;
                    chunk.drawCount = meshPos.baseMesh.indexCount;
                }

                m_terrainGrid[row][column] = chunk;
            }
        }
    }

    void renderGrid() {
        m_terrainShader.use();

        // Texture setup
        m_terrainShader.setInt("u_texLayerOne", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_texLayerOne.handle);

        m_terrainShader.setInt("u_texLayerTwo", 1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, m_texLayerTwo.handle);

        glBindVertexArray(m_terrainBufferHandles.VAO);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_terrainBufferHandles.SSBO);

        // Render chunks
        for (auto &row: m_terrainGrid) {
            for (auto &chunk: row) {
                chunk.render(m_terrainShader);
            }
        }

        // Cleanup
        glBindVertexArray(0);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
    }
};


#endif //TERRAINMANAGER_H
