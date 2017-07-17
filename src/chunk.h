#pragma once

#include <cstdint>

#include <glm/glm.hpp>
#include <glad/glad.h>

const int CHUNK_SIZE = 16;

class Chunk
{
public:
    Chunk();
    ~Chunk();

    void bind();
    int getVertexCount();

private:
    void buildVBO();
    void initBlocks();

    GLuint m_vao;
    GLuint m_vbo;
    int m_vertexCount;

    uint8_t m_blocks[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];
};