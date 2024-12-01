//
// Created by slice on 11/29/24.
//

#ifndef LECTURE04_H
#define LECTURE04_H
#include "../../OpenglUtils.h"
#include "../../OrbitCamera.h"
#include "../../RenderBase.h"


class Lecture04 : public RenderBase {
private:
    VaoHandle m_handleSphere;
    glm::vec3 m_lightDirection{1.0f, 1.0f, 1.0f};
    float m_ambientIntensity = 0.03f;
    float m_specularIntensity = 64.0f;
    OrbitCamera &m_camera;
public:
    explicit Lecture04(std::string_view title, OrbitCamera &cam) : RenderBase(title), m_camera(cam) {}

    void init() override {
        compileShaders("../src/Lectures/04-Lighting/shader.vert",
                       "../src/Lectures/04-Lighting/shader.frag");

        auto data = opengl_utils::getPrimitive(PrimitiveType::SPHERE);

        m_handleSphere = opengl_utils::generateVao(
            VertexAttribArray{ data.position, 3 },
            VertexAttribArray{ data.color, 4 },
            VertexAttribArray{ data.normal, 3 },
            IndexBufferArray{ data.ebo }
        );

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glEnable(GL_DEPTH_TEST);
    }

    void render() override {
        shaderReloadWindow();

        ImGuiWindowCreator{"Light parameters"}
            .input("Light direction", &m_lightDirection)
            .slider("Ambient Intensity", &m_ambientIntensity, 0.0f, 1.0f)
            .slider("Specular Intensity", &m_specularIntensity, 16.0f, 512.0f)
            .end();

        glUseProgram(m_programId);
        glBindVertexArray(m_handleSphere.id);

        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        m_program.setVec2f("u_windowDimensions", glm::vec2(viewport[2], viewport[3]));
        float aspectRatio = static_cast<float>(viewport[2]) / static_cast<float>(viewport[3]);

        glm::mat4 model = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f));
        model = glm::rotate(model, 0.6f, {1.0f, 0.0f, 0.0f});
        m_program.setMat4f("u_model", model);

        glm::mat4 view = m_camera.getViewMatrix();
        m_program.setMat4f("u_view", view);

        glm::mat4 projection = glm::perspective(glm::quarter_pi<float>(), aspectRatio, 0.1f, 100.0f);
        m_program.setMat4f("u_projection", projection);

        m_program.setVec3f("u_lightDirection", m_lightDirection);
        m_program.setVec3f("u_cameraPos", m_camera.getCamPos());
        m_program.setFloat("u_ambientIntensity", m_ambientIntensity);
        m_program.setFloat("u_specularIntensity", m_specularIntensity);

        glDrawElements(GL_TRIANGLES, m_handleSphere.elemenCount, GL_UNSIGNED_INT, nullptr);
    }
};



#endif //LECTURE04_H
