#pragma once

#include <glm/glm.hpp>

class Camera
{
public:
    enum Movement
    {
        FOWARD, BACKWARD, LEFT, RIGHT, UP, DOWN
    };

    Camera() = default;

    Camera(glm::vec3 pos) : m_pos(pos), m_front(glm::vec3(0.0f, 0.0f, -1.0f)), m_up(glm::vec3(0.0f, 1.0f, 0.0f)),
        m_worldUp(glm::vec3(0.0f, 1.0f, 0.0f)), m_yaw(-90.0f), m_pitch(0.0f)
    {
        updateVectors();
    }

    glm::mat4 getView();
    const glm::vec3 &getPos() const { return m_pos; };
    void processKeyboard(Movement dir, float dt);
    void processMouse(float dx, float dy);

private:
    void updateVectors();

    glm::vec3 m_pos;
    glm::vec3 m_front;
    glm::vec3 m_up;
    glm::vec3 m_right;
    glm::vec3 m_worldUp;

    float m_yaw;
    float m_pitch;

    const float m_moveSpeed = 5.0f;
    const float m_mouseSens = 0.1f;
};