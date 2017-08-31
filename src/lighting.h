#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <deque>
#include <map>
#include <memory>
#include <mutex>

#include <glm/glm.hpp>

//#include "common.h"
#include "world.h"

struct LightOp
{
    LightOp(bool _isBlock, const glm::ivec3 &_min, const glm::ivec3 &_max) :
        isBlock(_isBlock), min(_min), max(_max) {};
    bool isBlock;
    glm::ivec3 min;
    glm::ivec3 max;
};

class Lighting
{
public:
    Lighting(World &world) : m_ops(), m_world(world) {};

    void pushUpdate(bool isBlock, const glm::ivec3 &min, const glm::ivec3 &max);

    void lightNext();
    bool empty();
    int getNumOps() const { return m_ops.size(); };

private:
    void propegate(int x, int y, int z, int val, const LightOp &op);
    void dirty(Chunk &chunk, const glm::ivec3 &pos);

    std::deque<LightOp> m_ops;
    World &m_world;
    std::mutex m_mutex;
};