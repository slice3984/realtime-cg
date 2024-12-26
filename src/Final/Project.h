//
// Created by slice on 12/21/24.
//

#ifndef PROJECT_H
#define PROJECT_H
#include "TerrainGenerator.h"
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
#include "../Shaders/WaterShader/WaterShaderProgram.h"


class Project : public RenderBase {
public:
    explicit  Project(std::string_view title, FPSCamera &cam) : RenderBase(title), m_cam(cam) {}

    void init() override {
        TerrainPatch p = TerrainPatchLODGenerator::generateBasePatch(1024 * 2, 2);
        TerrainPatchLODGenerator::stitchPatchEdge(p, STITCHED_EDGE::TOP);
        //TerrainPatchLODGenerator::stitchPatchEdge(p, STITCHED_EDGE::BOTTOM);
        //TerrainPatchLODGenerator::stitchPatchEdge(p, STITCHED_EDGE::LEFT);
        //TerrainPatchLODGenerator::stitchPatchEdge(p, STITCHED_EDGE::RIGHT);


        h = TerrainPatchLODGenerator::generateTerrainPatchHandle(p);

        RenderCall mc = {
            h.VAO, h.elementCount, GL_UNSIGNED_INT
        };
        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        // Terrain
        RenderEntity skybox = generateSkybox();
        // Terrain - Water

        geometry_utils::TriangulatedPlaneMesh waterMesh = geometry_utils::generateTriangulatedPlaneMesh(400, 2, true);
        GLuint waterMeshVao = geometry_utils::uploadTriangulatedPlaneMeshToGPU(waterMesh);
        RenderCall waterMeshRc = {
            waterMeshVao, (GLuint) waterMesh.indices.size(), GL_UNSIGNED_INT,
            { { TextureType::CUBE_MAP, m_skyboxHandle }}
        };

        m_renderQueue
        .setShader(&m_skyboxShader)
        .addEntity("skybox", skybox)
        .setShader(&m_modelShader)
        .addEntity("mesh", {
            mc
        });

        /*
        .setShader(&m_terrainShader)
        .addEntity("terrain", { m_terrainGenerator.getRenderCall() })
        .setShader(&m_waterShader)
        .addEntity("water", { waterMeshRc });
*/
        m_renderer.addRenderQueue(&m_renderQueue);
    }

    void render() override {
        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        m_program.setVec2f("u_windowDimensions", glm::vec2(viewport[2], viewport[3]));
        float aspectRatio = static_cast<float>(viewport[2]) / static_cast<float>(viewport[3]);
        glm::mat4 view = m_cam.getViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(m_cam.getFov()), aspectRatio, 0.1f, 500.0f);

        glUseProgram(m_modelShader.getProgramId());
        m_modelShader.setMat4f("u_view", view);
        m_modelShader.setMat4f("u_projection", projection);
        m_modelShader.setVec3f("u_lightDirection", m_lightDirection);
        m_modelShader.setVec3f("u_cameraPos", m_cam.getCamPos());
        m_modelShader.setFloat("u_ambientIntensity", m_ambientIntensity);
        m_modelShader.setFloat("u_specularIntensity", m_specularIntensity);


        glUseProgram(m_skyboxShader.getProgramId());
        m_skyboxShader.setMat4f("u_view", view);
        m_skyboxShader.setMat4f("u_projection", projection);

        glUseProgram(m_terrainShader.getProgramId());
        m_terrainShader.setMat4f("u_view", view);
        m_terrainShader.setMat4f("u_projection", projection);
        m_terrainShader.setVec3f("u_camPos", m_cam.getCamPos()); // Used for infinite terrain
        m_terrainShader.setFloat("u_scale", m_terrainScale);
        m_terrainShader.setFloat("u_persistance", m_terrainPersistence);
        m_terrainShader.setFloat("u_lucunarity", m_terrainLucunarity);
        m_terrainShader.setInt("u_octaves", m_terrainOctaves);
        m_terrainShader.setFloat("u_terrainHeight", m_terrainHeight);

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



        float height = m_terrainGenerator.getHeight(
            m_cam.getCamPos(),
            m_terrainHeight,
            m_terrainOctaves,
            m_terrainScale,
            m_terrainPersistence,
            m_terrainLucunarity
            );

        //m_cam.updateHeight(height + 1.0f);

        ImGuiWindowCreator{"Terrain parmeters"}
        .slider("Terrain height", &m_terrainHeight, 10.0f, 200.0f)
        .slider("Scale", &m_terrainScale, 1.0f, 300.0f)
        .slider("Persistance", &m_terrainPersistence, 0.1f, 1.0f)
        .slider("Lucunarity", &m_terrainLucunarity, 1.0f, 10.0f)
        .slider("Octaves", &m_terrainOctaves, 1, 10)
        .end();

        m_renderer.renderAllQueues();
    }

private:
    FPSCamera &m_cam;
    TerrainGenerator m_terrainGenerator{400, 1};
    Renderer m_renderer;
    RenderQueue m_renderQueue{"scene"};
    GLuint m_skyboxHandle;
    TerrainPatchHandle h;

    // Shaders
    ModelShaderProgram m_modelShader;
    SkyboxShaderProgram m_skyboxShader;
    TerrainShaderProgram m_terrainShader;
    WaterShaderProgram m_waterShader;

    // Lighting
    glm::vec3 m_lightDirection{1.0f, 100.0f, 1.0f};
    float m_ambientIntensity = 0.3f;
    float m_specularIntensity = 64.0f;

    // Terrain parameters
    float m_terrainHeight{30.0f};
    float m_terrainScale{135.0f};
    float m_terrainPersistence{0.244f};
    float m_terrainLucunarity{10.0f};
    int m_terrainOctaves{4};

    // Used to setup base uniforms common in majority of shaders
    // Most for projection, lighting
    void setBaseUniforms(BaseShaderProgram const *shader, float aspectRatio, const glm::mat4 &view, const glm::mat4 &projection) {
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
            VertexAttribArray{ cubePrimitive.positions, 3 },
            IndexBufferArray{ cubePrimitive.ebo }
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
