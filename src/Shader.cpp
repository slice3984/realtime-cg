//
// Created by slice on 10/24/24.
//

#include "Shader.h"

Shader::Shader(std::string_view path, GLenum shaderType) : m_shaderType(shaderType) {
    std::ifstream shaderFile{path.begin()};

    if (!shaderFile) {
        std::cerr << "File not found" << std::endl;
    }

    std::stringstream buffer;
    buffer << shaderFile.rdbuf();
    m_shaderCode = buffer.str();
}

bool Shader::compile() {
    m_shaderId = glCreateShader(m_shaderType);
    const char *source = m_shaderCode.c_str();
    glShaderSource(m_shaderId, 1, &source, nullptr);
    glCompileShader(m_shaderId);

    int success;
    char infoLog[512];
    glGetShaderiv(m_shaderId, GL_COMPILE_STATUS, &success);

    if (!success) {
        glGetShaderInfoLog(m_shaderId, 512, nullptr, infoLog);
        std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;

        return false;
    }

    return true;
}
