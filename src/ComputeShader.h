//
// Created by slice on 12/31/24.
//

#ifndef COMPUTESHADER_H
#define COMPUTESHADER_H
#include <fstream>
#include <iostream>
#include <sstream>
#include <string_view>

#include "glad/glad.h"


class ComputeShader {
public:
    ComputeShader(std::string_view path) {
        std::ifstream shaderFile{path.begin()};

        if (!shaderFile) {
            std::cerr << "File not found" << std::endl;
        }

        std::stringstream buffer;
        buffer << shaderFile.rdbuf();
        m_shaderCode = buffer.str();

        bool success = compileShader();

        if (!success) {
            std::cerr << "Error in compute shader compilation stage" << std::endl;
        } else {
            createProgram();
        }
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

    void setIntArray(const char *name, const int *values, GLsizei count) const {
        glUniform1iv(glGetUniformLocation(m_programId, name), count, values);
    }

    GLuint getProgramId() const { return m_programId; }
private:
    GLuint m_shaderId{};
    GLuint m_programId{};
    std::string m_shaderCode;

    bool compileShader() {
        m_shaderId = glCreateShader(GL_COMPUTE_SHADER);
        const char *source = m_shaderCode.c_str();
        glShaderSource(m_shaderId, 1, &source, nullptr);
        glCompileShader(m_shaderId);

        int success;
        char infoLog[512];
        glGetShaderiv(m_shaderId, GL_COMPILE_STATUS, &success);

        if (!success) {
            glGetShaderInfoLog(m_shaderId, 512, nullptr, infoLog);
            std::cerr << "ERROR::SHADER::COMPUTE::COMPILATION_FAILED\n" << infoLog << std::endl;

            return false;
        }

        return true;
    }

    void createProgram() {
        m_programId = glCreateProgram();
        glAttachShader(m_programId, m_shaderId);
        glLinkProgram(m_programId);

        int success;
        char infoLog[512];
        glGetProgramiv(m_programId, GL_LINK_STATUS, &success);

        if (!success) {
            glGetProgramInfoLog(m_programId, 512, nullptr, infoLog);
            std::cerr << "ERROR::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
            glDeleteProgram(m_programId);
        }
    }
};



#endif //COMPUTESHADER_H
