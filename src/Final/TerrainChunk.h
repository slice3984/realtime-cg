//
// Created by slice on 12/26/24.
//

#ifndef TERRAINCHUNK_H
#define TERRAINCHUNK_H
#include <glm/glm.hpp>

#include "TerrainPatchLODGenerator.h"
#include "../Shaders/TerrainShader/TerrainShaderProgram.h"

class TerrainChunk {
public:
    int lod;
    glm::vec2 globalPos;
    // Starts at top left corner of the terrain grid
    // Bottom left corner of the chunk
    glm::vec2 localGridPos;
    MeshBufferPosition bufferPos;

    float gridSpacing;
    GLuint indexBufferOffset;
    GLuint drawCount;

    void render(const TerrainShaderProgram &shader) const {
        glm::mat4 model = glm::mat4{1.0f};
        model = glm::translate(model, glm::vec3(globalPos.x, 0.0f, globalPos.y));

        shader.setMat4f("u_model", model);
        shader.setVec2f("u_chunkOffset", globalPos);
        glDrawElements(GL_TRIANGLES, bufferPos.indexCount, GL_UNSIGNED_INT, (void*)(bufferPos.indexOffset * sizeof(GLuint)));
    }
};



#endif //TERRAINCHUNK_H
