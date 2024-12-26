//
// Created by slice on 12/26/24.
//

#ifndef TERRAINCHUNK_H
#define TERRAINCHUNK_H
#include <glm/glm.hpp>
#include "ChunkLODMesh.h"
#include "../Shaders/TerrainShader/TerrainShaderProgram.h"

class TerrainChunk {
public:
    ChunkMesh mesh;
    int lod;
    glm::vec2 pos;

    void render(const TerrainShaderProgram &shader) const {
        glBindVertexArray(mesh.VAO);

        glm::mat4 model = glm::mat4{1.0f};
        model = glm::translate(model, glm::vec3{pos.x, 1.0f, pos.y});

        shader.setMat4f("u_model", model);
        shader.setVec2f("u_chunkOffset", pos);

        glDrawElements(GL_TRIANGLES, mesh.elemCount, GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(0);
    }
};



#endif //TERRAINCHUNK_H
