//
// Created by slice on 12/26/24.
//

#ifndef TERRAINMANAGER_H
#define TERRAINMANAGER_H
#include <vector>
#include <glm/glm.hpp>

#include "InstancingManager.h"
#include "TerrainChunk.h"
#include "TerrainPatchLODGenerator.h"
#include "../ComputeShader.h"
#include "../GPUModelUploader.h"
#include "../Shaders/ModelShader/ModelShaderProgram.h"
#include "../Shaders/GrassShaderInstanced/GrassShaderInstancedProgram.h"
#include "../Shaders/TerrainShader/TerrainShaderProgram.h"


class TerrainManager {
public:
    static constexpr int XZ_CHUNK_AMOUNT = 5;

    TerrainManager(const int chunkSize, TerrainShaderProgram &terrainShader, GrassShaderInstancedProgram &modelShaderInstanced, const float &terrainHeight,
                   const int &octaves, const float &scale, const float &persistance,
                   const float &lucunarity) : m_chunkSize(chunkSize),
                                              m_terrainShader(terrainShader),
    m_modelShaderInstanced(modelShaderInstanced),
                                              m_terrainGrid(
                                                  5, std::vector<TerrainChunk>(5)),
                                              m_terrainHeight(terrainHeight),
                                              m_octaves(octaves), m_scale(scale), m_persistance(persistance),
                                              m_lucunarity(lucunarity),
                                                m_terrainComputeShader{"../src/Shaders/TerrainShader/shader.compute",
                                                } {
        generateChunkMeshes();
        setupInstancingManager();
        uploadTextures();
        recalculateChunks(glm::vec3{0.0f});
        dispatchCompute();
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
            dispatchCompute();
        }
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
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
    MeshBufferInfo m_meshBufferPositions; // Contains buffer positions of all LODs and Meshes
    std::vector<std::vector<TerrainChunk> > m_terrainGrid;
    TerrainShaderProgram m_terrainShader;
    TerrainBufferHandles m_terrainBufferHandles;
    ComputeShader m_terrainComputeShader;
    std::unique_ptr<InstancingManager> m_instancingManager;

    // Shaders
    GrassShaderInstancedProgram &m_modelShaderInstanced;
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

    static int calculateLod(const int row, const int column) {
        return std::max(std::abs(row - 2), std::abs(column - 2));
    }

    void generateChunkMeshes() {
        std::vector<std::pair<int, STITCHED_EDGE>> meshesToGenerate;

        // We have to generate a mesh for each individual rendered chunk
        // First temporarily store all the meshes we need
        for (int row = 0; row < XZ_CHUNK_AMOUNT; row++) {
            for (int column = 0; column < XZ_CHUNK_AMOUNT; column++) {
                // 2 at outer boundaries, 0 at center
                int lod = calculateLod(row, column);

                if (row > 0 && calculateLod(row - 1, column) < lod) {
                    // Top
                    meshesToGenerate.emplace_back(  lod, STITCHED_EDGE::TOP );
                } else if (column < 4 && calculateLod(row, column + 1) < lod) {
                    // Right
                    meshesToGenerate.emplace_back(  lod, STITCHED_EDGE::RIGHT );
                } else if (row < 4 && calculateLod(row + 1, column) < lod) {
                    // Bottom
                    meshesToGenerate.emplace_back(  lod, STITCHED_EDGE::BOTTOM );
                } else if (column > 0 && calculateLod(row, column - 1) < lod) {
                    // Left
                    meshesToGenerate.emplace_back(  lod, STITCHED_EDGE::LEFT );
                } else {
                    // No stitching
                    meshesToGenerate.emplace_back(  lod, STITCHED_EDGE::NONE );
                }
            }
        }

        m_meshBufferPositions = TerrainPatchLODGenerator::generateMultiMeshBuffer(m_chunkSize, meshesToGenerate);

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
        glm::vec2 gridChunk = {
            std::floor(currPos.x / m_chunkSize),
            std::floor(currPos.z / m_chunkSize)
        };
        glm::vec2 gridStartPos = {
            (gridChunk.x * m_chunkSize) - (2 * m_chunkSize),
            (gridChunk.y * m_chunkSize) - (2 * m_chunkSize)
        };

        for (int row = 0; row < XZ_CHUNK_AMOUNT; row++) {
            for (int column = 0; column < XZ_CHUNK_AMOUNT; column++) {
                int lod = calculateLod(row, column);
                int meshIndex = row * XZ_CHUNK_AMOUNT + column;

                TerrainChunk chunk;
                chunk.pos = {
                    gridStartPos.x + column * m_chunkSize,
                    gridStartPos.y + row * m_chunkSize
                };
                chunk.lod = lod;

                const MeshBufferDescriptor& descriptor = m_meshBufferPositions.meshes[meshIndex];
                chunk.bufferPos = descriptor.bufferPosition;
                chunk.gridSpacing = descriptor.stepSize;

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

        m_instancingManager->issueDrawCalls();
    }

    void setupInstancingManager() {
        m_instancingManager = std::make_unique<InstancingManager>(m_terrainComputeShader, m_meshBufferPositions.totalVertexCount);

        // Set models to be instanced
        GltfLoader loader;
        GPUModelUploader uploader;

        GltfScene grassBlade = loader.loadModel("../assets/models/terrain/grass.glb");
        std::vector<RenderCall> grassBladeRenderCalls = uploader.uploadGltfModel(grassBlade);
        m_instancingManager->addModelToBeInstanced(grassBladeRenderCalls, &m_modelShaderInstanced);

        GltfScene tv = loader.loadModel("../assets/models/tv.glb");
        std::vector<RenderCall> tvRenderCalls = uploader.uploadGltfModel(tv);
        m_instancingManager->addModelToBeInstanced(tvRenderCalls, &m_modelShaderInstanced);

        // Initialize buffers
        glUseProgram(m_terrainComputeShader.getProgramId());
        m_instancingManager->initializeSSBOBuffers();
        m_instancingManager->setupInstanceCountAtomics();
    }

    void dispatchCompute() {
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_terrainBufferHandles.SSBO);
        glUseProgram(m_terrainComputeShader.getProgramId());

        m_terrainComputeShader.setFloat("u_terrainHeight", m_terrainHeight);
        m_terrainComputeShader.setFloat("u_scale", m_scale);
        m_terrainComputeShader.setFloat("u_persistance", m_persistance);
        m_terrainComputeShader.setFloat("u_lucunarity", m_lucunarity);
        m_terrainComputeShader.setInt("u_octaves", m_octaves);

        const uint workGroupSize = 256;
        const uint verticesPerDispatch = 1024;
        for (const auto& row : m_terrainGrid) {
            for (const TerrainChunk& chunk : row) {
                const uint amountVertices = chunk.bufferPos.vertexCount;
                const uint baseOffset = chunk.bufferPos.vertexOffset;

                m_terrainComputeShader.setVec2f("u_chunkOffset", chunk.pos);
                m_terrainComputeShader.setInt("u_stepSize", (int)chunk.gridSpacing);
                m_instancingManager->setComputeShaderOffsetUniforms();

                for (uint offset = 0; offset < amountVertices; offset += verticesPerDispatch) {
                    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
                    uint count = std::min(verticesPerDispatch, (amountVertices - offset));
                    uint numGroups = (count + workGroupSize - 1) / workGroupSize;

                    m_terrainComputeShader.setInt("u_bufferOffset", baseOffset + offset);
                    m_terrainComputeShader.setInt("u_count", count);

                    glDispatchCompute(numGroups, 1, 1);
                    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
                }

                m_instancingManager->prepareAtomicCounterFetching();
            }
        }
    }
};


#endif //TERRAINMANAGER_H
