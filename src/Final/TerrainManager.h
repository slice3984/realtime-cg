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
    TerrainManager(const int chunkSize, TerrainShaderProgram &terrainShader) : m_chunkSize(chunkSize), m_terrainShader(terrainShader), m_terrainGrid(5, std::vector<TerrainChunk>(5)) {
        generateChunkMeshes();
        uploadTextures();
        recalculateChunks(glm::vec3{0.0f});
        renderGrid();
    }

    // Regenerate chunks if required
    void update(const glm::vec3 &camPos) {
        // Get current center LOD 0 chunk
        TerrainChunk &centerChunk = m_terrainGrid[2][2];

        if (camPos.x < centerChunk.pos.x || camPos.x > centerChunk.pos.x) {
            recalculateChunks(camPos);
        }

        renderGrid();
    }

private:
    int m_chunkSize;
    std::unordered_map<int, ChunkLODMesh> m_lodChunkMeshes; // LOD, Meshes
    std::vector<std::vector<TerrainChunk>> m_terrainGrid;
    TerrainShaderProgram m_terrainShader;

    // Textures
    TextureHandle m_texSand;

    void generateChunkMeshes() {
        // Generate meshes with 3 LODs (0, 1, 2)
        for (int lod = 0; lod < 3; lod++) {
            ChunkLODMesh lodMesh;

            TerrainPatch unstitchedPatch = TerrainPatchLODGenerator::generateBasePatch(m_chunkSize, lod);
            TerrainPatchHandle unstitchedHandle = TerrainPatchLODGenerator::generateTerrainPatchHandle(unstitchedPatch);
            lodMesh.unstitchedMeshVAO = unstitchedHandle.VAO;
            lodMesh.unstitchedMeshElemCount = unstitchedHandle.elementCount;

            for (const auto edge : { STITCHED_EDGE::LEFT, STITCHED_EDGE::TOP, STITCHED_EDGE::RIGHT, STITCHED_EDGE::BOTTOM }) {
                // Create new meshes since they get modified at stitching
                TerrainPatch patch = TerrainPatchLODGenerator::generateBasePatch(m_chunkSize, lod);

                // Generate additional midpoint vetices for stitching
                TerrainPatchLODGenerator::stitchPatchEdge(patch, edge);

                // Triangulate and upload to GPU
                TerrainPatchHandle stitchedHandle = TerrainPatchLODGenerator::generateTerrainPatchHandle(patch);

                lodMesh.stitchedMeshes[edge] = { stitchedHandle.VAO, stitchedHandle.elementCount };
            }

            m_lodChunkMeshes[lod] = std::move(lodMesh);
        }
    }

    void uploadTextures() {
        ImageData sandTexImg = opengl_utils::loadImage("../assets/textures/terrain/mud.png");
        m_texSand = opengl_utils::createTexture(sandTexImg, true);
        opengl_utils::updateTextureData(m_texSand, sandTexImg);
    }

    void recalculateChunks(const glm::vec3 &currPos) {
        glm::vec2 gridChunk = { std::floor(currPos.x / m_chunkSize), std::floor(currPos.z / m_chunkSize) };
        glm::vec2 gridStartPos{ (gridChunk.x * m_chunkSize) - (2 * m_chunkSize), (gridChunk.y * m_chunkSize) - (2 * m_chunkSize) };

        auto calculateLod = [](const int row, const int column) -> int {
            return std::max(std::abs( row - 2), std::abs(column - 2));
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
        m_terrainShader.setInt("u_texDiffuse", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_texSand.handle);

        for (auto &row : m_terrainGrid) {
            for (auto &chunk : row) {
                chunk.render(m_terrainShader);
            }
        }
    }
};



#endif //TERRAINMANAGER_H
