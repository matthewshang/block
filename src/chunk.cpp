#include "chunk.h"

#include <iostream>

const int Chunk::opposites[6] = {
    1, 0, 3, 2, 5, 4
};

Chunk::Chunk(glm::ivec3 pos) : m_pos(pos), m_dirty(true), m_glDirty(true), m_vertices()
{
    m_worldCenter = glm::vec3(pos.x * 16 + 8, pos.y * 16 + 8, pos.z * 16 + 8);

    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);

    glGenBuffers(1, &m_vbo);
    initBlocks();
    buildMesh();
    bufferData();

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

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

void makeCube(std::vector<float> &vertices, float x, float y, float z, bool faces[6])
{
    static const glm::vec3 positions[6][4] = {
        { glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3( 0.5f, -0.5f, -0.5f), glm::vec3( 0.5f,  0.5f, -0.5f), glm::vec3(-0.5f,  0.5f, -0.5f) },
        { glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec3( 0.5f, -0.5f,  0.5f), glm::vec3( 0.5f,  0.5f,  0.5f), glm::vec3(-0.5f,  0.5f,  0.5f) },
        { glm::vec3(-0.5f,  0.5f,  0.5f), glm::vec3(-0.5f,  0.5f, -0.5f), glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(-0.5f, -0.5f,  0.5f) },
        { glm::vec3( 0.5f,  0.5f,  0.5f), glm::vec3( 0.5f,  0.5f, -0.5f), glm::vec3( 0.5f, -0.5f, -0.5f), glm::vec3( 0.5f, -0.5f,  0.5f) },
        { glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3( 0.5f, -0.5f, -0.5f), glm::vec3( 0.5f, -0.5f,  0.5f), glm::vec3(-0.5f, -0.5f,  0.5f) },
        { glm::vec3(-0.5f,  0.5f, -0.5f), glm::vec3( 0.5f,  0.5f, -0.5f), glm::vec3( 0.5f,  0.5f,  0.5f), glm::vec3(-0.5f,  0.5f,  0.5f) }
    };

    static const glm::vec2 texcoords[6][4] = {
        { glm::vec2(0.0f, 0.0f), glm::vec2(1.0f, 0.0f), glm::vec2(1.0f, 1.0f), glm::vec2(0.0f, 1.0f) },
        { glm::vec2(0.0f, 0.0f), glm::vec2(1.0f, 0.0f), glm::vec2(1.0f, 1.0f), glm::vec2(0.0f, 1.0f) },
        { glm::vec2(1.0f, 0.0f), glm::vec2(1.0f, 1.0f), glm::vec2(0.0f, 1.0f), glm::vec2(0.0f, 0.0f) },
        { glm::vec2(1.0f, 0.0f), glm::vec2(1.0f, 1.0f), glm::vec2(0.0f, 1.0f), glm::vec2(0.0f, 0.0f) },
        { glm::vec2(0.0f, 1.0f), glm::vec2(1.0f, 1.0f), glm::vec2(1.0f, 0.0f), glm::vec2(0.0f, 0.0f) },
        { glm::vec2(0.0f, 1.0f), glm::vec2(1.0f, 1.0f), glm::vec2(1.0f, 0.0f), glm::vec2(0.0f, 0.0f) }
    };

    static const int indices[6][6] = {
        { 0, 3, 2, 0, 1, 2 },
        { 0, 1, 2, 0, 2, 3 },
        { 0, 2, 3, 0, 1, 2 },
        { 0, 3, 2, 0, 2, 1 },
        { 0, 2, 3, 0, 1, 2 },
        { 0, 3, 2, 0, 2, 1 }
    };

    for (int i = 0; i < 6; i++)
    {
        if (!faces[i]) continue;

        for (int v = 0; v < 6; v++)
        {
            int j = indices[i][v];
            vertices.push_back(positions[i][j].x + x);
            vertices.push_back(positions[i][j].y + y);
            vertices.push_back(positions[i][j].z + z);
            vertices.push_back(texcoords[i][j].x);
            vertices.push_back(texcoords[i][j].y);
        }
    }
}

void Chunk::buildMesh()
{
    if (m_dirty)
    {
        int total = 0;
        m_vertices.clear();

        for (int x = 0; x < CHUNK_SIZE; x++)
        {
            for (int y = 0; y < CHUNK_SIZE; y++)
            {
                for (int z = 0; z < CHUNK_SIZE; z++)
                {
                    if (m_blocks[x][y][z] != 0)
                    {
                        bool visible[6] = { true, true, true, true, true, true };
                        if (z > 0)              visible[0] = !m_blocks[x][y][z - 1];
                        if (z < CHUNK_SIZE - 1) visible[1] = !m_blocks[x][y][z + 1];
                        if (x > 0)              visible[2] = !m_blocks[x - 1][y][z];
                        if (x < CHUNK_SIZE - 1) visible[3] = !m_blocks[x + 1][y][z];
                        if (y > 0)              visible[4] = !m_blocks[x][y - 1][z];
                        if (y < CHUNK_SIZE - 1) visible[5] = !m_blocks[x][y + 1][z];

                        makeCube(m_vertices, x + m_pos.x * 16, y + m_pos.y * 16, z + m_pos.z * 16, visible);

                        for (int i = 0; i < 6; i++) total += visible[i] ? 1 : 0;
                    }
                }
            }
        }
        //std::cout << "Faces: " << total << std::endl;
        m_vertexCount = m_vertices.size() / 5;
        m_dirty = false;
        m_glDirty = true;
        m_empty = total == 0;

        //bufferData();
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

void Chunk::initBlocks()
{
    for (int x = 0; x < CHUNK_SIZE; x++)
    {
        for (int y = 0; y < CHUNK_SIZE; y++)
        {
            for (int z = 0; z < CHUNK_SIZE; z++)
            {
                m_blocks[x][y][z] = 0;
            }
        }
    }
    m_empty = true;
}