//
// Created by slice on 12/21/24.
//

#ifndef PROJECT_H
#define PROJECT_H
#include "TerrainGenerator.h"
#include "TerrainManager.h"
#include "TerrainPatchLODGenerator.h"
#include "../FPSCamera.h"
#include "../OpenglUtils.h"
#include "../Renderer.h"
#include "../RenderBase.h"
#include "../RenderQueue.h"
#include "../Shaders/SkyboxShader/SkyboxShaderProgram.h"
#include "../Shaders/TerrainShader/TerrainShaderProgram.h"
#include "../GPUModelUploader.h"
#include "../Shaders/ModelShader/ModelShaderProgram.h"
#include "../Shaders/GrassShaderInstanced/GrassShaderInstancedProgram.h"
#include "../Shaders/WaterShader/WaterShaderProgram.h"


class Project : public RenderBase {
public:
    explicit Project(std::string_view title, FPSCamera &cam) : RenderBase(title), m_cam(cam) {
    }

    void init() override {
        RenderCall mc = {
            h.VAO, h.elementCount, GL_UNSIGNED_INT
        };

        // Terrain
        RenderEntity skybox = generateSkybox();
        // Terrain - Water

        geometry_utils::TriangulatedPlaneMesh waterMesh = geometry_utils::generateTriangulatedPlaneMesh(400, 2, true);
        GLuint waterMeshVao = geometry_utils::uploadTriangulatedPlaneMeshToGPU(waterMesh);
        RenderCall waterMeshRc = {
            waterMeshVao, (GLuint) waterMesh.indices.size(), GL_UNSIGNED_INT,
            {{TextureType::CUBE_MAP, m_skyboxHandle}}
        };

        GltfLoader loader{};
        GltfScene suzanne = loader.loadModel("../assets/models/tv.glb");
        GPUModelUploader uploader;
        std::vector<RenderCall> suzanneCalls = uploader.uploadGltfModel(suzanne);


        m_renderQueue
                .setShader(&m_skyboxShader)
                .addEntity("skybox", skybox)
                .setShader(&m_modelShader)
                .setShader(&m_waterShader)
                .addEntity("water", {waterMeshRc})
                .setShader(&m_modelShader)
                .addEntity("suzanne", {suzanneCalls});
        m_renderer.addRenderQueue(&m_renderQueue);
    }

    void render() override {
        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        m_program.setVec2f("u_windowDimensions", glm::vec2(viewport[2], viewport[3]));
        float aspectRatio = static_cast<float>(viewport[2]) / static_cast<float>(viewport[3]);
        glm::mat4 view = m_cam.getViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(m_cam.getFov()), aspectRatio, 0.1f, 500.0f);

        // Base Model
        glUseProgram(m_modelShader.getProgramId());
        m_modelShader.setMat4f("u_view", view);
        m_modelShader.setMat4f("u_projection", projection);
        m_modelShader.setVec3f("u_lightDirection", m_lightDirection);
        m_modelShader.setVec3f("u_cameraPos", m_cam.getCamPos());
        m_modelShader.setFloat("u_ambientIntensity", m_ambientIntensity);
        m_modelShader.setFloat("u_specularIntensity", m_specularIntensity);

        // Grass instancing
        glUseProgram(m_GrassShaderInstanced.getProgramId());
        m_GrassShaderInstanced.setMat4f("u_view", view);
        m_GrassShaderInstanced.setMat4f("u_projection", projection);
        m_GrassShaderInstanced.setVec3f("u_lightDirection", m_lightDirection);
        m_GrassShaderInstanced.setVec3f("u_cameraPos", m_cam.getCamPos());
        m_GrassShaderInstanced.setFloat("u_ambientIntensity", m_ambientIntensity);
        m_GrassShaderInstanced.setFloat("u_specularIntensity", m_specularIntensity);

        // Skybox
        glUseProgram(m_skyboxShader.getProgramId());
        m_skyboxShader.setMat4f("u_view", view);
        m_skyboxShader.setMat4f("u_projection", projection);

        // Water
        glUseProgram(m_waterShader.getProgramId());
        m_waterShader.setMat4f("u_view", view);
        m_waterShader.setMat4f("u_projection", projection);
        m_waterShader.setFloat("u_time", getElapsedTime());
        m_waterShader.setVec3f("u_camPos", m_cam.getCamPos());
        m_waterShader.setFloat("u_scale", m_terrainScale);
        m_waterShader.setFloat("u_persistance", m_terrainPersistence);
        m_waterShader.setFloat("u_lucunarity", m_terrainLucunarity);
        m_waterShader.setInt("u_octaves", m_terrainOctaves);
        m_waterShader.setVec3f("u_lightDirection", m_lightDirection);
        m_waterShader.setFloat("u_ambientIntensity", m_ambientIntensity);
        m_waterShader.setFloat("u_specularIntensity", m_specularIntensity);
        m_waterShader.setFloat("u_terrainHeight", m_terrainHeight);


        glm::vec3 localPos = glm::vec3(64.0f, 0.0f, 64.0f);
        const TerrainChunk &chunk = m_terrainManager.getTerrainChunk(2, 2);

        glm::vec3 normal = m_terrainManager.calculateNormal(localPos, chunk);
        glm::vec3 modelUp = glm::vec3(0.0f, 1.0f, 0.0f);
        glm::vec3 rotationAxis = glm::normalize(glm::cross(modelUp, normal));

        float rotationAngle = glm::acos(glm::dot(modelUp, normal));
        m_renderQueue["suzanne"].setTranslation(
            m_terrainManager.getWorldSpacePositionInChunk({64.0f, 64.0f}, chunk) + glm::vec3(0.0f, 1.0f, 0.0f));
        m_renderQueue["suzanne"].setRotationAxis(rotationAxis);
        m_renderQueue["suzanne"].setRotationAngle(glm::degrees(rotationAngle));
        m_renderQueue["suzanne"].setScale(glm::vec3{10.0f});

        glUseProgram(m_terrainShader.getProgramId());
        m_terrainShader.setMat4f("u_view", view);
        m_terrainShader.setMat4f("u_projection", projection);
        m_terrainShader.setVec3f("u_camPos", m_cam.getCamPos()); // Used for infinite terrain
        m_terrainShader.setFloat("u_scale", m_terrainScale);
        m_terrainShader.setFloat("u_persistance", m_terrainPersistence);
        m_terrainShader.setFloat("u_lucunarity", m_terrainLucunarity);
        m_terrainShader.setInt("u_octaves", m_terrainOctaves);
        m_terrainShader.setFloat("u_terrainHeight", m_terrainHeight);
        m_terrainShader.setVec3f("u_lightDirection", m_lightDirection);
        m_terrainShader.setVec3f("u_cameraPos", m_cam.getCamPos());
        m_terrainShader.setFloat("u_ambientIntensity", m_ambientIntensity);
        m_terrainShader.setFloat("u_specularIntensity", m_specularIntensity);
        m_terrainManager.update(m_cam.getCamPos());

        m_renderer.renderAllQueues();

        float height = m_terrainManager.getHeight(m_cam.getCamPos());

        //m_cam.updateHeight(height + 2.0f);

        ImGuiWindowCreator terrainWindow{"Terrain parameters"};
        terrainWindow
                .slider("Terrain height", &m_terrainHeight, 10.0f, 200.0f)
                .slider("Scale", &m_terrainScale, 1.0f, 300.0f)
                .slider("Persistance", &m_terrainPersistence, 0.1f, 1.0f)
                .slider("Lucunarity", &m_terrainLucunarity, 1.0f, 10.0f)
                .slider("Octaves", &m_terrainOctaves, 1, 10)
                .slider("Light", &m_lightDirection, 0, 100);

        if (ImGui::Button("Toggle Wireframe")) {
            toggleTerrainWireframe();
        }

        terrainWindow.end();
    }

private:
    FPSCamera &m_cam;
    //TerrainGenerator m_terrainGenerator{400, 1};
    Renderer m_renderer;
    RenderQueue m_renderQueue{"scene"};
    GLuint m_skyboxHandle;
    TerrainPatchHandle h;
    MeshBufferInfo test;
    GLuint ssbo, ebo, vao;
    float debug{0.0};

    // Shaders
    ModelShaderProgram m_modelShader;
    GrassShaderInstancedProgram m_GrassShaderInstanced;
    SkyboxShaderProgram m_skyboxShader;
    TerrainShaderProgram m_terrainShader;
    WaterShaderProgram m_waterShader;

    // Lighting
    glm::vec3 m_lightDirection{1.0f, 100.0f, 1.0f};
    float m_ambientIntensity = 0.3f;
    float m_specularIntensity = 64.0f;

    // Terrain
    bool m_terrainWireframe{false};
    float m_terrainHeight{30.0f};
    float m_terrainScale{300.0f};
    float m_terrainPersistence{0.244f};
    float m_terrainLucunarity{10.0f};
    int m_terrainOctaves{4};
    TerrainManager m_terrainManager{
        256, m_terrainShader, m_GrassShaderInstanced, m_terrainHeight, m_terrainOctaves, m_terrainScale, m_terrainPersistence,
        m_terrainLucunarity
    };

    void toggleTerrainWireframe() {
        m_terrainWireframe = !m_terrainWireframe;

        if (m_terrainWireframe) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        } else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
    }

    // Used to setup base uniforms common in majority of shaders
    // Most for projection, lighting
    void setBaseUniforms(BaseShaderProgram const *shader, float aspectRatio, const glm::mat4 &view,
                         const glm::mat4 &projection) {
        shader->setMat4f("u_view", view);
        shader->setMat4f("u_projection", projection);
        shader->setVec3f("u_lightDirection", m_lightDirection);
        shader->setVec3f("u_cameraPos", m_cam.getCamPos());
        shader->setFloat("u_ambientIntensity", m_ambientIntensity);
        shader->setFloat("u_specularIntensity", m_specularIntensity);
    }

    RenderEntity generateSkybox() {
        // Get a cube primitive and generate required VAO
        PrimitiveData cubePrimitive = opengl_utils::getPrimitive(PrimitiveType::CUBE);

        VaoHandle cubeHandle = opengl_utils::generateVao(
            VertexAttribArray{cubePrimitive.positions, 3},
            IndexBufferArray{cubePrimitive.ebo}
        );

        // Get the cube map texture
        TextureHandle skyboxCubemap = opengl_utils::loadCubemap({
            "../assets/textures/cubemap_cloudy/posx.png", // Right
            "../assets/textures/cubemap_cloudy/negx.png", // Left
            "../assets/textures/cubemap_cloudy/posy.png", // Top
            "../assets/textures/cubemap_cloudy/negy.png", // Bottom
            "../assets/textures/cubemap_cloudy/posz.png", // Front
            "../assets/textures/cubemap_cloudy/negz.png", // Back
        });

        // Store handle so it can be used for environment effects
        m_skyboxHandle = skyboxCubemap.handle;

        // Create the RenderEntity, cube as geometry, cube map as texture
        RenderEntity skybox = {
            RenderCall{
                cubeHandle.id, cubeHandle.elemenCount, GL_UNSIGNED_INT,
                {{TextureType::CUBE_MAP, skyboxCubemap.handle}}
            },
        };

        return skybox;
    }
};


#endif //PROJECT_H
