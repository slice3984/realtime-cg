//
// Created by slice on 11/13/24.
//

#include "OpenglUtils.h"

namespace opengl_utils {
    std::vector<const GltfPrimitive *> unpackGltfScene(const GltfScene &scene) {
        std::vector<const GltfPrimitive *> primitives;

        for (const GltfObject &object: scene.objects) {
            for (const GltfMesh &mesh: object.meshes) {
                for (const GltfPrimitive &primitive: mesh.primitives) {
                    primitives.push_back(&primitive);
                }
            }
        }

        return primitives;
    }

    ImageData loadImage(const std::string &imagePath) {
        int width, height, nChannels;

        unsigned char *data = stbi_load(imagePath.c_str(), &width, &height, &nChannels, 0);

        if (data) {
            return {
                width,
                height,
                nChannels,
                data
            };
        } else {
            throw std::runtime_error("Unable to load image: " + imagePath);
        }
    }

    TextureHandle createTexture(int width, int height, int nChannels, bool repeatTexture, GLuint target) {
        int format = GL_RGBA;

        switch (nChannels) {
            case 1: format = GL_RED;
            break;
            case 2: format = GL_RG;
            break;
            case 3: format = GL_RGB;
            break;
        }

        GLuint handle;
        glGenTextures(1, &handle);
        glBindTexture(target, handle);

        if (target == GL_TEXTURE_CUBE_MAP) {
            for (GLuint i = 0; i < 6; ++i) {
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA8, width, height, 0, format, GL_UNSIGNED_BYTE, nullptr);
            }
        } else if (target == GL_TEXTURE_2D) {
            glTexImage2D(target, 0, GL_RGBA8, width, height, 0, format, GL_UNSIGNED_BYTE, nullptr);
            glGenerateMipmap(GL_TEXTURE_2D);
        }

        if (repeatTexture) {
            glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_REPEAT);
        } else {
            glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }

        glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        return { handle, width, height, format };
    }

    TextureHandle createTexture(const ImageData &image, bool repeatTexture, GLuint target) {
        return createTexture(image.width, image.height, image.nChannels, repeatTexture, target);
    }

    void updateTextureData(TextureHandle texture, const ImageData &image, GLuint target) {
        glBindTexture(target, texture.handle);

        glTexSubImage2D(target,
                        0,
                        0,
                        0,
                        texture.width,
                        texture.height,
                        texture.format,
                        GL_UNSIGNED_BYTE,
                        image.buffer
        );

        glGenerateMipmap(target);
        glBindTexture(target, 0);
    }

    TextureHandle loadCubemap(const std::vector<std::string> &imagePaths) {
        if (imagePaths.size() != 6) {
            throw std::runtime_error("6 images are required to generate a cube map texture");
        }

        std::vector<ImageData> images;

        for (const std::string &path : imagePaths) {
            images.emplace_back(loadImage(path));
        }

        // We assume all images got the same size
        TextureHandle cubeTex = createTexture(images[0], false, GL_TEXTURE_CUBE_MAP);

        // Set all sides of the cube map texture
        for (std::size_t i = 0; i < images.size(); i++) {
            updateTextureData(cubeTex, images[i], GL_TEXTURE_CUBE_MAP_POSITIVE_X + i);
        }
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        return cubeTex;
    }
}
