//
// Created by slice on 12/7/24.
//

#ifndef MODELSHADERPROGRAM_H
#define MODELSHADERPROGRAM_H
#include "BaseShaderProgram.h"

class ModelShaderProgram : public BaseShaderProgram {
public:
    ModelShaderProgram(std::string_view vertexShaderPath, std::string_view fragmentShaderPath)
         : BaseShaderProgram(vertexShaderPath, fragmentShaderPath) {
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
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, it->second);
        }
    }
};



#endif //MODELSHADERPROGRAM_H
