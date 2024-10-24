//
// Created by slice on 10/24/24.
//

#include "ShaderProgram.h"

bool ShaderProgram::linkProgram() {
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
