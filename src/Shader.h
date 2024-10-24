//
// Created by slice on 10/24/24.
//

#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <fstream>
#include <iostream>
#include <sstream>

#include "glad/glad.h"

class Shader {
private:
    std::string m_shaderCode;
    const GLenum m_shaderType;
    GLuint m_shaderId{};

public:
    Shader(std::string_view path, GLenum shaderType);
    bool compile();
    [[nodiscard]] GLuint getShaderId() const { return m_shaderId; }
};



#endif //SHADER_H
