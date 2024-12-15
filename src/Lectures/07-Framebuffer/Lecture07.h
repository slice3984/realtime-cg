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
#include "../../Shaders/TilingShader/TilingShaderProgram.h"


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
        TextureHandle tex = opengl_utils::createTexture(floorImg, true);
        opengl_utils::updateTextureData(tex, floorImg);

        RenderEntity plane = {
            RenderCall{
                m_planeHandle.id, m_planeHandle.elemenCount, GL_UNSIGNED_INT,
                {{ TextureType::DIFFUSE, tex.handle }}
            }
        };

        glm::vec2 screenBufferSize{1280, 960};
        TextureHandle screenTex = opengl_utils::createTexture(screenBufferSize.x, screenBufferSize.y);

        RenderEntity screen = {
            RenderCall{
                m_planeHandle.id, m_planeHandle.elemenCount, GL_UNSIGNED_INT,
                { {TextureType::DIFFUSE, screenTex.handle}}
            },
            glm::vec3{0.0f, 10.0f, -20.0f},
            glm::vec3{16.0f, 1.0f, 9.0f},
            glm::vec3{1.0f, 0.0f, 0.0f},
            90.0f
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
                .setShader(&m_tilingShader)
                .addEntity("plane", plane)
                .setShader(&m_modelShader)
                .addEntity("suzanne", {suzanneCalls, glm::vec3{0.0f, 2.0f, 0.0f}})
                .setShader(&m_skyboxShader)
                .addEntity("skybox", skybox);

        m_renderQueueScreen
            .setShader(&m_modelShader)
            .addEntity("screen", screen);

        m_renderer.addRenderQueue(&m_renderQueue);
        m_renderer.addRenderQueue(&m_renderQueueScreen);

        // Renderbuffer
        GLuint screenDepth;
        glGenRenderbuffers(1, &screenDepth);
        glBindRenderbuffer(GL_RENDERBUFFER, screenDepth);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, screenBufferSize.x, screenBufferSize.y);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);

        // Framebuffer
        glGenFramebuffers(1, &m_screenFrameBuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, m_screenFrameBuffer);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, screenTex.handle, 0);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, screenDepth);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cerr << "Frame buffer check failed" << std::endl;
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
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

        glUseProgram(m_tilingShader.getProgramId());
        m_tilingShader.setMat4f("u_view", view);
        m_tilingShader.setMat4f("u_projection", projection);
        m_tilingShader.setVec3f("u_lightDirection", m_lightDirection);
        m_tilingShader.setVec3f("u_cameraPos", m_camera.getCamPos());
        m_tilingShader.setFloat("u_ambientIntensity", m_ambientIntensity);
        m_tilingShader.setFloat("u_specularIntensity", m_specularIntensity);
        m_tilingShader.setVec2f("u_tilingFactor", glm::vec2{25.0f});

        glUseProgram(m_skyboxShader.getProgramId());
        m_skyboxShader.setMat4f("u_view", view);
        m_skyboxShader.setMat4f("u_projection", projection);

        m_renderQueue["plane"].setScale(glm::vec3{50.0f});
        m_renderer.renderQueue("Scene");

        // Framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, m_screenFrameBuffer);
        glClear(GL_COLOR_BUFFER_BIT  | GL_DEPTH_BUFFER_BIT);
        m_renderer.renderQueue("Scene");
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        m_renderer.renderQueue("Screen");
    }

private:
    FPSCamera &m_camera;
    ModelShaderProgram m_modelShader;
    SkyboxShaderProgram m_skyboxShader;
    TilingShaderProgram m_tilingShader;
    Renderer m_renderer;
    GLuint m_screenFrameBuffer;
    RenderQueue m_renderQueue{"Scene"};
    RenderQueue m_renderQueueScreen{"Screen"};
    TextureHandle m_textureHandleCube;
    TextureHandle m_textureHandlePlane;
    VaoHandle m_cubeHandle;
    VaoHandle m_planeHandle;
    glm::vec3 m_lightDirection{1.0f, 1.0f, 1.0f};
    float m_ambientIntensity = 0.3f;
    float m_specularIntensity = 64.0f;
};


#endif //LECTURE07_H
