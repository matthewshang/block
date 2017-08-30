#pragma once

#include <glm/glm.hpp>

#include "camera.h"
#include "common.h"

class InputManager;

class Player
{
public:
    Player(const glm::vec3 &pos, Camera &camera) : 
        m_pos(pos), m_camera(camera), m_flying(false), m_fov(45.0f),
        m_flyTimer(0.0f) {};

    void update(float dt, ChunkMap &chunks, InputManager &input);

    const glm::vec3 &getPos() const { return m_pos; };
    void setPos(const glm::vec3 &pos) { m_pos = pos; };

    float getFov() const { return m_fov; };

private:
    bool collide(glm::vec3 &pos, ChunkMap &chunks);

    glm::vec3 m_pos;
    glm::vec3 m_vel;
    glm::vec3 m_moveVel;

    bool m_flying;
    float m_flyTimer;

    bool m_sprinting;
    float m_sprintTimer;
    float m_fov;

    Camera &m_camera;
};