//
// Created by slice on 11/13/24.
//

#ifndef OPENGLUTILS_H
#define OPENGLUTILS_H
#include <iosfwd>
#include <vector>
#include <cassert>
#include <cstring>

#include "GltfLoader.h"
#include "glad/glad.h"
#include <stb_image.h>

#include "ImageData.h"
#include "TextureHandle.h"

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
        this->buffer = const_cast<GLuint *>(buffer.begin());
    }

    explicit IndexBufferArray(const std::vector<GLuint> &buffer) {
        this->buffer = (GLuint *) buffer.data();
        this->elemCount = buffer.size();
    }
};

template<class T>
struct VertexAttribArray {
    T *buffer;
    std::size_t size;
    std::size_t elemCount;
    std::size_t dataTypeSize;

    VertexAttribArray(const std::initializer_list<T> &buffer, std::size_t size) {
        elemCount = buffer.size() / size;
        assert(buffer.size() % size == 0);

        this->buffer = const_cast<T *>(buffer.begin());

        this->size = size;
        this->dataTypeSize = sizeof(this->buffer[0]);
    }

    VertexAttribArray(const std::vector<T> &buffer, std::size_t size) {
        elemCount = buffer.size() / size;
        assert(buffer.size() % size == 0);

        this->buffer = (T *) buffer.data();
        this->size = size;
        this->dataTypeSize = sizeof(buffer.at(0));
    }
};

struct PrimitiveData {
    std::vector<GLfloat> positions;
    std::vector<GLfloat> texCoords;
    std::vector<GLushort> colors;
    std::vector<GLfloat> normals;
    std::vector<GLfloat> tangents;
    std::vector<GLuint> ebo;
};

enum class PrimitiveType {
    CUBE, SPHERE, PLANE
};

namespace opengl_utils {
    template<typename... Args>
    VaoHandle generateVao(Args... args) {
        enum class GlDataType {
            FLOAT, INT, USHORT
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

                if constexpr (std::is_same_v<T, VertexAttribArray<GLfloat> >) {
                    arr.type = GlDataType::FLOAT;
                    arr.stride = arg.size * sizeof(GLfloat);
                } else if constexpr (std::is_same_v<T, VertexAttribArray<GLint> >) {
                    arr.type = GlDataType::INT;
                    arr.stride = arg.size * sizeof(GLint);
                } else if constexpr (std::is_same_v<T, VertexAttribArray<GLushort> >) {
                    arr.type = GlDataType::USHORT;
                    arr.stride = arg.size * sizeof(GLushort);
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
                    glVertexAttribPointer(i, attribArray.size, GL_FLOAT, GL_FALSE, maxOffset,
                                          (void *) (attribArray.bufferOffset));
                    break;
                case GlDataType::INT:
                    glVertexAttribPointer(i, attribArray.size, GL_INT, GL_FALSE, maxOffset,
                                          (void *) (attribArray.bufferOffset));
                    break;
                // Usually color data that has to be normalized
                case GlDataType::USHORT:
                    glVertexAttribPointer(i, attribArray.size, GL_UNSIGNED_SHORT, GL_TRUE, maxOffset,
                                          (void *) (attribArray.bufferOffset));
            }

            glEnableVertexAttribArray(i);
        }

        VaoHandle handle = {
            VAO,
            false,
            (GLuint) processedArrays[0].elemCount
        };

        // Upload EBO if passed
        if (indexBuffer.elemCount > 0) {
            GLuint EBO;
            glGenBuffers(1, &EBO);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indexBuffer.elemCount, indexBuffer.buffer,
                         GL_STATIC_DRAW);

            handle.usesEBO = true;
            handle.elemenCount = indexBuffer.elemCount;
        }

        return handle;
    }

    std::vector<const GltfPrimitive *> unpackGltfScene(const GltfScene &scene);

inline PrimitiveData getPrimitive(PrimitiveType type) {
    // Cache already loaded primitives
    static std::unordered_map<PrimitiveType, PrimitiveData> primitiveCache;

    if (primitiveCache.find(type) == primitiveCache.end()) {
        std::string path = "../assets/models/primitives/";

        switch (type) {
            case PrimitiveType::CUBE:
                path += "cube.glb";
                break;
            case PrimitiveType::SPHERE:
                path += "sphere.glb";
                break;
            case PrimitiveType::PLANE:
                path += "plane.glb";
                break;
        }

        PrimitiveData data;

        // We assume there's only one primitive in every scene used here
        GltfLoader loader;
        GltfScene scene = loader.loadModel(path);
        auto unpacked = unpackGltfScene(scene);
        const GltfPrimitive *firstPrimitive = unpacked[0];
        const GltfVertexAttrib &pos = firstPrimitive->get(GltfAttribute::POSITION);
        const GltfVertexAttrib &texCoord = firstPrimitive->get(GltfAttribute::TEXCOORD_0);
        const GltfVertexAttrib &normal = firstPrimitive->get(GltfAttribute::NORMAL);
        const GltfVertexAttrib &tangent = firstPrimitive->get(GltfAttribute::TANGENT);

        // Check if color attribute exists, otherwise use a default white color
        const GltfVertexAttrib *color = nullptr;
        try {
            color = &firstPrimitive->get(GltfAttribute::COLOR_0);
        } catch (const std::exception& e) {
            // Color attribute is missing, use default white color
        }

        auto constructVector = [](const GltfVertexAttrib &attrib) -> std::vector<GLfloat> {
            // We assume its always GLfloat
            GLfloat *dataPtr = static_cast<GLfloat *>(attrib.buffer);
            std::size_t vecSize = attrib.bufferSize / 4; // 4 byte for float
            std::vector<GLfloat> vec;
            vec.resize(vecSize);

            std::memcpy(vec.data(), dataPtr, attrib.bufferSize);

            return vec;
        };

        data.positions = constructVector(pos);
        data.texCoords = constructVector(texCoord);
        data.normals = constructVector(normal);
        data.tangents = constructVector(tangent);

        if (color) {
            GLfloat *dataPtr = static_cast<GLfloat *>(color->buffer);
            std::size_t vecSize = color->bufferSize / 4; // 4 byte for float
            std::vector<GLfloat> tempVec;
            tempVec.resize(vecSize);

            std::memcpy(tempVec.data(), dataPtr, color->bufferSize);

            std::vector<GLushort> convertedColors;
            convertedColors.reserve(tempVec.size());
            for (size_t i = 0; i < tempVec.size(); i++) {
                convertedColors.push_back(static_cast<GLushort>(tempVec[i] * 65535));
            }

            data.colors = std::move(convertedColors);
        } else {
            size_t numVertices = data.positions.size() / 3;
            std::vector<GLushort> defaultColor(numVertices * 4, 0);
            for (size_t i = 3; i < defaultColor.size(); i += 4) {
                defaultColor[i] = 65535;
            }
            data.colors = std::move(defaultColor);
        }

        std::vector<GLuint> indexVec;
        indexVec.resize(firstPrimitive->elemCount);

        switch (firstPrimitive->componentType) {
            case 5121: {
                // GL_UNSIGNED_BYTE
                GLubyte *byteBuffer = static_cast<GLubyte *>(firstPrimitive->indexBuffer);
                for (size_t i = 0; i < firstPrimitive->elemCount; i++) {
                    indexVec[i] = static_cast<GLuint>(byteBuffer[i]);
                }
                break;
            }
            case 5123: {
                // GL_UNSIGNED_SHORT
                GLushort *shortBuffer = static_cast<GLushort *>(firstPrimitive->indexBuffer);
                for (size_t i = 0; i < firstPrimitive->elemCount; i++) {
                    indexVec[i] = static_cast<GLuint>(shortBuffer[i]);
                }
                break;
            }
            case 5125: {
                // GL_UNSIGNED_INT
                GLuint *intBuffer = static_cast<GLuint *>(firstPrimitive->indexBuffer);
                for (size_t i = 0; i < firstPrimitive->elemCount; i++) {
                    indexVec[i] = intBuffer[i];
                }
                break;
            }
            default: {
                std::cerr << "Unsupported componentType: " << firstPrimitive->componentType << std::endl;
                break;
            }
        }

        data.ebo = std::move(indexVec);
        primitiveCache[type] = std::move(data);
    }

    return primitiveCache[type];
}




    ImageData loadImage(const std::string &imagePath);
    TextureHandle createTexture(int width, int height, int nChannels = 4, bool repeatTexture = false, GLuint target = GL_TEXTURE_2D);
    TextureHandle createTexture(const ImageData &image, bool repeatTexture = false, GLuint target = GL_TEXTURE_2D);
    void updateTextureData(TextureHandle texture, const ImageData &image, GLuint target = GL_TEXTURE_2D);
    TextureHandle loadCubemap(const std::vector<std::string> &imagePaths);
}

#endif //OPENGLUTILS_H
