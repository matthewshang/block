#include "computejob.h"

#include <iostream>

#include <glm/gtc/matrix_transform.hpp>

#include "blocks.h"
#include "geometry.h"

ComputeJob::ComputeJob(Chunk &chunk, ChunkMap &map) :
    m_chunk(chunk), m_chunkmap(map)
{
    std::memset(m_data.lightMap, 0, 27 * CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE);
    std::memset(m_data.opaqueMap, 0, 27 * CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE);

}

void ComputeJob::execute()
{
    calcLighting(m_chunkmap);
    buildMesh();
}

void ComputeJob::transfer()
{
    for (int x = 0; x < CHUNK_SIZE; x++)
    {
        for (int y = 0; y < CHUNK_SIZE; y++)
        {
            for (int z = 0; z < CHUNK_SIZE; z++)
            {
                m_chunk.setLight(x, y, z, m_data.lightMap[x + CHUNK_SIZE][y + CHUNK_SIZE][z + CHUNK_SIZE]);
            }
        }
    }

    m_chunk.m_vertexCount = m_vertices.size() / 6;
    m_chunk.m_vertices = std::move(m_vertices);
    m_chunk.m_dirty = false;
    m_chunk.m_glDirty = true;
    m_chunk.m_empty = m_empty;
}

void ComputeJob::getLights(Chunk &c, const glm::ivec3 &delta, std::queue<LightNode> &queue)
{
    glm::ivec3 d = delta * 16;
    glm::ivec3 d2 = (delta + 1) * 16;
    for (int x = 0; x < CHUNK_SIZE; x++)
    {
        for (int y = 0; y < CHUNK_SIZE; y++)
        {
            for (int z = 0; z < CHUNK_SIZE; z++)
            {
                int type = c.getBlock(x, y, z);
                if (Blocks::isLight(type))
                {
                    queue.emplace(d.x + x, d.y + y, d.z + z, 15);
                }
                bool val = !Blocks::isLight(type) && !Blocks::isTransparent(type);
                m_data.opaqueMap[d2.x + x][d2.y + y][d2.z + z] = val;
            }
        }
    }
}

void ComputeJob::calcLighting(ChunkMap &chunks)
{
    std::queue<LightNode> lightQueue;
    for (int a = -1; a < 2; a++)
    {
        for (int b = -1; b < 2; b++)
        {
            for (int c = -1; c < 2; c++)
            {
                if (a == 0 && b == 0 && c == 0)
                {
                    getLights(m_chunk, glm::ivec3(0), lightQueue);
                    continue;
                }
                glm::ivec3 coords = m_chunk.getCoords() + glm::ivec3(a, b, c);
                auto neighbor = chunks.find(coords);
                if (neighbor == chunks.end())
                    continue;

                getLights(*neighbor->second, glm::ivec3(a, b, c), lightQueue);
            }
        }
    }

    const int MIN = -CHUNK_SIZE;
    const int MAX = 2 * CHUNK_SIZE - 1;

    while (!lightQueue.empty())
    {
        LightNode &node = lightQueue.front();
        int x = node.x,
            y = node.y,
            z = node.z,
            light = node.light;
        lightQueue.pop();

        if (x < MIN || x > MAX || y < MIN || y > MAX || z < MIN || z > MAX)
            continue;

        int val = m_data.lightMap[x + CHUNK_SIZE][y + CHUNK_SIZE][z + CHUNK_SIZE];
        if (val >= light)
            continue;

        if (m_data.opaqueMap[x + CHUNK_SIZE][y + CHUNK_SIZE][z + CHUNK_SIZE])
            continue;

        m_data.lightMap[x + CHUNK_SIZE][y + CHUNK_SIZE][z + CHUNK_SIZE] = light--;
        lightQueue.emplace(x - 1, y, z, light);
        lightQueue.emplace(x + 1, y, z, light);
        lightQueue.emplace(x, y - 1, z, light);
        lightQueue.emplace(x, y + 1, z, light);
        lightQueue.emplace(x, y, z - 1, light);
        lightQueue.emplace(x, y, z + 1, light);
    }
}

void ComputeJob::smoothLighting(int x, int y, int z, float light[6][4])
{
    static const int indices[6][4] = {
        { 0, 1, 2, 3 },{ 7, 6, 5, 4 },{ 4, 5, 1, 0 },
        { 3, 2, 6, 7 },{ 1, 5, 6, 2 },{ 4, 0, 3, 7 }
    };

    static const glm::ivec3 start[8] = {
        glm::ivec3(-1, -1, 0), glm::ivec3(-1, 0, 0), glm::ivec3(0, 0, 0), glm::ivec3(0, -1, 0),
        glm::ivec3(-1, -1, -1), glm::ivec3(-1, 0, -1), glm::ivec3(0, 0, -1), glm::ivec3(0, -1, -1)
    };

    static const glm::ivec3 off[8] = {
        glm::ivec3(0, 0, 0), glm::ivec3(1, 0, 0), glm::ivec3(0, 1, 0), glm::ivec3(0, 0, 1),
        glm::ivec3(1, 1, 0), glm::ivec3(1, 0, 1), glm::ivec3(0, 1, 1), glm::ivec3(1, 1, 1)
    };

    float corners[8] = { 0.0f };

    for (int i = 0; i < 8; i++)
    {
        glm::ivec3 pos = start[i] + glm::ivec3(x, y, z);
        for (int j = 0; j < 8; j++)
        {
            const glm::ivec3 &d = off[j];
            corners[i] += static_cast<float>(
                m_data.lightMap[pos.x + d.x][pos.y + d.y][pos.z + d.z]);
        }
        corners[i] /= 8.0f;
    }

    for (int i = 0; i < 6; i++)
    {
        for (int j = 0; j < 4; j++)
            light[i][j] = corners[indices[i][j]];
    }
}

void ComputeJob::smoothLighting2(int x, int y, int z, float light[6][4])
{
    static const glm::ivec3 start[6][4] = {
        { glm::ivec3(-1, -1, 1), glm::ivec3(-1, 0, 1), glm::ivec3(0, 0, 1), glm::ivec3(0, -1, 1) },
        { glm::ivec3(0, -1, -1), glm::ivec3(0, 0, -1), glm::ivec3(-1, 0, -1), glm::ivec3(-1, -1, -1) },
        { glm::ivec3(-1, -1, -1), glm::ivec3(-1, 0, -1), glm::ivec3(-1, 0, 0), glm::ivec3(-1, -1, 0) },
        { glm::ivec3(1, -1, 1), glm::ivec3(1, 0, 1), glm::ivec3(1, 0, 0), glm::ivec3(1, -1, 0) },
        { glm::ivec3(-1, 1, 1), glm::ivec3(-1, 1, 0), glm::ivec3(0, 1, 0), glm::ivec3(0, 1, 1) },
        { glm::ivec3(-1, -1, -1), glm::ivec3(-1, -1, 0), glm::ivec3(0, -1, 0), glm::ivec3(0, -1, -1) }
    };

    static const glm::ivec3 off[6][4] = {
        { glm::ivec3(0, 0, 0), glm::ivec3(0, 1, 0), glm::ivec3(1, 1, 0), glm::ivec3(1, 0, 0) },
        { glm::ivec3(0, 0, 0), glm::ivec3(0, 1, 0), glm::ivec3(1, 1, 0), glm::ivec3(1, 0, 0) },
        { glm::ivec3(0, 0, 0), glm::ivec3(0, 1, 0), glm::ivec3(0, 1, 1), glm::ivec3(0, 0, 1) },
        { glm::ivec3(0, 0, 0), glm::ivec3(0, 1, 0), glm::ivec3(0, 1, -1), glm::ivec3(0, 0, -1) },
        { glm::ivec3(0, 0, 0), glm::ivec3(0, 0, -1), glm::ivec3(1, 0, -1), glm::ivec3(1, 0, 0) },
        { glm::ivec3(0, 0, 0), glm::ivec3(0, 0, 1), glm::ivec3(1, 0, 1), glm::ivec3(1, 0, 0) }
    };

    for (int i = 0; i < 6; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            glm::ivec3 pos = start[i][j] + glm::ivec3(x, y, z);
            float val = 0.0f;
            for (int k = 0; k < 4; k++)
            {
                const glm::ivec3 &d = off[i][k];
                val += static_cast<float>(
                    m_data.lightMap[pos.x + d.x][pos.y + d.y][pos.z + d.z]);
            }
            light[i][j] = val / 4.0f;
        }
    }
}

void ComputeJob::faceLighting(int x, int y, int z, float light[6][4])
{
    static const glm::ivec3 off[6] = {
        glm::ivec3(0, 0, 1), glm::ivec3(0, 0, -1), glm::ivec3(-1, 0, 0),
        glm::ivec3(1, 0, 0), glm::ivec3(0, 1, 0), glm::ivec3(0, -1, 0)
    };

    for (int i = 0; i < 6; i++)
    {
        const glm::ivec3 &d = off[i];
        for (int j = 0; j < 4; j++)
            light[i][j] = static_cast<float>(m_data.lightMap[x + d.x][y + d.y][z + d.z]);
    }
}

void ComputeJob::buildMesh()
{
    int total = 0;
    m_vertices.clear();

    for (int x = 0; x < CHUNK_SIZE; x++)
    {
        for (int y = 0; y < CHUNK_SIZE; y++)
        {
            for (int z = 0; z < CHUNK_SIZE; z++)
            {
                if (m_chunk.getBlock(x, y, z) == Blocks::Air)
                    continue;

                bool visible[6] = { true, true, true, true, true, true };
                float light[6][4] = { 0.0f };

                int dx = x + CHUNK_SIZE;
                int dy = y + CHUNK_SIZE;
                int dz = z + CHUNK_SIZE;

                //smoothLighting(dx, dy, dz, light);
                smoothLighting2(dx, dy, dz, light);
                //faceLighting(dx, dy, dz, light);

                visible[0] = !m_data.opaqueMap[dx][dy][dz + 1];
                visible[1] = !m_data.opaqueMap[dx][dy][dz - 1];
                visible[2] = !m_data.opaqueMap[dx - 1][dy][dz];
                visible[3] = !m_data.opaqueMap[dx + 1][dy][dz];
                visible[4] = !m_data.opaqueMap[dx][dy + 1][dz];
                visible[5] = !m_data.opaqueMap[dx][dy - 1][dz];

                const glm::ivec3 &pos = m_chunk.getCoords();
                int type = m_chunk.getBlock(x, y, z);

                if (Blocks::isPlant(type))
                {
                    Geometry::makePlant(m_vertices, x + pos.x * 16, y + pos.y * 16, z + pos.z * 16,
                        type, m_data.lightMap[dx][dy][dz]);
                }
                else
                {
                    Geometry::makeCube(m_vertices, x + pos.x * 16, y + pos.y * 16, z + pos.z * 16, visible,
                        type, light);
                }

                for (int i = 0; i < 6; i++) total += visible[i] ? 1 : 0;
            }
        }
    }

    m_empty = total == 0;
}