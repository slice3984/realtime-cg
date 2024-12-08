//
// Created by slice on 12/7/24.
//

#ifndef RENDERCALL_H
#define RENDERCALL_H

#include <map>
#include <unordered_map>

#include "TextureType.h"
#include "glad/glad.h"

struct RenderCall {
    GLuint vao;
    GLuint elemCount;
    int componentType;

    std::map<TextureType, GLuint> textureHandles;
};

#endif //RENDERCALL_H
