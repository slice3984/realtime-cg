//
// Created by slice on 12/6/24.
//

#ifndef TEXTUREHANDLE_H
#define TEXTUREHANDLE_H
#include "glad/glad.h"

struct TextureHandle {
    GLuint handle;
    int width;
    int height;
    int format; // For example: RGB, RGBA..
};

#endif //TEXTUREHANDLE_H
