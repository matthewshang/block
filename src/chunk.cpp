#include "chunk.h"

#include <chrono>
#include <iostream>
#include <queue>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "blocks.h"
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
    //buildMesh();
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

void makeCube(std::vector<float> &vertices, float x, float y, float z, bool faces[6], int type, float light[6][4])
{
    static const glm::vec3 positions[6][4] = {
        { glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec3(-0.5f,  0.5f,  0.5f), glm::vec3( 0.5f,  0.5f,  0.5f), glm::vec3( 0.5f, -0.5f,  0.5f) },
        { glm::vec3( 0.5f, -0.5f, -0.5f), glm::vec3( 0.5f,  0.5f, -0.5f), glm::vec3(-0.5f,  0.5f, -0.5f), glm::vec3(-0.5f, -0.5f, -0.5f) },
        { glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(-0.5f,  0.5f, -0.5f), glm::vec3(-0.5f,  0.5f,  0.5f), glm::vec3(-0.5f, -0.5f,  0.5f) },
        { glm::vec3( 0.5f, -0.5f,  0.5f), glm::vec3( 0.5f,  0.5f,  0.5f), glm::vec3( 0.5f,  0.5f, -0.5f), glm::vec3( 0.5f, -0.5f, -0.5f) },
        { glm::vec3(-0.5f,  0.5f,  0.5f), glm::vec3(-0.5f,  0.5f, -0.5f), glm::vec3( 0.5f,  0.5f, -0.5f), glm::vec3( 0.5f,  0.5f,  0.5f) },
        { glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec3( 0.5f, -0.5f,  0.5f), glm::vec3( 0.5f, -0.5f, -0.5f) }
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

void makePlant(std::vector<float> &vertices, float x, float y, float z, int type, int light)
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

    Timer timer, lighting, meshing;
    timer.start();

    ChunkData data;
    std::memset(data.lightMap, 0, 27 * CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE);
    std::memset(data.opaqueMap, 0, 27 * CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE);

    lighting.start();
    calcLighting(chunks, data);
    //lighting.log("  Lighting time: ");
    meshing.start();
    buildMesh(data);
    //meshing.log("   Meshing time: ");

    //timer.log("Compute time: ");
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

void Chunk::getLights(glm::ivec3 &delta, std::queue<LightNode> &queue, ChunkData &blocks)
{
    glm::ivec3 d = delta * 16;
    glm::ivec3 d2 = (delta + 1) * 16;
    for (int x = 0; x < CHUNK_SIZE; x++)
    {
        for (int y = 0; y < CHUNK_SIZE; y++)
        {
            for (int z = 0; z < CHUNK_SIZE; z++)
            {
                int type = getBlock(x, y, z);
                if (Blocks::isLight(type))
                {
                    queue.emplace(d.x + x, d.y + y, d.z + z, 15);
                }
                bool val = !Blocks::isLight(type) && !Blocks::isTransparent(type);
                blocks.opaqueMap[d2.x + x][d2.y + y][d2.z + z] = val;
            }
        }
    }
}

void Chunk::calcLighting(ChunkMap &chunks, ChunkData &data)
{
    Timer t_queue;

    std::queue<LightNode> lightQueue;
    for (int a = -1; a < 2; a++)
    {
        for (int b = -1; b < 2; b++)
        {
            for (int c = -1; c < 2; c++)
            {
                if (a == 0 && b == 0 && c == 0)
                {
                    this->getLights(glm::ivec3(0), lightQueue, data);
                    continue;
                }
                glm::ivec3 coords = m_pos + glm::ivec3(a, b, c);
                auto neighbor = chunks.find(coords);
                if (neighbor == chunks.end())
                    continue;

                neighbor->second->getLights(glm::ivec3(a, b, c), lightQueue, data);
            }
        }
    }

    //t_queue.start();
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

        int val = data.lightMap[x + CHUNK_SIZE][y + CHUNK_SIZE][z + CHUNK_SIZE];
        if (val >= light)
            continue;

        if (data.opaqueMap[x + CHUNK_SIZE][y + CHUNK_SIZE][z + CHUNK_SIZE])
            continue;

        data.lightMap[x + CHUNK_SIZE][y + CHUNK_SIZE][z + CHUNK_SIZE] = light--;
        lightQueue.emplace(x - 1, y, z, light);
        lightQueue.emplace(x + 1, y, z, light);
        lightQueue.emplace(x, y - 1, z, light);
        lightQueue.emplace(x, y + 1, z, light);
        lightQueue.emplace(x, y, z - 1, light);
        lightQueue.emplace(x, y, z + 1, light);
    }
    //t_queue.log("prop time: ");

    for (int x = 0; x < CHUNK_SIZE; x++)
    {
        for (int y = 0; y < CHUNK_SIZE; y++)
        {
            for (int z = 0; z < CHUNK_SIZE; z++)
            {
                setLight(x, y, z, data.lightMap[x + CHUNK_SIZE][y + CHUNK_SIZE][z + CHUNK_SIZE]);
            }
        }
    }
}

void Chunk::smoothLighting(int x, int y, int z, ChunkData &data, float light[6][4])
{
    static const int indices[6][4] = {
        { 0, 1, 2, 3 }, { 7, 6, 5, 4 }, { 4, 5, 1, 0 },
        { 3, 2, 6, 7 }, { 1, 5, 6, 2 }, { 4, 0, 3, 7 }
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
                data.lightMap[pos.x + d.x][pos.y + d.y][pos.z + d.z]);
        }
        corners[i] /= 8.0f;
    }

    for (int i = 0; i < 6; i++)
    {
        for (int j = 0; j < 4; j++)
            light[i][j] = corners[indices[i][j]];
    }
}

void Chunk::faceLighting(int x, int y, int z, ChunkData & data, float light[6][4])
{
    static const glm::ivec3 off[6] = {
        glm::ivec3(0, 0, 1), glm::ivec3(0, 0, -1), glm::ivec3(-1, 0, 0),
        glm::ivec3(1, 0, 0), glm::ivec3(0, 1, 0), glm::ivec3(0, -1, 0)
    };

    for (int i = 0; i < 6; i++)
    {
        const glm::ivec3 &d = off[i];
        for (int j = 0; j < 4; j++)
            light[i][j] = static_cast<float>(data.lightMap[x + d.x][y + d.y][z + d.z]);
    }
}

void Chunk::buildMesh(ChunkData &data)
{
    int total = 0;
    m_vertices.clear();

    for (int x = 0; x < CHUNK_SIZE; x++)
    {
        for (int y = 0; y < CHUNK_SIZE; y++)
        {
            for (int z = 0; z < CHUNK_SIZE; z++)
            {
                if (m_blocks[x][y][z] == Blocks::Air)
                    continue;

                bool visible[6] = { true, true, true, true, true, true };
                float light[6][4] = { 0.0f };

                int dx = x + CHUNK_SIZE;
                int dy = y + CHUNK_SIZE;
                int dz = z + CHUNK_SIZE;

                smoothLighting(dx, dy, dz, data, light);
                //faceLighting(dx, dy, dz, data, light);

                visible[0] = !data.opaqueMap[dx][dy][dz + 1];
                visible[1] = !data.opaqueMap[dx][dy][dz - 1];
                visible[2] = !data.opaqueMap[dx - 1][dy][dz];
                visible[3] = !data.opaqueMap[dx + 1][dy][dz];
                visible[4] = !data.opaqueMap[dx][dy + 1][dz];
                visible[5] = !data.opaqueMap[dx][dy - 1][dz];

                if (Blocks::isPlant(m_blocks[x][y][z]))
                {
                    makePlant(m_vertices, x + m_pos.x * 16, y + m_pos.y * 16, z + m_pos.z * 16,
                        m_blocks[x][y][z], getLight(x, y, z));
                }
                else
                {
                    makeCube(m_vertices, x + m_pos.x * 16, y + m_pos.y * 16, z + m_pos.z * 16, visible,
                        m_blocks[x][y][z], light);
                }

                for (int i = 0; i < 6; i++) total += visible[i] ? 1 : 0;
            }
        }
    }

    m_vertexCount = m_vertices.size() / 6;
    m_dirty = false;
    m_glDirty = true;
    m_empty = total == 0;
}