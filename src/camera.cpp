#include "camera.h"

#include <algorithm>

#include <glm/gtc/matrix_transform.hpp>

glm::mat4 Camera::getView()
{
    return glm::lookAt(m_pos, m_pos + m_front, m_up);
}

void Camera::processKeyboard(Camera::Movement dir, float dt)
{
    float velo = m_moveSpeed * dt;
    switch (dir)
    {
    case Movement::FOWARD: m_pos += m_front * velo; break;
    case Movement::BACKWARD: m_pos -= m_front * velo; break;
    case Movement::LEFT: m_pos -= m_right * velo; break;
    case Movement::RIGHT: m_pos += m_right * velo; break;
    }
}

void Camera::processMouse(float dx, float dy)
{
    m_yaw += dx * m_mouseSens;
    m_pitch += dy * m_mouseSens;

    m_pitch = std::min(std::max(m_pitch, -89.0f), 89.0f);
    updateVectors();
}

void Camera::updateVectors()
{
    glm::vec3 front(
        cos(glm::radians(m_pitch)) * cos(glm::radians(m_yaw)),
        sin(glm::radians(m_pitch)),
        cos(glm::radians(m_pitch)) * sin(glm::radians(m_yaw)));
    m_front = glm::normalize(front);
    m_right = glm::normalize(glm::cross(m_front, m_worldUp));
    m_up = glm::normalize(glm::cross(m_right, m_front));
}