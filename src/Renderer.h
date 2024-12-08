//
// Created by slice on 12/7/24.
//

#ifndef RENDERER_H
#define RENDERER_H
#include "RenderQueue.h"
#include "glad/glad.h"
#include <vector>

class Renderer {
public:
    Renderer() = default;

    void addRenderQueue(RenderQueue *queue) {
        m_renderQueues.emplace_back(queue);
    }

    void renderAllQueues() {
        for (auto &queue : m_renderQueues) {
            for (const std::string &key : queue->m_keys) {
                // Only change the shader when required
                BaseShaderProgram *currentShader{};

                // Preserve order of insertion
                const RenderData &renderData = queue->m_renderData.at(key);

                if (currentShader) {
                    if (currentShader->getProgramId() != renderData.shader->getProgramId()) {
                        currentShader = renderData.shader;
                        currentShader->use();
                    }
                } else {
                    currentShader = renderData.shader;
                    currentShader->use();
                }

                const RenderEntity &renderEntity = renderData.renderEntity;

                bool setModelMatrix = true;
                for (auto &renderCall : renderEntity.getRenderCalls()) {
                    glBindVertexArray(renderCall.vao);

                    if (setModelMatrix) {
                        setModelMatrix = false;
                        renderData.shader->preRender(renderEntity, renderCall, true);
                    } else {
                        renderData.shader->preRender(renderEntity, renderCall, false);
                    }

                    glDrawElements(GL_TRIANGLES, renderCall.elemCount, renderCall.componentType, nullptr);
                    glBindVertexArray(0);
                }
            }
        }
    }

private:
    std::vector<RenderQueue*> m_renderQueues;
};

#endif //RENDERER_H
