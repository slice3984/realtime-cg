//
// Created by slice on 10/24/24.
//

#ifndef SHADERPROGRAM_H
#define SHADERPROGRAM_H

#include "Shader.h"
#include "glad/glad.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

class ShaderProgram {
private:
    GLuint m_programId{};

public:
    ShaderProgram() {
        m_programId = glCreateProgram();
    }

    void attachShader(const Shader &shader) {
        glAttachShader(m_programId, shader.getShaderId());
    }

    GLuint getProgramId() const {
        return m_programId;
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

    void setVec3f(const char *name, glm::vec3 value) const {
        glUniform3fv(glGetUniformLocation(m_programId, name), 1, glm::value_ptr(value));
    }
};



#endif //SHADERPROGRAM_H
