//
// Created by slice on 12/21/24.
//

#ifndef TERRAINSAHDERPROGRAM_H
#define TERRAINSAHDERPROGRAM_H
#include "../BaseShaderProgram.h"


class TerrainShaderProgram : public BaseShaderProgram {
public:
    TerrainShaderProgram()
    : BaseShaderProgram("../src/Shaders/TerrainShader/shader.vert", "../src/Shaders/TerrainShader/shader.frag") {
        m_stateDescriptor.cullFaceEnabled = true;
        m_stateDescriptor.cullFaceMode = GL_BACK;
        m_stateDescriptor.depthTestEnabled = true;
        //m_stateDescriptor.blendingEnabled = true;
        m_stateDescriptor.depthMaskEnabled = true;
    }

    void preRender(const RenderEntity &renderEntity, const RenderCall &renderCall, bool setModelMatrix) override {
        // Model matrix
        if (setModelMatrix) {
            setMat4f("u_model", renderEntity.getModelMatrix());
        }

        this->setInt("u_texDiffuse", 0);
        auto it = renderCall.textureHandles.find(TextureType::DIFFUSE);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, it->second);
    }

    void preRender(const RenderCall &renderCall) {}
};



#endif //TERRAINSAHDERPROGRAM_H
