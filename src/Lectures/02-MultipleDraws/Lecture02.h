///
// Created by slice on 11/14/24.
///

#ifndef LECTURE02_H
#define LECTURE02_H

#include "../../OpenglUtils.h"
#include "../../RenderBase.h"

class Lecture02 : public RenderBase {
private:
    VaoHandle m_handleQuad;
    VaoHandle m_handleTri;
public:
    explicit Lecture02(std::string_view title) : RenderBase(title) {}

    void init() override {
        compileShaders("../src/Lectures/02-MultipleDraws/shader.vert",
        "../src/Lectures/02-MultipleDraws/shader.frag");

        std::vector<GLfloat> values = {
            -1.0f,  1.0f,  0.0f, // Top left
             1.0f,  1.0f,  0.0f, // Top right
            -1.0f, -1.0f,  0.0f, // Bottom left
             1.0f, -1.0f,  0.0f  // Bottom right
        };

        m_handleQuad = opengl_utils::generateVao(
            VertexAttribArray{
                values, 3
            },
            VertexAttribArray{
                {
                         1.0f, 0.0f, 0.0f,
                         0.0f, 1.0f, 0.0f,
                         1.0f, 0.0f, 0.0f,
                         0.0f, 1.0f, 0.0f
                    }, 3
            },
            IndexBufferArray {
                2, 1, 0,
                2, 3, 1
            }
        );

        m_handleTri = opengl_utils::generateVao(
            VertexAttribArray{
                {
                    -0.5f, -1.0f, 0.0f,
                    0.5f, -1.0f, 0.0f,
                    0.0f,  0.0f, 0.0f
                }, 3
            },
            VertexAttribArray{
                {
                        1.0f, 0.0f, 0.0f,
                        0.0f, 1.0f, 0.0f,
                        0.0f, 0.0f, 1.0f
                    }, 3
            }
        );
    }

    void render() override {

        glUseProgram(m_programId);
        glBindVertexArray(m_handleQuad.id);
        glDrawElements(GL_TRIANGLES, m_handleQuad.elemenCount, GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(m_handleTri.id);
        glDrawArrays(GL_TRIANGLES, 0, m_handleTri.elemenCount);
        glBindVertexArray(0);

    }
};

#endif // LECTURE02_H