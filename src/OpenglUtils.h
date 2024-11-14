//
// Created by slice on 11/13/24.
//

#ifndef OPENGLUTILS_H
#define OPENGLUTILS_H
#include <variant>
#include <vector>
#include "glad/glad.h"


struct VaoHandle {
    GLuint id;
    bool usesEBO;
    GLuint elemenCount;
};

using GlArrayTypePtr = std::variant<const GLint*, const GLfloat*>;
using GlArrayType = std::variant<GLint, GLfloat>;

struct VertexAttribArray {
    GlArrayTypePtr buffer;
    std::size_t elemCount;
    std::size_t size;

    template <typename T>
    VertexAttribArray(std::initializer_list<T>&& buffer, std::size_t elemCount, std::size_t size) {
        isOwning = true;
        T* localBuffer = new T[buffer.size()];
        std::copy(buffer.begin(), buffer.end(), localBuffer);
        this->buffer = localBuffer;
        this->elemCount = elemCount;
        this->size = size;
    }

    template <typename T>
    VertexAttribArray(const std::vector<T> &buffer, std::size_t elemCount, std::size_t size) {
        this->buffer = buffer.data();
        this->elemCount = elemCount;
        this->size = size;
    }

    ~VertexAttribArray() {
        if (isOwning) {
            // TODO: Figure out a way how to actually free the memory
        }
    }

private:
    bool isOwning = false;
};

struct IndexBufferArray {
    const GLuint *buffer;
    GLuint elemCount;

    IndexBufferArray(std::initializer_list<GLuint> buffer, GLuint elemCount) {
        this->buffer = buffer.begin();
        this->elemCount = elemCount;
    }

    IndexBufferArray(GLuint *buffer, GLuint elemCount) {
        this->buffer = buffer;
        this->elemCount = elemCount;
    }
};

namespace opengl_utils {
    VaoHandle generateVao(const std::vector<VertexAttribArray> &attributes,const IndexBufferArray &indexBuffer = { nullptr, 0 });
}

#endif //OPENGLUTILS_H
