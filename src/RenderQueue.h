#ifndef RENDERQUEUE_H
#define RENDERQUEUE_H
#include <string>
#include <string_view>
#include <utility>
#include <vector>
#include <unordered_map>

#include "BaseShaderProgram.h"
#include "glad/glad.h"
#include "RenderCall.h"

struct RenderData {
    RenderEntity renderEntity;
    BaseShaderProgram *shader;
};

class RenderQueue {
public:
    RenderQueue(std::string_view name) : m_name(name) {}

    RenderQueue& setShader(BaseShaderProgram* shader) {
        m_currentShader = shader;
        return *this;
    }

    RenderQueue& addEntity(const std::string& name, RenderEntity renderEntity) {
        insertRenderEntity(name, std::move(renderEntity));
        return *this;
    }

    RenderEntity& operator[](const std::string& name) {
        return m_renderData.at(name).renderEntity;
    }

    const RenderEntity& operator[](const std::string& name) const {
        return m_renderData.at(name).renderEntity;
    }

private:
    friend class Renderer;

    // To preserve insertion order
    std::vector<std::string> m_keys;
    std::unordered_map<std::string, RenderData> m_renderData;
    std::string m_name;
    BaseShaderProgram* m_currentShader;

    void insertRenderEntity(const std::string& name, RenderEntity renderEntity) {
        m_keys.push_back(name);
        m_renderData.emplace(name, RenderData{std::move(renderEntity), m_currentShader});
    }
};

#endif //RENDERQUEUE_H
