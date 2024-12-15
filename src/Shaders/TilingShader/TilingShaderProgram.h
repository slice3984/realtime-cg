//
// Created by slice on 12/15/24.
//

#ifndef TILINGSHADERPROGRAM_H
#define TILINGSHADERPROGRAM_H
#include "../BaseShaderProgram.h"


class TilingShaderProgram : public BaseShaderProgram {
public:
    TilingShaderProgram() : BaseShaderProgram("../src/Shaders/TilingShader/shader.vert", "../src/Shaders/TilingShader/shader.frag") {
        m_stateDescriptor.cullFaceEnabled = true;
        m_stateDescriptor.cullFaceMode = GL_BACK;
        m_stateDescriptor.depthTestEnabled = true;
    }

    void preRender(const RenderEntity &renderEntity, const RenderCall &renderCall, bool setModelMatrix) override {
        // Model matrix
        if (setModelMatrix) {
            setMat4f("u_model", renderEntity.getModelMatrix());
        }

        // Bind textures
        // Diffuse
        this->setInt("u_texDiffuse", 0);
        auto it = renderCall.textureHandles.find(TextureType::DIFFUSE);
        if (it != renderCall.textureHandles.end()) {
            // If a diffuse texture is found, bind it
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, it->second);
        } else {
            // Otherwise use a default one
            if (defaultTextureHandleDiffuse < 0) {
                defaultTextureHandleDiffuse = generateDefaultTexture();
            }

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, defaultTextureHandleDiffuse);
        }
    }

private:
    inline static GLint defaultTextureHandleDiffuse = -1;

    [[nodiscard]] static GLuint generateDefaultTexture() {
        GLuint defaultTexture;
        glGenTextures(1, &defaultTexture);
        glBindTexture(GL_TEXTURE_2D, defaultTexture);

        GLubyte whitePixel[4] = {255, 255, 255, 255};
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, whitePixel);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        return defaultTexture;
    }
};



#endif //TILINGSHADERPROGRAM_H
