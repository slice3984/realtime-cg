//
// Created by slice on 11/2/24.
//

#ifndef ASSIGNMENT01_H
#define ASSIGNMENT01_H
#include "glad/glad.h"
#include <glm/glm.hpp>
#include "../../RenderBase.h"

class Assignment01 : public RenderBase {
private:
    GLuint m_VAO;
    glm::vec3 m_color;
    int m_columnCount = 50;
    float m_animSpeed = 0.2f;

public:
    explicit Assignment01(std::string_view title) : RenderBase(title) {}

    void init() override {
        // Full screen quad
        GLfloat vertices[] = {
            -1.0f, 1.0f, 0.0f, // Top left
            1.0f, 1.0f, 0.0f, // Top right
            -1.0f, -1.0f, 0.0f, // Bottom left
            1.0f, -1.0f, 0.0f // Bottom right
        };

        GLuint indices[] = {
            2, 1, 0,
            2, 3, 1
        };
        glGenVertexArrays(1, &m_VAO);
        glBindVertexArray(m_VAO);

        GLuint EBO;
        glGenBuffers(1, &EBO);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        GLuint VBO;
        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        compileShaders("../src/Assignments/01-GLSL/shader.vert",
            "../src/Assignments/01-GLSL/shader.frag");

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void *) nullptr);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    void render() override {
        shaderReloadWindow();

        ImGuiWindowCreator("Options")
            .heading("Info")
            .display("Time ", getElapsedTime())
            .heading("Settings")
            .slider("Column count", &m_columnCount, 5, 50)
            .slider("Anim speed", &m_animSpeed, 0.0f, 3.0f)
            .end();

        // Viewport size
        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);

        glUseProgram(m_programId);
        m_program.setFloat("u_time", getElapsedTime());
        m_program.setVec3f("u_color", m_color);
        m_program.setVec2f("u_windowDimensions", glm::vec2(viewport[2], viewport[3]));
        m_program.setInt("u_columnCount", m_columnCount);
        m_program.setFloat("u_animSpeed", m_animSpeed);
        glBindVertexArray(m_VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(0);
    }
};



#endif //ASSIGNMENT01_H
