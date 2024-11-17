//
// Created by slice on 11/13/24.
//

#ifndef OPENGLUTILS_H
#define OPENGLUTILS_H
#include <iosfwd>
#include <sstream>
#include <vector>
#include <cassert>
#include <cstring>
#include "glad/glad.h"

struct VaoHandle {
    GLuint id;
    bool usesEBO;
    GLuint elemenCount;
};

struct IndexBufferArray {
    GLuint *buffer;
    GLuint elemCount;

    IndexBufferArray(const std::initializer_list<GLuint> &buffer) {
        this->elemCount = buffer.size();
        this->buffer = const_cast<GLuint*>(buffer.begin());
    }

    explicit IndexBufferArray(const std::vector<GLuint> &buffer) {
        this->buffer = (GLuint*)buffer.data();
        this->elemCount = buffer.size();
    }
};

template <class T>
struct VertexAttribArray {
    T* buffer;
    std::size_t size;
    std::size_t elemCount;
    std::size_t dataTypeSize;

    VertexAttribArray(const std::initializer_list<T> &buffer, std::size_t size) {
        elemCount = buffer.size() / size;
        assert(buffer.size() % size == 0);

        this->buffer = const_cast<T*>(buffer.begin());

        this->size = size;
        this->dataTypeSize = sizeof(this->buffer[0]);
    }

    VertexAttribArray(const std::vector<T> &buffer, std::size_t size) {
        elemCount = buffer.size() / size;
        assert(buffer.size() % size == 0);

        this->buffer = (T*)buffer.data();
        this->size = size;
        this->dataTypeSize = sizeof(buffer.at(0));
    }
};

namespace opengl_utils {
    template<typename  ...Args>
    VaoHandle generateVao(Args ...args) {
        enum class GlDataType {
            FLOAT, INT
        };

        struct ProcessedVertexArray {
            const void *buffer;
            GlDataType type;
            std::size_t stride;
            std::size_t size;
            std::size_t elemCount;
            std::size_t bufferOffset;
        };

        std::vector<ProcessedVertexArray> processedArrays;
        IndexBufferArray indexBuffer = {};
        std::size_t maxOffset{};
        std::size_t mainBufferSize{};

        auto processArg = [&](auto &&arg) -> void {
            using T = std::decay_t<decltype(arg)>;

            // Exclude IndexBufferArray, fails @ compile time check
            if constexpr (!std::is_same_v<T, IndexBufferArray>) {
                ProcessedVertexArray arr;

                if constexpr (std::is_same_v<T, VertexAttribArray<GLfloat>>) {
                    arr.type = GlDataType::FLOAT;
                    arr.stride = arg.size * sizeof(GLfloat);
                } else if constexpr (std::is_same_v<T, VertexAttribArray<GLint>>) {
                    arr.type = GlDataType::INT;
                    arr.stride = arg.size * sizeof(GLint);
                }

                arr.size = arg.size;
                arr.buffer = arg.buffer;
                arr.elemCount = arg.elemCount;
                arr.bufferOffset = maxOffset;
                maxOffset += arr.stride;
                mainBufferSize += arg.elemCount * arg.size * arg.dataTypeSize;

                processedArrays.push_back(std::move(arr));
            } else {
                indexBuffer = arg;
            }
        };

        (processArg(args), ...);

        // Generate VAO and bind
        GLuint VAO;
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);

        // Generate interleaved byte buffer of vertex attributes
        std::byte mainBuffer[mainBufferSize]{};

        for (std::size_t i = 0; i < processedArrays.size(); i++) {
            const ProcessedVertexArray &attribArray = processedArrays[i];

            for (std::size_t j = 0; j < attribArray.elemCount; j++) {
                std::size_t mainBufferPos = j * maxOffset + attribArray.bufferOffset;
                std::size_t vertBufferPos = j * attribArray.stride;

                std::memcpy(mainBuffer + mainBufferPos,
                            (attribArray.buffer + vertBufferPos),
                            attribArray.stride
                );
            }
        }

        // Generate VBO and bind
        GLuint VBO;
        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);

        // Upload buffer to GPU
        glBufferData(GL_ARRAY_BUFFER, mainBufferSize, mainBuffer, GL_STATIC_DRAW);

        // Setup attribute pointers
        for (std::size_t i = 0; i < processedArrays.size(); i++) {
            const ProcessedVertexArray &attribArray = processedArrays[i];

            switch (attribArray.type) {
                case GlDataType::FLOAT:
                    glVertexAttribPointer(i, attribArray.size, GL_FLOAT, GL_FALSE, maxOffset, (void *)(attribArray.bufferOffset));
                    break;
                case GlDataType::INT:
                    glVertexAttribPointer(i, attribArray.size, GL_INT, GL_FALSE, maxOffset, (void *)(attribArray.bufferOffset));
                    break;
            }

            glEnableVertexAttribArray(i);
        }

        VaoHandle handle = {
            VAO,
            false,
            (GLuint)processedArrays[0].elemCount
        };

        // Upload EBO if passed
        if (indexBuffer.elemCount > 0) {
            GLuint EBO;
            glGenBuffers(1, &EBO);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indexBuffer.elemCount, indexBuffer.buffer, GL_STATIC_DRAW);

            handle.usesEBO = true;
            handle.elemenCount = indexBuffer.elemCount;
        }

        return handle;
    }
}

#endif //OPENGLUTILS_H
