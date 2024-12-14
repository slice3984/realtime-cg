//
// Created by slice on 11/23/24.
//

#ifndef ORBITCAMERA_H
#define ORBITCAMERA_H

#include <iostream>
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>

class OrbitCamera {
private:
    float m_orbitPan = 0.0f;
    float m_orbitTilt = 0.0f;
    float m_orbitDistance = 3.0f;
    glm::vec3 m_camPos;
    glm::mat4 m_view = glm::identity<glm::mat4>();

private:
    void updateViewMatrix() {
        glm::mat4 rotationX = glm::rotate(glm::mat4(1.0f), glm::radians(m_orbitTilt), glm::vec3(1.0f, 0.0f, 0.0f));
        glm::mat4 rotationY = glm::rotate(glm::mat4(1.0f), glm::radians(m_orbitPan), glm::vec3(0.0f, 1.0f, 0.0f));

        m_camPos = glm::vec3(0.0f, 0.0f, m_orbitDistance);

        m_camPos = glm::vec3(rotationY * rotationX * glm::vec4(m_camPos, 1.0f));

        m_view = glm::lookAt(m_camPos, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    }

public:
    explicit OrbitCamera(float orbitDistance) : m_orbitDistance(orbitDistance) {
        updateViewMatrix();
    }

    glm::mat4& getViewMatrix() {
        return m_view;
    }

    void handleMouseDragEvent(double deltaX, double deltaY) {
        float sensitivity = 0.1f;
        m_orbitPan -= deltaX * sensitivity;
        m_orbitTilt -= deltaY * sensitivity;

        // Clamp tilt to avoid flipping
        m_orbitTilt = glm::clamp(m_orbitTilt, -89.0f, 89.0f);

        updateViewMatrix();
    }

    void handleMouseScrollEvent(double deltaY) {
        float factor = 1.0f + deltaY * 0.1f;
        m_orbitDistance = glm::clamp(m_orbitDistance * factor, 1.0f, 9.0f);

        updateViewMatrix();
    }

    [[nodiscard]] glm::vec3 getCamPos() const {
        return m_camPos;
    }

};


#endif //ORBITCAMERA_H
