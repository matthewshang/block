#pragma once

#include <cstdint>
#include <map>
#include <queue>
#include <vector>

#include <glm/glm.hpp>
#include <glad/glad.h>

#include "common.h"

const int CHUNK_SIZE = 16;

class Chunk
{
public:
    friend class ComputeJob;

    static const int opposites[6];

    Chunk(glm::ivec3 pos);
    ~Chunk();

    void bufferData();

    void compute(ChunkMap &chunks);

    void bind();
    void setDirty(bool dirty) { m_dirty = dirty; };
    bool isDirty() { return m_dirty; };
    int getVertexCount();
    bool isEmpty();
    void setBlock(int x, int y, int z, uint8_t type);
    uint8_t getBlock(int x, int y, int z);
    void setSunlight(int x, int y, int z, int val);
    int getSunlight(int x, int y, int z);
    void setLight(int x, int y, int z, int val);
    int getLight(int x, int y, int z);

    const glm::ivec3 &getCoords() { return m_pos; };
    const glm::vec3 &getCenter() { return m_worldCenter; };

private:
    void initBlocks();

    GLuint m_vao;
    GLuint m_vbo;
    int m_vertexCount;
    bool m_empty;
    bool m_dirty;
    bool m_glDirty;

    glm::ivec3 m_pos;
    glm::vec3 m_worldCenter;
    uint8_t m_blocks[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];
    uint8_t m_lightmap[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];
    std::vector<float> m_vertices;
};