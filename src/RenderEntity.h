#ifndef RENDERENTITY_H
#define RENDERENTITY_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include "RenderCall.h"

class RenderEntity {
public:
    RenderEntity(const std::vector<RenderCall> &renderCalls,
                 glm::vec3 translation = glm::vec3(0.0f),
                 glm::vec3 scale = glm::vec3(1.0f),
                 glm::vec3 rotationAxis = glm::vec3(0.0f, 1.0f, 0.0f),
                 float rotationAngle = 0.0f)
        : m_renderCalls(renderCalls),
          m_translation(translation),
          m_scale(scale),
          m_rotationAxis(rotationAxis),
          m_rotationAngle(rotationAngle) {}

    RenderEntity(const RenderCall &renderCall,
                 glm::vec3 translation = glm::vec3(0.0f),
                 glm::vec3 scale = glm::vec3(1.0f),
                 glm::vec3 rotationAxis = glm::vec3(0.0f, 1.0f, 0.0f),
                 float rotationAngle = 0.0f)
        : RenderEntity(std::vector<RenderCall>{renderCall}, translation, scale, rotationAxis, rotationAngle) {}

    [[nodiscard]] glm::mat4 getModelMatrix() const {
        glm::mat4 model = glm::mat4(1.0f);

        model = glm::translate(model, m_translation);
        model = glm::rotate(model, glm::radians(m_rotationAngle), m_rotationAxis);
        model = glm::scale(model, m_scale);

        return model;
    }

    [[nodiscard]] const std::vector<RenderCall> &getRenderCalls() const {
        return m_renderCalls;
    }

    [[nodiscard]] glm::vec3 &getTranslation() {
        return m_translation;
    }

    void setTranslation(const glm::vec3 &translation) {
        m_translation = translation;
    }

    [[nodiscard]] glm::vec3 &getScale() {
        return m_scale;
    }

    void setScale(const glm::vec3 &scale) {
        m_scale= scale;
    }

    [[nodiscard]] glm::vec3 &getRotationAxis() {
        return m_rotationAxis;
    }

    void setRotationAxis(const glm::vec3 &rotationAxis) {
        m_rotationAxis = rotationAxis;
    }

    [[nodiscard]] float &getRotationAngle() {
        return m_rotationAngle;
    }

    void setRotationAngle(float rotationAngle) {
        m_rotationAngle = rotationAngle;
    }

private:
    friend class Renderer;

    std::vector<RenderCall> m_renderCalls;
    glm::vec3 m_translation;
    glm::vec3 m_scale;
    glm::vec3 m_rotationAxis;
    float m_rotationAngle;
};

#endif //RENDERENTITY_H
