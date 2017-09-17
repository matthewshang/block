#include "chunk.h"

#include <chrono>
#include <iostream>
#include <queue>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "blocks.h"
#include "computejob.h"
#include "timer.h"
#include "world.h"

// Data order: X -> Z -> Y

Chunk::Chunk(glm::ivec3 pos) : m_pos(pos), m_dirty(false), m_glDirty(true), m_vertices(),
m_lightmap{ 0 }, m_empty(true), m_blocks{}
{
    m_worldCenter = glm::vec3(pos.x * 16 + 8, pos.y * 16 + 8, pos.z * 16 + 8);

    m_mesh = std::make_unique<Mesh>(m_vertices, std::vector<int>{3, 2, 1, 1}, true, false);
}

bool Chunk::isEmpty()
{
    return m_empty;
}

void Chunk::setBlock(int x, int y, int z, uint8_t type)
{
    m_blocks[x * 16 * 16 + z * 16 + y] = type;
    m_dirty = true;
}

uint8_t Chunk::getBlock(int x, int y, int z)
{
    return m_blocks[x * 16 * 16 + z * 16 + y];
}

uint8_t Chunk::getBlock(const glm::ivec3 &pos)
{
    return m_blocks[pos.x * 16 * 16 + pos.z * 16 + pos.y];
}

void Chunk::setSunlight(int x, int y, int z, int val)
{
    int idx = x * 16 * 16 + z * 16 + y;
    m_lightmap[idx] = (m_lightmap[idx] & 0xF) | (val << 4);
}

int Chunk::getSunlight(int x, int y, int z)
{
    return (m_lightmap[x * 16 * 16 + z * 16 + y] >> 4) & 0xF;
}

int Chunk::getSunlight(const glm::ivec3 &pos)
{
    return (m_lightmap[pos.x * 16 * 16 + pos.z * 16 + pos.y] >> 4) & 0xF;
}

void Chunk::setLight(int x, int y, int z, int val)
{
    int idx = x * 16 * 16 + z * 16 + y;
    m_lightmap[idx] = (m_lightmap[idx] & 0xF0) | val;
}

int Chunk::getLight(int x, int y, int z)
{
    return (m_lightmap[x * 16 * 16 + z * 16 + y]) & 0xF;
}

int Chunk::getLight(const glm::ivec3 &pos)
{
    return (m_lightmap[pos.x * 16 * 16 + pos.z * 16 + pos.y]) & 0xF;
}

void Chunk::bufferData()
{
    if (m_glDirty)
    {
        m_mesh->updateData(m_vertices);
        m_glDirty = false;
    }
}

void Chunk::initBlocks()
{
    for (int x = 0; x < CHUNK_SIZE; x++)
    {
        for (int y = 0; y < CHUNK_SIZE; y++)
        {
            for (int z = 0; z < CHUNK_SIZE; z++)
            {
                m_blocks[x * 16 * 16 + z * 16 + y] = Blocks::Air;
            }
        }
    }
    m_empty = true;
}