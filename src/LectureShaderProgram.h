//
// Created by slice on 12/7/24.
//

#ifndef LECTURESHADERPROGRAM_H
#define LECTURESHADERPROGRAM_H

#include "Shader.h"
#include "glad/glad.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

// Only really used to no break the base lecture shader logic
class LectureShaderProgram {
private:
    GLuint m_programId{};
public:
    LectureShaderProgram() {
        m_programId = glCreateProgram();
    }

    void attachShader(const Shader &shader) const {
        glAttachShader(m_programId, shader.getShaderId());
    }

    [[nodiscard]] GLuint getProgramId() const {
        return m_programId;
    }

    [[nodiscard]] bool linkProgram() const {
        glLinkProgram(m_programId);

        GLint success;
        glGetProgramiv(m_programId, GL_LINK_STATUS, &success);
        if (!success) {
            char infoLog[512];
            glGetProgramInfoLog(m_programId, 512, nullptr, infoLog);
            std::cerr << "ERROR::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
            return false;
        }

        return true;
    }

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
};


#endif //LECTURESHADERPROGRAM_H
