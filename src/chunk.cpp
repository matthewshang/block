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
m_lightmap{}, m_empty(true)
{
    m_worldCenter = glm::vec3(pos.x * 16 + 8, pos.y * 16 + 8, pos.z * 16 + 8);

    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);

    glGenBuffers(1, &m_vbo);
    initBlocks();
    bufferData();

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

Chunk::~Chunk()
{
    glDeleteVertexArrays(1, &m_vao);
    glDeleteBuffers(1, &m_vbo);
}

void Chunk::bind()
{
    glBindVertexArray(m_vao);
}

int Chunk::getVertexCount()
{
    return m_vertexCount;
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
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(float), m_vertices.data(), GL_STATIC_DRAW);
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