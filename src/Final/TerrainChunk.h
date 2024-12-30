//
// Created by slice on 12/26/24.
//

#ifndef TERRAINCHUNK_H
#define TERRAINCHUNK_H
#include <glm/glm.hpp>
#include "../Shaders/TerrainShader/TerrainShaderProgram.h"

class TerrainChunk {
public:
    int lod;
    glm::vec2 pos;

    GLuint indexBufferOffset;
    GLuint drawCount;

    void render(const TerrainShaderProgram &shader) const {
        glm::mat4 model = glm::mat4{1.0f};
        model = glm::translate(model, glm::vec3{pos.x, 1.0f, pos.y});

        shader.setMat4f("u_model", model);
        shader.setVec2f("u_chunkOffset", pos);
        glDrawElements(GL_TRIANGLES, drawCount, GL_UNSIGNED_INT, (void*)(indexBufferOffset * sizeof(GLuint)));
    }
};



#endif //TERRAINCHUNK_H
