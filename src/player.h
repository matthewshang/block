#pragma once

#include <glm/glm.hpp>

#include "camera.h"
#include "common.h"

class InputManager;

class Player
{
public:
    Player(const glm::vec3 &pos, Camera &camera) : 
        m_pos(pos), m_camera(camera), m_flying(false) {};

    void update(float dt, ChunkMap &chunks, InputManager &input);

    const glm::vec3 &getPos() const { return m_pos; };
    void setPos(const glm::vec3 &pos) { m_pos = pos; };

private:
    bool collide(glm::vec3 &pos, ChunkMap &chunks);

    glm::vec3 m_pos;
    glm::vec3 m_vel;

    bool m_flying;
    float m_flyTimer;

    Camera &m_camera;
};