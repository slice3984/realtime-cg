//
// Created by slice on 12/15/24.
//

#ifndef LECTURE07_H
#define LECTURE07_H
#include "../../FPSCamera.h"
#include "../../OpenglUtils.h"
#include "../../Renderer.h"
#include "../../RenderQueue.h"
#include "../../RenderBase.h"
#include "../../Shaders/ModelShader/ModelShaderProgram.h"
#include "../../Shaders/SkyboxShader/SkyboxShaderProgram.h"


class Lecture07 : public RenderBase {
public:
    explicit Lecture07(std::string_view title, FPSCamera &cam) : RenderBase(title), m_camera(cam) {
    }

    void init() override {
        PrimitiveData planePrimitive = opengl_utils::getPrimitive(PrimitiveType::PLANE);

        m_planeHandle = opengl_utils::generateVao(
            VertexAttribArray{planePrimitive.positions, 3},
            VertexAttribArray{planePrimitive.normals, 3},
            VertexAttribArray{planePrimitive.tangents, 4},
            VertexAttribArray{planePrimitive.texCoords, 2},
            IndexBufferArray{planePrimitive.ebo}
        );

        ImageData floorImg = opengl_utils::loadImage("../assets/textures/granite_floor.jpg");
        TextureHandle tex = opengl_utils::createTexture(floorImg);
        opengl_utils::updateTextureData(tex, floorImg);

        RenderEntity plane = {
            RenderCall{
                m_planeHandle.id, m_planeHandle.elemenCount, GL_UNSIGNED_INT,
                {{ TextureType::DIFFUSE, tex.handle }}
            }
        };


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

        RenderEntity skybox = {
            RenderCall{
                m_cubeHandle.id, m_cubeHandle.elemenCount, GL_UNSIGNED_INT,
                {{TextureType::CUBE_MAP, m_textureHandleCube.handle}}
            },
        };

        m_renderQueue
                .setShader(&m_modelShader)
                .addEntity("plane", plane)
                .setShader(&m_skyboxShader)
                .addEntity("skybox", skybox);

        m_renderer.addRenderQueue(&m_renderQueue);
    }

    void render() override {
        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        m_program.setVec2f("u_windowDimensions", glm::vec2(viewport[2], viewport[3]));
        float aspectRatio = static_cast<float>(viewport[2]) / static_cast<float>(viewport[3]);

        glm::mat4 view = m_camera.getViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(m_camera.getFov()), aspectRatio, 0.1f, 100.0f);

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

        m_renderQueue["plane"].setScale(glm::vec3{50.0f});
        m_renderer.renderAllQueues();
    }

private:
    FPSCamera &m_camera;
    ModelShaderProgram m_modelShader;
    SkyboxShaderProgram m_skyboxShader;
    Renderer m_renderer;
    RenderQueue m_renderQueue{"Scene"};
    TextureHandle m_textureHandleCube;
    TextureHandle m_textureHandlePlane;
    VaoHandle m_cubeHandle;
    VaoHandle m_planeHandle;
    glm::vec3 m_lightDirection{1.0f, 1.0f, 1.0f};
    float m_ambientIntensity = 0.3f;
    float m_specularIntensity = 64.0f;
};


#endif //LECTURE07_H
