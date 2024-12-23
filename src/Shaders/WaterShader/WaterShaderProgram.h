//
// Created by slice on 12/22/24.
//

#ifndef WATERSHADERPROGRAM_H
#define WATERSHADERPROGRAM_H
#include "../BaseShaderProgram.h"


class WaterShaderProgram : public BaseShaderProgram {
public:
    WaterShaderProgram()
        : BaseShaderProgram("../src/Shaders/WaterShader/shader.vert", "../src/Shaders/WaterShader/shader.frag") {
        m_stateDescriptor.cullFaceEnabled = true;
        m_stateDescriptor.cullFaceMode = GL_BACK;
        m_stateDescriptor.depthTestEnabled = true;
    };

    void preRender(const RenderEntity &renderEntity, const RenderCall &renderCall, bool setModelMatrix) override {
        // Model matrix
        if (setModelMatrix) {
            setMat4f("u_model", renderEntity.getModelMatrix());
        }

        this->setInt("m_skybox", 1);
        auto it = renderCall.textureHandles.find(TextureType::CUBE_MAP);
        if (it != renderCall.textureHandles.end()) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, it->second);
        }
    }
};



#endif //WATERSHADERPROGRAM_H
