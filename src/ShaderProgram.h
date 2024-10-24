//
// Created by slice on 10/24/24.
//

#ifndef SHADERPROGRAM_H
#define SHADERPROGRAM_H

#include "Shader.h"
#include "glad/glad.h"

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
};



#endif //SHADERPROGRAM_H
