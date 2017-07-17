#pragma once

#include <cstdint>

#include <glm/glm.hpp>
#include <glad/glad.h>

#include "chunkcoord.h"

const int CHUNK_SIZE = 16;

class Chunk
{
public:
    Chunk(ChunkCoord pos);
    ~Chunk();

    void buildMesh();

    void bind();
    int getVertexCount();
    bool isEmpty();
    void setBlock(int x, int y, int z, uint8_t type);

    const ChunkCoord &getCoords() { return m_pos; };

private:
    void initBlocks();

    GLuint m_vao;
    GLuint m_vbo;
    int m_vertexCount;
    bool m_empty;
    bool m_dirty;

    ChunkCoord m_pos;
    uint8_t m_blocks[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];
};