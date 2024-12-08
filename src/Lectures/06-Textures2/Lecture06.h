//
// Created by slice on 12/6/24.
//

#ifndef LECTURE06_H
#define LECTURE06_H
#include "../../OpenglUtils.h"
#include "../../OrbitCamera.h"
#include "../../RenderBase.h"
#include "../../RenderQueue.h"
#include "../../Renderer.h"
#include "../../SkyboxShaderProgram.h"


class Lecture06 : public RenderBase {
public:
    explicit Lecture06(std::string_view title, OrbitCamera &cam) : RenderBase(title), m_camera(cam) {
    }

    void init() override {
        // Skybox
        PrimitiveData cubePrimitive = opengl_utils::getPrimitive(PrimitiveType::CUBE);

        m_cubeHandle = opengl_utils::generateVao(
            VertexAttribArray{cubePrimitive.positions, 3},
            IndexBufferArray{cubePrimitive.ebo}
        );

        m_textureHandleCube = opengl_utils::loadCubemap({
            "../assets/textures/cubemap_beach/posx.jpg", // Right
            "../assets/textures/cubemap_beach/negx.jpg", // Left
            "../assets/textures/cubemap_beach/posy.jpg", // Top
            "../assets/textures/cubemap_beach/negy.jpg", // Bottom
            "../assets/textures/cubemap_beach/posz.jpg", // Front
            "../assets/textures/cubemap_beach/negz.jpg", // Back
        });

        GltfLoader loader{};
        GltfScene suzanne = loader.loadModel("../assets/models/suzanne/Suzanne.gltf");
        GPUModelUploader uploader;
        std::vector<RenderCall> suzanneCalls = uploader.uploadGltfModel(suzanne);

        RenderEntity skybox = {
            RenderCall{
                m_cubeHandle.id, m_cubeHandle.elemenCount, GL_UNSIGNED_INT,
                {{TextureType::CUBE_MAP, m_textureHandleCube.handle}}
            },
        };

        m_renderQueue
                .setShader(&m_modelShader)
                .addEntity("suzanne", {suzanneCalls})
                .setShader(&m_skyboxShader)
                .addEntity("skybox", skybox)
                .setShader(&m_modelShader)
                .addEntity("suzanne2", {suzanneCalls, glm::vec3{0.0f, 2.0f, 0.0f}});

        m_renderer.addRenderQueue(&m_renderQueue);
    }

    void render() override {
        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        m_program.setVec2f("u_windowDimensions", glm::vec2(viewport[2], viewport[3]));
        float aspectRatio = static_cast<float>(viewport[2]) / static_cast<float>(viewport[3]);

        glm::mat4 view = m_camera.getViewMatrix();
        glm::mat4 projection = glm::perspective(glm::quarter_pi<float>(), aspectRatio, 0.1f, 100.0f);

        glUseProgram(m_modelShader.getProgramId());
        m_modelShader.setMat4f("u_view", view);
        m_modelShader.setMat4f("u_projection", projection);
        m_modelShader.setVec3f("u_lightDirection", m_lightDirection);
        m_modelShader.setVec3f("u_cameraPos", m_camera.getCamPos());
        m_modelShader.setFloat("u_ambientIntensity", m_ambientIntensity);
        m_modelShader.setFloat("u_specularIntensity", m_specularIntensity);

        glUseProgram(m_skyboxShader.getProgramId());
        m_skyboxShader.setMat4f("u_view", view);
        m_skyboxShader.setMat4f("u_projection", projection);

        m_renderQueue["suzanne"].setRotationAngle((glm::sin(getElapsedTime())) * 90.0f);
        m_renderer.renderAllQueues();
    }

private:
    RenderQueue m_renderQueue{"Scene"};
    ModelShaderProgram m_modelShader{
        "../src/Lectures/06-Textures2/shader.vert",
        "../src/Lectures/06-Textures2/shader.frag"
    };
    SkyboxShaderProgram m_skyboxShader{
        "../src/Lectures/06-Textures2/skybox.vert",
        "../src/Lectures/06-Textures2/skybox.frag"
    };
    Renderer m_renderer;
    TextureHandle m_textureHandle;
    TextureHandle m_textureHandleCube;
    VaoHandle m_sphereHandle;
    VaoHandle m_cubeHandle;
    OrbitCamera &m_camera;
    glm::vec3 m_lightDirection{1.0f, 1.0f, 1.0f};
    float m_ambientIntensity = 0.1f;
    float m_specularIntensity = 64.0f;
};


#endif //LECTURE06_H
