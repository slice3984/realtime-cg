//
// Created by slice on 11/30/24.
//

#ifndef GPUMODELUPLOADER_H
#define GPUMODELUPLOADER_H
#include <vector>

#include "GltfLoader.h"
#include "glad/glad.h"

struct RenderCall {
    GLuint vao;
    GLuint elemCount;
    int componentType;
    GLint textureDiffuse = -1;
    GLint textureMetallicRoughness = -1;
    GLint textureNormal = -1;
    GLint textureOcclusion = -1;
    GLint textureEmissive = -1;
};

class GPUModelUploader {
private:
    GLuint processPrimitive(const GltfPrimitive &primitive, const GltfScene &model, RenderCall &renderCall) {
        // Create VAO
        GLuint VAO;
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);

        // We always expect the given attributes to be in the primitive
        const std::vector<GltfAttribute> attribOrder = {
            GltfAttribute::POSITION,
            GltfAttribute::NORMAL,
            GltfAttribute::TANGENT,
            GltfAttribute::TEXCOORD_0
        };

        auto getComponentSize = [](int componentType) -> std::size_t {
            switch (componentType) {
                case GL_FLOAT: return sizeof(float);
                case GL_UNSIGNED_INT: return sizeof(unsigned int);
                case GL_INT: return sizeof(int);
                default: throw std::runtime_error("Unsupported component type");
            }
        };

        // Temp map so also tangents can be generated if missing
        std::unordered_map<GltfAttribute, const GltfVertexAttrib*> attribs;

        for (auto const &[key, val] : primitive.attributes) {
            attribs[key] = &val;
        }

        // Calculate total size of a single vertex and determine vertex count
        std::size_t vertexSize = 0;
        std::size_t vertexCount = 0;

        for (const auto &attrib : attribOrder) {
            if (attribs.find(attrib) != attribs.end()) {
                const auto &attribute = *attribs.at(attrib);
                std::size_t componentSize = getComponentSize(attribute.componentType);
                std::size_t componentsPerElement = attribute.bufferSize / (attribute.elemCount * componentSize);

                vertexSize += componentSize * componentsPerElement;

                if (vertexCount == 0) {
                    vertexCount = attribute.elemCount;
                }
            } else if (attrib == GltfAttribute::TANGENT) {
                std::vector<float> tmpBuffer(vertexCount * 3, 0.0f);

                for (std::size_t i = 0; i < vertexCount; i++) {
                    glm::vec3 defaultTangent(0.0f, 0.0f, 0.0f);

                    std::memcpy(&tmpBuffer[i * 3], &defaultTangent, sizeof(defaultTangent));
                }

                GltfVertexAttrib tmpTangentAttrib = {
                    "Tangent",
                    tmpBuffer.data(),
                    GL_FLOAT,
                    3,
                    vertexCount,
                    tmpBuffer.size()
                };

                attribs[GltfAttribute::TANGENT] = &tmpTangentAttrib;
            } else {
                throw std::runtime_error("Missing vertex attribute in model");
            }
        }

        std::size_t mainBufferSize = vertexSize * vertexCount;
        auto *mainBuffer = new std::byte[mainBufferSize];

        // Fill the buffer
        for (std::size_t i = 0; i < vertexCount; ++i) {
            std::size_t offset = i * vertexSize;

            for (const auto &attrib : attribOrder) {
                const auto &attribute = *attribs.at(attrib);

                std::size_t componentSize = getComponentSize(attribute.componentType);
                std::size_t componentsPerElement = attribute.bufferSize / (attribute.elemCount * componentSize);

                // Copy data for the current attribute
                std::memcpy(
                    mainBuffer + offset,
                    static_cast<std::byte *>(attribute.buffer) + i * componentSize * componentsPerElement,
                    componentSize * componentsPerElement
                );

                // Advance the offset for the next attribute
                offset += componentSize * componentsPerElement;
            }
        }

        // VBO
        GLuint VBO;
        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, mainBufferSize, mainBuffer, GL_STATIC_DRAW);

        // Setup attribute pointers
        std::size_t offset = 0;
        GLuint index = 0;

        for (const auto &attrib : attribOrder) {
            if (primitive.attributes.find(attrib) != primitive.attributes.end()) {
                const auto &attribute = primitive.attributes.at(attrib);
                std::size_t componentSize = getComponentSize(attribute.componentType);
                std::size_t componentsPerElement = attribute.bufferSize / (attribute.elemCount * componentSize);

                glEnableVertexAttribArray(index);
                glVertexAttribPointer(
                    index,
                    static_cast<GLint>(componentsPerElement),
                    attribute.componentType,
                    GL_FALSE,
                    static_cast<GLsizei>(vertexSize),
                    reinterpret_cast<void *>(offset)
                );

                offset += componentSize * componentsPerElement;
                index++;
            }
        }

        // EBO
        GLuint EBO;
        glGenBuffers(1, &EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, primitive.indexBufferSize, primitive.indexBuffer, GL_STATIC_DRAW);

        // Material processing (returning texture handles)
        processMaterial(primitive, model, renderCall);

        // Clean up
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        delete[] mainBuffer;

        return VAO;
    }

    void processMaterial(const GltfPrimitive &primitive, const GltfScene &model, RenderCall &renderCall) {
        if (primitive.materialIdx >= 0) {
            auto uploadTexture = [](const GltfImage &img) -> GLuint {
                GLuint tex;
                glGenTextures(1, &tex);
                glBindTexture(GL_TEXTURE_2D, tex);

                int format = GL_RGBA;
                switch (img.component) {
                    case 1: format = GL_R8; break;
                    case 3: format = GL_RGB; break;
                    case 4: format = GL_RGBA; break;
                    default: throw std::runtime_error("Invalid internal pixel format in texture");
                }

                glTexImage2D(GL_TEXTURE_2D, 0, format, img.width, img.height, 0, format, img.pixelType, img.buffer.data());
                glGenerateMipmap(GL_TEXTURE_2D);

                return tex;
            };

            const GltfMaterial &mat = model.materials.at(primitive.materialIdx);

            // We ignore baseColorFactor for now
            for (const GltfTextureProperties &properties : mat.textureProperties) {
                const GltfImage &image = model.images[properties.index];

                switch (properties.type) {
                    case GltfTextureType::DIFFUSE: renderCall.textureDiffuse = uploadTexture(image); break;
                    case GltfTextureType::METALLIC_ROUGHNESS: renderCall.textureMetallicRoughness = uploadTexture(image); break;
                    case GltfTextureType::NORMAL: renderCall.textureNormal = uploadTexture(image); break;
                    case GltfTextureType::OCCLUSION: renderCall.textureOcclusion = uploadTexture(image); break;
                    case GltfTextureType::EMISSIVE: renderCall.textureEmissive = uploadTexture(image); break;
                }
            }
        }
    }

public:
    GPUModelUploader() = default;

    std::vector<RenderCall> uploadGltfModel(const GltfScene &model) {
        std::vector<RenderCall> renderCalls;

        for (const GltfObject &object : model.objects) {
            for (const GltfMesh &mesh : object.meshes) {
                for (const GltfPrimitive &primitive : mesh.primitives) {
                    GLuint texture = 0; // Store the texture handle here
                    RenderCall renderCall{};
                    renderCall.vao = processPrimitive(primitive, model, renderCall);
                    renderCall.componentType = primitive.componentType;
                    renderCall.elemCount = primitive.elemCount;

                    renderCalls.push_back(renderCall);
                }
            }
        }

        return renderCalls;
    }
};

#endif //GPUMODELUPLOADER_H
