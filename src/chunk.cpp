#include "chunk.h"

#include <chrono>
#include <iostream>
#include <queue>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "blocks.h"
#include "computejob.h"
#include "timer.h"

const int Chunk::opposites[6] = {
    1, 0, 3, 2, 5, 4
};

Chunk::Chunk(glm::ivec3 pos) : m_pos(pos), m_dirty(false), m_glDirty(true), m_vertices(),
m_lightmap{}, m_empty(true), m_blocks{}
{
    m_worldCenter = glm::vec3(pos.x * 16 + 8, pos.y * 16 + 8, pos.z * 16 + 8);

    m_mesh = std::make_unique<Mesh>(m_vertices, 7, std::vector<int>{3, 2, 1, 1}, true, false);
}

bool Chunk::isEmpty()
{
    return m_empty;
}

void Chunk::setBlock(int x, int y, int z, uint8_t type)
{
    m_blocks[x][y][z] = type;
    m_dirty = true;
}

uint8_t Chunk::getBlock(int x, int y, int z)
{
    return m_blocks[x][y][z];
}

void Chunk::setSunlight(int x, int y, int z, int val)
{
    m_lightmap[x][y][z] = (m_lightmap[x][y][z] & 0xF) | (val << 4);
}

int Chunk::getSunlight(int x, int y, int z)
{
    return (m_lightmap[x][y][z] >> 4) & 0xF;
}

void Chunk::setLight(int x, int y, int z, int val)
{
    m_lightmap[x][y][z] = (m_lightmap[x][y][z] & 0xF0) | val;
}

int Chunk::getLight(int x, int y, int z)
{
    return (m_lightmap[x][y][z]) & 0xF;
}

void Chunk::bufferData()
{
    if (m_glDirty)
    {
        m_mesh->updateData(m_vertices);
        m_glDirty = false;
    }
}

void Chunk::compute(ChunkMap &chunks)
{
    if (!m_dirty)
        return;

    ComputeJob job(*this, chunks);
    job.execute();
    job.transfer();
}

void Chunk::initBlocks()
{
    for (int x = 0; x < CHUNK_SIZE; x++)
    {
        for (int y = 0; y < CHUNK_SIZE; y++)
        {
            for (int z = 0; z < CHUNK_SIZE; z++)
            {
                m_blocks[x][y][z] = Blocks::Air;
            }
        }
    }
    m_empty = true;
}