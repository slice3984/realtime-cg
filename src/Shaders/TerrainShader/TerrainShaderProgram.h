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
    }

    void preRender(const RenderEntity &renderEntity, const RenderCall &renderCall, bool setModelMatrix) override {
        // Model matrix
        if (setModelMatrix) {
            setMat4f("u_model", renderEntity.getModelMatrix());
        }
    }
};



#endif //TERRAINSAHDERPROGRAM_H
