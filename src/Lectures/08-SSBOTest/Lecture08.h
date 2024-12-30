//
// Created by slice on 12/30/24.
//

#ifndef LECTURE08_H
#define LECTURE08_H
#include <string_view>

#include "../../RenderBase.h"

class Lecture08 : public RenderBase {
public:
    explicit  Lecture08(std::string_view title) : RenderBase(title) {

    }

    void init() override {
        struct VertexData {
            glm::vec3 pos;
            float padding1;
            glm::vec3 normal;
            float padding2;
        };

        VertexData vertexData[] = {
            {{-0.5f, -0.5f, 0.0f}, 0.0f, {0.0f, 1.0f, 0.0f}, 0.0f},
            {{ 0.5f, -0.5f, 0.0f}, 0.0f, {0.0f, 1.0f, 0.0f}, 0.0f},
            {{ 0.5f,  0.5f, 0.0f}, 0.0f, {0.0f, 1.0f, 0.0f}, 0.0f},
            {{-0.5f,  0.5f, 0.0f}, 0.0f, {0.0f, 1.0f, 0.0f}, 0.0f}
        };

        GLuint indices[] = {
            0, 1, 2,
            0, 2, 3
        };

        size_t s = sizeof(vertexData);

        // SSBO
        glGenBuffers(1, &m_SSBO);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_SSBO);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

        // VAO & EBO
        glGenVertexArrays(1, &m_VAO);
        glBindVertexArray(m_VAO);

        GLuint ebo;
        glGenBuffers(1, &ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
        glBindVertexArray(0);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);



        compileShaders("../src/Lectures/08-SSBOTest/shader.vert",
                       "../src/Lectures/08-SSBOTest/shader.frag");
    }

    void render() override {
        shaderReloadWindow();
        glUseProgram(m_programId);
        glBindVertexArray(m_VAO);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_SSBO);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_SSBO);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(0);
    }

private:
    GLuint m_VAO;
    GLuint m_SSBO;
};



#endif //LECTURE08_H
