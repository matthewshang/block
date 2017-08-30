#pragma once

#include <deque>
#include <mutex>

#include <glm/glm.hpp>

#include "common.h"

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
    Lighting(ChunkMap &chunks) : m_ops(), m_chunks(chunks) {};

    void pushUpdate(bool isBlock, const glm::ivec3 &min, const glm::ivec3 &max);

    void lightNext();
    bool empty();

private:
    void propegate(int x, int y, int z, int val, const LightOp &op);
    void dirty(Chunk &chunk, const glm::ivec3 &pos);

    std::deque<LightOp> m_ops;
    ChunkMap &m_chunks;
    std::mutex m_mutex;
};