//
// Created by slice on 10/24/24.
//

#ifndef BASESHADERPROGRAM_H
#define BASESHADERPROGRAM_H

#include "../Shader.h"
#include "../../Linking/include/glad/glad.h"
#include <../../external/glm/glm/glm.hpp>
#include <../../external/glm/glm/gtc/type_ptr.hpp>

#include "../RenderEntity.h"

class BaseShaderProgram {
    struct GLStateDescriptor {
        bool cullFaceEnabled = false;
        GLenum cullFaceMode = GL_BACK;

        bool depthTestEnabled = true;
        GLenum depthFunc = GL_LESS;

        bool depthMaskEnabled = true;

        bool blendingEnabled = false;
        GLenum blendSrc = GL_SRC_ALPHA;
        GLenum blendDst = GL_ONE_MINUS_SRC_ALPHA;

        void apply() const {
            // Cull face
            if (cullFaceEnabled) {
                glEnable(GL_CULL_FACE);
                glCullFace(cullFaceMode);
            } else {
                glDisable(GL_CULL_FACE);
            }

            // Depth test
            if (depthTestEnabled) {
                glEnable(GL_DEPTH_TEST);
                glDepthFunc(depthFunc);
            } else {
                glDisable(GL_DEPTH_TEST);
            }

            // Depth Mask
            if (depthMaskEnabled) {
                glDepthMask(GL_TRUE);
            } else {
                glDepthMask(GL_FALSE);
            }

            // Blending
            if (blendingEnabled) {
                glEnable(GL_BLEND);
                glBlendFunc(blendSrc, blendDst);
            } else {
                glDisable(GL_BLEND);
            }
        }
    };

protected:
    GLuint m_programId{};
    GLStateDescriptor m_stateDescriptor;
    virtual ~BaseShaderProgram() = default;
public:
    BaseShaderProgram() {
        m_programId = glCreateProgram();
    }

    BaseShaderProgram(std::string_view vertexShaderPath, std::string_view fragmentShaderPath) {
        m_programId = glCreateProgram();
        Shader vertexShader{vertexShaderPath, GL_VERTEX_SHADER};
        bool successVert = vertexShader.compile();

        Shader fragmentShader{fragmentShaderPath, GL_FRAGMENT_SHADER};
        bool successFrag = fragmentShader.compile();

        if (!successVert || !successFrag) {
            throw std::runtime_error("Failed to compile shaders");
        }

        attachShader(vertexShader);
        attachShader(fragmentShader);

        if (!linkProgram()) {
            throw std::runtime_error("Failed to link shader program");
        }
    }

    void attachShader(const Shader &shader) {
        glAttachShader(m_programId, shader.getShaderId());
    }

    GLuint getProgramId() const {
        return m_programId;
    }

    void use() {
        glUseProgram(getProgramId());
        m_stateDescriptor.apply();
    }

    bool linkProgram();

    void deleteProgram() {
        glDeleteProgram(m_programId);
    }

    void setBool(const char *name, bool value) const {
        glUniform1i(glGetUniformLocation(m_programId, name), static_cast<int>(value));
    }

    void setInt(const char *name, int value) const {
        glUniform1i(glGetUniformLocation(m_programId, name), value);
    }

    void setFloat(const char *name, float value) const {
        glUniform1f(glGetUniformLocation(m_programId, name), value);
    }

    void setVec2f(const char *name, glm::vec2 value) const {
        glUniform2fv(glGetUniformLocation(m_programId, name), 1, glm::value_ptr(value));
    }

    void setVec3f(const char *name, glm::vec3 value) const {
        glUniform3fv(glGetUniformLocation(m_programId, name), 1, glm::value_ptr(value));
    }

    void setMat3f(const char *name, glm::mat3 value) const {
        glUniformMatrix3fv(glGetUniformLocation(m_programId, name), 1, GL_FALSE, glm::value_ptr(value));
    }

    void setMat4f(const char *name, glm::mat4 value) const {
        glUniformMatrix4fv(glGetUniformLocation(m_programId, name), 1, GL_FALSE, glm::value_ptr(value));
    }

    // Supposed to be called before issuing render calls, used to bind textures etc
    virtual void preRender(const RenderEntity &renderEntity, const RenderCall &renderCall, bool setModelMatrix = true) = 0;
};



#endif //BASESHADERPROGRAM_H
