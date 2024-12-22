//
// Created by slice on 12/14/24.
//

#ifndef FPSCAMERA_H
#define FPSCAMERA_H
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "ImGuiWindowCreator.h"
#include "Final/SimplexNoise.h"

enum CameraMovement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 60.0f;
const float SENSITIVITY = 0.04f;
const float ZOOM = 45.0f;

class FPSCamera {
public:
    FPSCamera(glm::vec3 position = glm::vec3{0.0f, 0.0f, 0.0f}, glm::vec3 up = glm::vec3{0.0f, 1.0f, 0.0f},
              float yaw = YAW, float pitch = PITCH) : m_front(glm::vec3{0.0f, 0.0f, -1.0f}), m_movementSpeed(SPEED),
                                                      m_mouseSensitivity(SENSITIVITY), m_zoom(ZOOM) {
        m_position = position;
        m_worldUp = up;
        m_yaw = yaw;
        m_pitch = pitch;

        updateCameraVectors();
    }

    FPSCamera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw,
              float pitch) : m_front(glm::vec3{0.0f, 0.0f, -1.0f}), m_movementSpeed(SPEED),
                             m_mouseSensitivity(SENSITIVITY), m_zoom(ZOOM) {
        m_position = glm::vec3{posX, posY, posZ};
        m_worldUp = glm::vec3{upX, upY, upZ};
        m_yaw = yaw;
        m_pitch = pitch;

        updateCameraVectors();
    }

    [[nodiscard]] glm::mat4 getViewMatrix() const
    {
        return glm::lookAt(m_position, m_position + m_front, m_up);
    }

    [[nodiscard]] glm::vec3 getCamPos() const {
        return m_position;
    }

    void updateHeight(const float height) {
        m_position.y = height;
    }

    [[nodiscard]] float getFov() const {
        return m_zoom;
    }

    void displayViewMatrix() const {
        ImGuiWindowCreator{"FPS Camera view matrix"}
        .display("View", getViewMatrix())
        .end();
    }

    void processKeyboard(CameraMovement direction, float deltaTime)
    {
        float velocity = m_movementSpeed * deltaTime;

        if (direction == FORWARD) {
            m_position += m_front * velocity;
        }

        if (direction == BACKWARD) {
            m_position -= m_front * velocity;
        }

        if (direction == LEFT) {
            m_position -= m_right * velocity;
        }

        if (direction == RIGHT) {
            m_position += m_right * velocity;
        }

        if (direction == UP) {
            m_position += m_up * velocity;
        }

        if (direction == DOWN) {
            m_position -= m_up * velocity;
        }
    }

    void processMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true)
    {
        xoffset *= m_mouseSensitivity;
        yoffset *= m_mouseSensitivity;

        m_yaw   += xoffset;
        m_pitch += yoffset;

        if (constrainPitch)
        {
            if (m_pitch > 89.0f) {
                m_pitch = 89.0f;
            }

            if (m_pitch < -89.0f) {
                m_pitch = -89.0f;
            }
        }

        updateCameraVectors();
    }

    void processMouseScroll(float yoffset)
    {
        m_zoom -= yoffset;

        if (m_zoom < 1.0f) {
            m_zoom = 1.0f;
        }

        if (m_zoom > 45.0f) {
            m_zoom = 45.0f;
        }
    }

private:
    glm::vec3 m_position;
    glm::vec3 m_front;
    glm::vec3 m_up;
    glm::vec3 m_right;
    glm::vec3 m_worldUp;

    float m_yaw;
    float m_pitch;

    float m_movementSpeed;
    float m_mouseSensitivity;
    float m_zoom;

    void updateCameraVectors()
    {
        glm::vec3 front;
        front.x = std::cos(glm::radians(m_yaw)) *  std::cos(glm::radians(m_pitch));
        front.y = std::sin(glm::radians(m_pitch));
        front.z = std::sin(glm::radians(m_yaw)) *  std::cos(glm::radians(m_pitch));

        m_front = glm::normalize(front);
        m_right = glm::normalize(glm::cross(m_front, m_worldUp));
        m_up    = glm::normalize(glm::cross(m_right, m_front));
    }
};


#endif //FPSCAMERA_H
