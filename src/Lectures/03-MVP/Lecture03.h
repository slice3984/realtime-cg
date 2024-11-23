//
// Created by slice on 11/22/24.
//

#ifndef LECTURE03_H
#define LECTURE03_H
#include "../../RenderBase.h"
#include "../../GltfLoader.h"
#include "../../OpenglUtils.h"

class Lecture03 : public RenderBase {
private:
    VaoHandle m_handleQuad;
    VaoHandle m_handleCube;
    OrbitCamera &m_camera;

public:
    explicit Lecture03(std::string_view title, OrbitCamera &cam) : RenderBase(title), m_camera(cam) {
    }

    void init() override {
        GltfLoader loader;
        GltfScene scene = loader.loadModel("../models/lantern/Lantern.gltf");

        auto data = opengl_utils::getPrimitive(PrimitiveType::CUBE);


        compileShaders("../src/Lectures/03-MVP/shader.vert",
                       "../src/Lectures/03-MVP/shader.frag");

        m_handleCube = opengl_utils::generateVao(
            VertexAttribArray{
                data.position, 3
            },
            VertexAttribArray{
                data.color, 4
            },
            IndexBufferArray{
                data.ebo
            }
        );

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glEnable(GL_DEPTH_TEST);
    }

    void render() override {
        shaderReloadWindow();
        glUseProgram(m_programId);
        glBindVertexArray(m_handleCube.id);

        // Viewport: Get the current window dimensions
        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        m_program.setVec2f("u_windowDimensions", glm::vec2(viewport[2], viewport[3]));
        float aspectRatio = static_cast<float>(viewport[2]) / static_cast<float>(viewport[3]);

        glm::mat4 model = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f));
        m_program.setMat4f("u_model", model);

        glm::mat4 view = m_camera.getViewMatrix();
        m_program.setMat4f("u_view", view);

        glm::mat4 projection = glm::perspective(glm::quarter_pi<float>(), aspectRatio, 0.1f, 100.0f);
        m_program.setMat4f("u_projection", projection);

        glDrawElements(GL_TRIANGLES, m_handleCube.elemenCount, GL_UNSIGNED_INT, nullptr);

        model = glm::translate(model, glm::vec3{3.0f, 0.0f, 0.0f});
        m_program.setMat4f("u_model", model);

        glDrawElements(GL_TRIANGLES, m_handleCube.elemenCount, GL_UNSIGNED_INT, nullptr);
    }
};


#endif //LECTURE03_H
