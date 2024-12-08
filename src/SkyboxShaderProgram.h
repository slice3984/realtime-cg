//
// Created by slice on 12/7/24.
//

#ifndef SKYBOXSHADERPROGRAM_H
#define SKYBOXSHADERPROGRAM_H
#include "BaseShaderProgram.h"


class SkyboxShaderProgram : public BaseShaderProgram {
public:
    SkyboxShaderProgram(std::string_view vertexShaderPath, std::string_view fragmentShaderPath)
      : BaseShaderProgram(vertexShaderPath, fragmentShaderPath) {
        m_stateDescriptor.cullFaceEnabled = true;
        m_stateDescriptor.cullFaceMode = GL_FRONT;
        m_stateDescriptor.depthTestEnabled = true;
        m_stateDescriptor.depthFunc = GL_LEQUAL;
    }

    void preRender(const RenderEntity &renderEntity, const RenderCall &renderCall, bool setModelMatrix) override {
        this->setInt("m_skybox", 1);

        auto it = renderCall.textureHandles.find(TextureType::CUBE_MAP);
        if (it != renderCall.textureHandles.end()) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, it->second);
        }
    }
};



#endif //SKYBOXSHADERPROGRAM_H
