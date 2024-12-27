//
// Created by slice on 12/26/24.
//

#ifndef TERRAINMANAGER_H
#define TERRAINMANAGER_H
#include <vector>
#include <glm/glm.hpp>
#include "ChunkLODMesh.h"
#include "TerrainChunk.h"
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

    //glm::vec3 getNormal()

private:
    int m_chunkSize;
    std::unordered_map<int, ChunkLODMesh> m_lodChunkMeshes; // LOD, Meshes
    std::vector<std::vector<TerrainChunk> > m_terrainGrid;
    TerrainShaderProgram m_terrainShader;

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
        // Generate meshes with 3 LODs (0, 1, 2)
        for (int lod = 0; lod < 3; lod++) {
            ChunkLODMesh lodMesh;

            TerrainPatch unstitchedPatch = TerrainPatchLODGenerator::generateBasePatch(m_chunkSize, lod);
            TerrainPatchHandle unstitchedHandle = TerrainPatchLODGenerator::generateTerrainPatchHandle(unstitchedPatch);
            lodMesh.unstitchedMeshVAO = unstitchedHandle.VAO;
            lodMesh.unstitchedMeshElemCount = unstitchedHandle.elementCount;

            for (const auto edge: {
                     STITCHED_EDGE::LEFT, STITCHED_EDGE::TOP, STITCHED_EDGE::RIGHT, STITCHED_EDGE::BOTTOM
                 }) {
                // Create new meshes since they get modified at stitching
                TerrainPatch patch = TerrainPatchLODGenerator::generateBasePatch(m_chunkSize, lod);

                // Generate additional midpoint vetices for stitching
                TerrainPatchLODGenerator::stitchPatchEdge(patch, edge);

                // Triangulate and upload to GPU
                TerrainPatchHandle stitchedHandle = TerrainPatchLODGenerator::generateTerrainPatchHandle(patch);

                lodMesh.stitchedMeshes[edge] = {stitchedHandle.VAO, stitchedHandle.elementCount};
            }

            m_lodChunkMeshes[lod] = std::move(lodMesh);
        }
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
                ChunkLODMesh &mesh = m_lodChunkMeshes[lod];
                if (row > 0 && calculateLod(row - 1, column) < lod) {
                    // Top
                    chunk.mesh.VAO = mesh.stitchedMeshes[STITCHED_EDGE::TOP].VAO;
                    chunk.mesh.elemCount = mesh.stitchedMeshes[STITCHED_EDGE::TOP].elemCount;
                } else if (column < 4 && calculateLod(row, column + 1) < lod) {
                    // Right
                    chunk.mesh.VAO = mesh.stitchedMeshes[STITCHED_EDGE::RIGHT].VAO;
                    chunk.mesh.elemCount = mesh.stitchedMeshes[STITCHED_EDGE::RIGHT].elemCount;
                } else if (row < 4 && calculateLod(row + 1, column) < lod) {
                    // Bottom
                    chunk.mesh.VAO = mesh.stitchedMeshes[STITCHED_EDGE::BOTTOM].VAO;
                    chunk.mesh.elemCount = mesh.stitchedMeshes[STITCHED_EDGE::BOTTOM].elemCount;
                } else if (column > 0 && calculateLod(row, column - 1) < lod) {
                    // Left
                    chunk.mesh.VAO = mesh.stitchedMeshes[STITCHED_EDGE::LEFT].VAO;
                    chunk.mesh.elemCount = mesh.stitchedMeshes[STITCHED_EDGE::LEFT].elemCount;
                } else {
                    // No stitching
                    chunk.mesh.VAO = mesh.unstitchedMeshVAO;
                    chunk.mesh.elemCount = mesh.unstitchedMeshElemCount;
                }

                m_terrainGrid[row][column] = chunk;
            }
        }
    }

    void renderGrid() {
        m_terrainShader.use();
        m_terrainShader.setInt("u_texLayerOne", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_texLayerOne.handle);

        m_terrainShader.setInt("u_texLayerTwo", 1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, m_texLayerTwo.handle);

        for (auto &row: m_terrainGrid) {
            for (auto &chunk: row) {
                chunk.render(m_terrainShader);
            }
        }
    }
};


#endif //TERRAINMANAGER_H
