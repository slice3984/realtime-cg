//
// Created by slice on 12/6/24.
//

#ifndef LECTURE06_H
#define LECTURE06_H
#include "../../OpenglUtils.h"
#include "../../OrbitCamera.h"
#include "../../RenderBase.h"


class Lecture06 : public RenderBase {
public:
    explicit Lecture06(std::string_view title, OrbitCamera &cam) : RenderBase(title), m_camera(cam) {}

    void init() override {
        compileShaders("../src/Lectures/06-Textures2/shader.vert",
                "../src/Lectures/06-Textures2/shader.frag");

        PrimitiveData spherePrimitive = opengl_utils::getPrimitive(PrimitiveType::SPHERE);
        m_sphereHandle = opengl_utils::generateVao(
            VertexAttribArray{ spherePrimitive.positions, 3 },
            VertexAttribArray{ spherePrimitive.normals, 3 },
            VertexAttribArray{ spherePrimitive.texCoords, 2 },
            IndexBufferArray{ spherePrimitive.ebo }
        );

        ImageData imageSphere = opengl_utils::loadImage("../assets/textures/sphere_test_diffuse.png");
        m_textureHandle = opengl_utils::createTexture(imageSphere);
        opengl_utils::updateTextureData(m_textureHandle, imageSphere);

        // Skybox
        PrimitiveData cubePrimitive = opengl_utils::getPrimitive(PrimitiveType::CUBE);

        m_cubeHandle = opengl_utils::generateVao(
            VertexAttribArray{ cubePrimitive.positions, 3 },
            IndexBufferArray{ cubePrimitive.ebo }
        );

        m_textureHandleCube = opengl_utils::loadCubemap({
            "../assets/textures/cubemap_beach/posx.jpg",  // Right
            "../assets/textures/cubemap_beach/negx.jpg",  // Left
            "../assets/textures/cubemap_beach/posy.jpg",  // Top
            "../assets/textures/cubemap_beach/negy.jpg",  // Bottom
            "../assets/textures/cubemap_beach/posz.jpg",  // Front
            "../assets/textures/cubemap_beach/negz.jpg",  // Back
        });


        // Skybox shader
        Shader vsSky{"../src/Lectures/06-Textures2/skybox.vert", GL_VERTEX_SHADER};
        vsSky.compile();

        Shader fsSky{"../src/Lectures/06-Textures2/skybox.frag", GL_FRAGMENT_SHADER};
        fsSky.compile();

        m_progSky.attachShader(vsSky);
        m_progSky.attachShader(fsSky);

        m_progSky.linkProgram();

        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
    }

    void render() override {
        glUseProgram(m_programId);

        glCullFace(GL_BACK);


        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        m_program.setVec2f("u_windowDimensions", glm::vec2(viewport[2], viewport[3]));
        float aspectRatio = static_cast<float>(viewport[2]) / static_cast<float>(viewport[3]);

        glm::mat4 model = glm::mat4{1.0f};
        model = glm::rotate(model, (getElapsedTime() / 1), glm::vec3{0.0f, 1.0f, 0.0f});
        m_program.setMat4f("u_model", model);

        glm::mat4 view = m_camera.getViewMatrix();
        m_program.setMat4f("u_view", view);

        glm::mat4 projection = glm::perspective(glm::quarter_pi<float>(), aspectRatio, 0.1f, 100.0f);
        m_program.setMat4f("u_projection", projection);

        m_program.setVec3f("u_lightDirection", m_lightDirection);
        m_program.setVec3f("u_cameraPos", m_camera.getCamPos());
        m_program.setFloat("u_ambientIntensity", m_ambientIntensity);
        m_program.setFloat("u_specularIntensity", m_specularIntensity);
        m_program.setInt("u_diffuseTex", 0);

        glBindVertexArray(m_sphereHandle.id);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_textureHandle.handle);
        glDrawElements(GL_TRIANGLES, m_sphereHandle.elemenCount, GL_UNSIGNED_INT, nullptr);

        // Skybox
        //glDepthMask(GL_FALSE);
        glUseProgram(m_progSky.getProgramId());
        glBindVertexArray(m_cubeHandle.id);
        glCullFace(GL_FRONT);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, m_textureHandleCube.handle);
view = glm::mat4(glm::mat3(m_camera.getViewMatrix()));
        m_progSky.setMat4f("u_view", view);
        m_progSky.setMat4f("u_projection", projection);
        m_progSky.setInt("u_skybox", 0);

        glDrawElements(GL_TRIANGLES, m_cubeHandle.elemenCount, GL_UNSIGNED_INT, nullptr);
        //glDepthMask(GL_TRUE);
    }
private:
    ShaderProgram m_progSky;
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
