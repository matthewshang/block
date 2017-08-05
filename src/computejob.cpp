#include "computejob.h"

#include <glm/gtc/matrix_transform.hpp>

#include "blocks.h"

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

void ComputeJob::getLights(Chunk &c, glm::ivec3 &delta, std::queue<LightNode> &queue)
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

static void makeCube(std::vector<float> &vertices, float x, float y, float z, bool faces[6], int type, float light[6][4])
{
    static const glm::vec3 positions[6][4] = {
        { glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec3(-0.5f,  0.5f,  0.5f), glm::vec3(0.5f,  0.5f,  0.5f), glm::vec3(0.5f, -0.5f,  0.5f) },
        { glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(0.5f,  0.5f, -0.5f), glm::vec3(-0.5f,  0.5f, -0.5f), glm::vec3(-0.5f, -0.5f, -0.5f) },
        { glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(-0.5f,  0.5f, -0.5f), glm::vec3(-0.5f,  0.5f,  0.5f), glm::vec3(-0.5f, -0.5f,  0.5f) },
        { glm::vec3(0.5f, -0.5f,  0.5f), glm::vec3(0.5f,  0.5f,  0.5f), glm::vec3(0.5f,  0.5f, -0.5f), glm::vec3(0.5f, -0.5f, -0.5f) },
        { glm::vec3(-0.5f,  0.5f,  0.5f), glm::vec3(-0.5f,  0.5f, -0.5f), glm::vec3(0.5f,  0.5f, -0.5f), glm::vec3(0.5f,  0.5f,  0.5f) },
        { glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec3(0.5f, -0.5f,  0.5f), glm::vec3(0.5f, -0.5f, -0.5f) }
    };

    static const glm::vec2 texcoords[6] = {
        glm::vec2(0.0f, 0.0f), glm::vec2(0.0f, 1.0f), glm::vec2(1.0f, 1.0f), glm::vec2(1.0f, 0.0f)
    };

    static const int indices[6] = {
        0, 1, 2, 0, 2, 3
    };

    static const int flipped[6] = {
        0, 1, 3, 3, 1, 2
    };

    float s = 1.0f / 16.0f;

    for (int i = 0; i < 6; i++)
    {
        if (!faces[i]) continue;

        int idx = Blocks::faces[type][i];
        float tu = s * static_cast<float>(idx % 16);
        float tv = 1.0f - s * static_cast<float>(idx / 16) - s;

        // https://0fps.net/2013/07/03/ambient-occlusion-for-minecraft-like-worlds/
        bool flip = light[i][0] + light[i][2] > light[i][1] + light[i][3];
        for (int v = 0; v < 6; v++)
        {
            int j = flip ? flipped[v] : indices[v];
            vertices.push_back(positions[i][j].x + x);
            vertices.push_back(positions[i][j].y + y);
            vertices.push_back(positions[i][j].z + z);
            vertices.push_back(tu + texcoords[j].x * s);
            vertices.push_back(tv + texcoords[j].y * s);
            float lightVal = (static_cast<float>(light[i][j]) + 1.0f) / 16.0f;
            lightVal = (std::max)(lightVal, 0.1f);
            vertices.push_back(lightVal);
        }
    }
}

static void makePlant(std::vector<float> &vertices, float x, float y, float z, int type, int light)
{
    static const glm::vec3 positions[2][4] = {
        { glm::vec3(0.5f,  0.5f,  0.0f), glm::vec3(0.5f, -0.5f,  0.0f), glm::vec3(-0.5f, -0.5f,  0.0f), glm::vec3(-0.5f,  0.5f,  0.0f) },
        { glm::vec3(0.0f,  0.5f,  0.5f), glm::vec3(0.0f, -0.5f,  0.5f), glm::vec3(0.0f, -0.5f, -0.5f), glm::vec3(0.0f,  0.5f, -0.5f) }
    };

    static const glm::vec2 texcoords[4] = {
        glm::vec2(1.0f, 1.0f), glm::vec2(1.0f, 0.0f), glm::vec2(0.0f, 0.0f), glm::vec2(0.0f, 1.0f)
    };

    static const int indices[6] = {
        0, 1, 2, 0, 2, 3
    };

    glm::mat4 model;
    model = glm::rotate(model, 45.0f, glm::vec3(0, 1, 0));

    float s = 1.0f / 16.0f;
    for (int i = 0; i < 2; i++)
    {
        int idx = Blocks::faces[type][i];
        float tu = s * static_cast<float>(idx % 16);
        float tv = 1.0f - s * static_cast<float>(idx / 16) - s;
        for (int v = 0; v < 6; v++)
        {
            int j = indices[v];
            glm::vec4 pos = model * glm::vec4(positions[i][j], 1.0f);
            vertices.push_back(pos.x + x);
            vertices.push_back(pos.y + y);
            vertices.push_back(pos.z + z);
            vertices.push_back(tu + texcoords[j].x * s);
            vertices.push_back(tv + texcoords[j].y * s);
            vertices.push_back((static_cast<float>(light) + 1.0f) / 16.0f);
        }
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
                    makePlant(m_vertices, x + pos.x * 16, y + pos.y * 16, z + pos.z * 16,
                        type, m_chunk.getLight(x, y, z));
                }
                else
                {
                    makeCube(m_vertices, x + pos.x * 16, y + pos.y * 16, z + pos.z * 16, visible,
                        type, light);
                }

                for (int i = 0; i < 6; i++) total += visible[i] ? 1 : 0;
            }
        }
    }

    m_empty = total == 0;
}