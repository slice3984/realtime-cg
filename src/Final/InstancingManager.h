//
// Created by slice on 1/3/25.
//

#ifndef INSTANCINGMANAGER_H
#define INSTANCINGMANAGER_H
#include <glm/vec3.hpp>

#include "../ComputeShader.h"
#include "../RenderCall.h"
#include "../../external/glfw/src/internal.h"
#include "../Shaders/BaseShaderProgram.h"


// Main goal is to create and allocate the required SSBOs and handle instancing draw calls
// Flow
// 1. Add models to be instanced
// 2. Initialize the SSBO
//   -> Generate instance count atomics and set to 0
// 3. On terrain compute recalculations
//   -> Set Shader offset uniforms
//   -> Retrieve instancing amounts
//   -> Reset atomics to 0
// 4. Issue draw calls in render loop
class InstancingManager {
public:
    InstancingManager(const ComputeShader &computeShader, const int totalVertexCount) : m_computeShader(computeShader),
        m_totalVertexCount(totalVertexCount) {
    }

    void addModelToBeInstanced(std::vector<RenderCall> model, BaseShaderProgram *shader) {
        const uint elementOffset = m_totalVertexCount * m_instancedDrawCalls.size();

        m_instancedDrawCalls.emplace_back(InstancedDrawCall{
            shader,
            0,
            elementOffset,
            std::move(model)
        });
    }

    void prepareAtomicCounterFetching() {
        m_fenceHandle = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
        m_isComputeShaderDone = false;
    }

    // The SSBO will store instance data for each type of model in its own region inside the buffer
    // makes it easier to populate the buffer in the compute shader
    // Example structure: [InstanceData grass, InstanceData grass2, [OffsetSpace], InstanceData tree..]
    void initializeSSBOBuffers() {
        // This wastes memory on the GPU since we will never need that amount of memory but makes working with it easier
        GLuint bufferSize = m_totalVertexCount * m_instancedDrawCalls.size() * sizeof(InstancingData);

        glGenBuffers(1, &m_SSBOHandle);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_SSBOHandle);
        glBufferData(GL_SHADER_STORAGE_BUFFER, bufferSize, nullptr, GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_SSBOHandle);
    }

    void setupInstanceCountAtomics() {
        glGenBuffers(1, &m_atomicCounterBuffer);
        glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, m_atomicCounterBuffer);

        m_instanceCounts.resize(m_instancedDrawCalls.size(), 0);
        glBufferData(GL_ATOMIC_COUNTER_BUFFER, m_instanceCounts.size() * sizeof(GLuint), m_instanceCounts.data(),
                     GL_DYNAMIC_DRAW);

        glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 2, m_atomicCounterBuffer);
    }

    // Used to set the starting memory regions in the instancing SSBO for each model type
    void setComputeShaderOffsetUniforms() {
        std::vector<int> offsets(m_instancedDrawCalls.size());

        for (size_t i = 0; i < m_instancedDrawCalls.size(); i++) {
            offsets[i] = m_instancedDrawCalls[i].instanceDataElementOffset;
        }

        m_computeShader.setIntArray("u_modelInstanceOffsets", offsets.data(), offsets.size());
    }

    void issueDrawCalls() {
        if (!m_isComputeShaderDone) {
            if (!pollFenceState()) {
                return;
            } else {
                retrieveInstancingAmountsFromGPU();
            }
        }

        BaseShaderProgram *currentShader = nullptr;
        for (const InstancedDrawCall &model: m_instancedDrawCalls) {
            // Avoid unnecessary rebindings
            if (!currentShader || currentShader->getProgramId() != model.shader->getProgramId()) {
                currentShader = model.shader;
                currentShader->use();
            }

            currentShader->setFloat("u_time", glfwGetTime());
            currentShader->setInt("u_baseInstance", model.instanceDataElementOffset);  // Add this line


            for (const RenderCall &renderCall: model.modelRenderCalls) {
                glBindVertexArray(renderCall.vao);
                currentShader->preRender(renderCall);

                // Resulted in a lot of headaches, baseInstance does not set the starting point of
                // gl_InstanceID, it only sets gl_BaseInstance which is OpenGL 4.6+, passed as uniform instead
                glDrawElementsInstancedBaseInstance(GL_TRIANGLES,
                                                    renderCall.elemCount,
                                                    renderCall.componentType,
                                                    (void *) nullptr,
                                                    model.instanceCount,
                                                    model.instanceDataElementOffset
                );

                glBindVertexArray(0);
            }
        }
    }

private:
    struct InstancedDrawCall {
        BaseShaderProgram *shader;
        GLuint instanceCount;
        uint instanceDataElementOffset;
        std::vector<RenderCall> modelRenderCalls;
    };

    // Respects GPU memory alignment
    struct InstancingData {
        glm::vec3 pos;
        float scaling;
    };

    const ComputeShader &m_computeShader;
    bool m_isComputeShaderDone;
    GLsync m_fenceHandle;
    GLuint m_atomicCounterBuffer;
    std::vector<GLuint> m_instanceCounts;
    std::vector<InstancedDrawCall> m_instancedDrawCalls;
    uint m_totalVertexCount;
    GLuint m_SSBOHandle;

    bool pollFenceState() {
        GLenum waitRet = glClientWaitSync(m_fenceHandle, GL_SYNC_FLUSH_COMMANDS_BIT, 0);

        if (waitRet == GL_ALREADY_SIGNALED || waitRet == GL_CONDITION_SATISFIED) {
            // Compute shader is done
            m_isComputeShaderDone = true;
            glDeleteSync(m_fenceHandle);
            m_fenceHandle = nullptr;
            return true;
        }

        return false;
    }

    void resetInstanceCountAtomics() {
        glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, m_atomicCounterBuffer);
        std::fill(m_instanceCounts.begin(), m_instanceCounts.end(), 0);
        glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, m_instanceCounts.size() * sizeof(GLuint), m_instanceCounts.data());
    }

    void retrieveInstancingAmountsFromGPU() {
        glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, m_atomicCounterBuffer);
        glGetBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, m_instancedDrawCalls.size() * sizeof(GLuint),
                           m_instanceCounts.data());

        // Update the instance counts for each draw call
        for (size_t i = 0; i < m_instancedDrawCalls.size(); i++) {
            m_instancedDrawCalls[i].instanceCount = m_instanceCounts[i];
        }

        resetInstanceCountAtomics();
    }
};


#endif //INSTANCINGMANAGER_H
