//
// Created by slice on 11/30/24.
//

#ifndef LECTURE05_H
#define LECTURE05_H
#include "../../GPUModelUploader.h"
#include "../../OrbitCamera.h"
#include "../../RenderBase.h"


class Lecture05 : public RenderBase {
private:
    std::vector<RenderCall> m_renderCalls;
    OrbitCamera &m_camera;
    glm::vec3 m_lightDirection{1.0f, 1.0f, 1.0f};
    float m_ambientIntensity = 0.03f;
    float m_specularIntensity = 64.0f;
public:
    explicit Lecture05(std::string_view title, OrbitCamera &cam) : RenderBase(title), m_camera(cam) {}

    void init() override {
        compileShaders("../src/Lectures/05-Textures/shader.vert",
                       "../src/Lectures/05-Textures/shader.frag");

        GltfLoader loader{};
        GltfScene suzanne = loader.loadModel("../models/suzanne/Suzanne.gltf");
        GPUModelUploader uploader;
        m_renderCalls = uploader.uploadGltfModel(suzanne);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glEnable(GL_DEPTH_TEST);
    }

    void render() override {
        glUseProgram(m_programId);

        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        m_program.setVec2f("u_windowDimensions", glm::vec2(viewport[2], viewport[3]));
        float aspectRatio = static_cast<float>(viewport[2]) / static_cast<float>(viewport[3]);

        glm::mat4 model = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f));
        //model = glm::translate(model, glm::vec3(0.0f, -70.0f, -20.0f));
        //model = glm::rotate(model, getElapsedTime(), glm::vec3{0.0f, 1.0f, 0.0f});
        m_program.setMat4f("u_model", model);

        glm::mat4 view = m_camera.getViewMatrix();
        m_program.setMat4f("u_view", view);

        glm::mat4 projection = glm::perspective(glm::quarter_pi<float>(), aspectRatio, 0.1f, 100.0f);
        m_program.setMat4f("u_projection", projection);

        m_program.setVec3f("u_lightDirection", m_lightDirection);
        m_program.setVec3f("u_cameraPos", m_camera.getCamPos());
        m_program.setFloat("u_ambientIntensity", m_ambientIntensity);
        m_program.setFloat("u_specularIntensity", m_specularIntensity);
        m_program.setInt("u_texDiffuse", 0);

        for (RenderCall call : m_renderCalls) {
            glBindVertexArray(call.vao);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, call.textureDiffuse);
            glDrawElements(GL_TRIANGLES, call.elemCount, call.componentType, nullptr);
            glBindVertexArray(0);
        }
    }
};



#endif //LECTURE05_H
