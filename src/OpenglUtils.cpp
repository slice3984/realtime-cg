//
// Created by slice on 11/13/24.
//

#include "OpenglUtils.h"

#include <variant>
#include <cassert>
#include <cstring>
#include <iostream>

namespace opengl_utils {
    VaoHandle generateVao(const std::vector<VertexAttribArray> &attributes, const IndexBufferArray &indexBuffer) {
        std::vector<std::size_t> bufferStrides;
        std::vector<std::size_t> bufferOffsets;
        std::vector<std::size_t> elementsInBuffers;
        std::size_t maxOffset{};
        std::size_t mainBufferSize{};

        auto updateVariables = [&](std::size_t datatypeSize, const VertexAttribArray &attribArray) {
            std::size_t attribSize = datatypeSize * attribArray.size;
            bufferStrides.push_back(attribSize);
            bufferOffsets.push_back(maxOffset);
            maxOffset += attribSize;
            mainBufferSize += datatypeSize * attribArray.elemCount * attribArray.size;
        };

        for (const VertexAttribArray &attribArray : attributes) {
            elementsInBuffers.push_back(attribArray.elemCount);

            // Determinate the offsets in each vertex attribute array in bytes and the total size
            if (std::holds_alternative<const GLint*>(attribArray.buffer)) {
                updateVariables(sizeof(GLint), attribArray);
            } else if (std::holds_alternative<const GLfloat*>(attribArray.buffer)) {
                updateVariables(sizeof(GLfloat), attribArray);
            }
        }

        // Verify each buffer got the same amount of elements
        for (std::size_t i = 0; i < elementsInBuffers.size() - 1; i++) {
            assert(elementsInBuffers[i] == elementsInBuffers[i + 1]);
        }

        // Generate VAO & bind
        GLuint VAO;
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);

        // Generate interleaved byte buffer of vertex attributes
        std::byte mainBuffer[mainBufferSize]{};

        for (std::size_t i = 0; i < attributes.size(); i++) {
            const VertexAttribArray &attribArray = attributes[i];

            const void* attribBuffer = std::visit([](const auto* buffer) -> const void* {
                return static_cast<const void*>(buffer);
            }, attribArray.buffer);

            for (std::size_t j = 0; j < attribArray.elemCount; j++) {
                std::size_t mainBufferPos = j * maxOffset + bufferOffsets[i];
                std::size_t vertBufferPos = j * bufferStrides[i];

                std::memcpy(mainBuffer + mainBufferPos,
                            static_cast<const std::byte*>(attribBuffer) + vertBufferPos,
                            bufferStrides[i]);
            }
        }

        // Generate & bind VBO
        GLuint VBO;
        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);

        // Upload buffer to GPU
        glBufferData(GL_ARRAY_BUFFER, mainBufferSize, mainBuffer, GL_STATIC_DRAW);

        // Setup attribute pointers
        for (std::size_t i = 0; i < attributes.size(); i++) {
            const VertexAttribArray &attribArray = attributes[i];

            if (std::holds_alternative<const GLint*>(attribArray.buffer)) {
                glVertexAttribPointer(i, attribArray.size, GL_INT, GL_FALSE, maxOffset, (void *)(bufferOffsets[i]));
            } else if (std::holds_alternative<const GLfloat*>(attribArray.buffer)) {
                glVertexAttribPointer(i, attribArray.size, GL_FLOAT, GL_FALSE, maxOffset, (void *)(bufferOffsets[i]));
            }

            glEnableVertexAttribArray(i);
        }

        VaoHandle handle = {
            VAO,
            false,
            (GLuint)elementsInBuffers[0]
        };

        // Upload the EBO if passed
        if (indexBuffer.elemCount > 0) {
            GLuint EBO;
            glGenBuffers(1, &EBO);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indexBuffer.elemCount, indexBuffer.buffer, GL_STATIC_DRAW);

            handle.usesEBO = true;
            handle.elemenCount = indexBuffer.elemCount;
        }

        // Unbind VAO
        glBindVertexArray(0);

        return handle;
    }
}
