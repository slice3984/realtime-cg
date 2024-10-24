//
// Created by slice on 10/24/24.
//

#ifndef LECTURE01_H
#define LECTURE01_H
#include "glad/glad.h"
#include "../LectureBase.h"
#include "../../ShaderProgram.h"


class Lecture01 : public  LectureBase {
private:
    GLuint m_programId;
    GLuint m_VAO;
public:
    explicit Lecture01(std::string_view title) : LectureBase(title) {
    }

    void init() override {
        GLfloat vertices[] = {
            -0.5f, -0.5f, 0.0f,
             0.5f, -0.5f, 0.0f,
             0.0f,  0.5f, 0.0f
        };

        GLuint indices[] = {
            0, 1, 2
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

        Shader vertShader{"../src/Lectures/01-Triangle/shader.vert", GL_VERTEX_SHADER};
        vertShader.compile();

        Shader fragShader{"../src/Lectures/01-Triangle/shader.frag", GL_FRAGMENT_SHADER};
        fragShader.compile();

        ShaderProgram program{};
        program.attachShader(vertShader);
        program.attachShader(fragShader);
        program.linkProgram();

        m_programId = program.getProgramId();

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)nullptr);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    void render() override {
        glUseProgram(m_programId);
        glBindVertexArray(m_VAO);
        glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(0);
    }
};



#endif //LECTURE01_H
