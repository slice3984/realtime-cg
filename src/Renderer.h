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

    void renderQueue(RenderQueue* queue) {
        for (const std::string &key : queue->m_keys) {
            BaseShaderProgram *currentShader{};

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

                renderData.shader->preRender(renderEntity, renderCall, setModelMatrix);
                setModelMatrix = false;

                glDrawElements(GL_TRIANGLES, renderCall.elemCount, renderCall.componentType, nullptr);
                glBindVertexArray(0);
            }
        }
    }

    void renderAllQueues() {
        for (auto &queue : m_renderQueues) {
            renderQueue(queue);
        }
    }

    void renderQueue(std::string_view name) {
        for (auto &queue : m_renderQueues) {
            if (queue->getName() == name) {
                renderQueue(queue);
                break;
            }
        }
    }

private:
    std::vector<RenderQueue*> m_renderQueues;
};

#endif //RENDERER_H
