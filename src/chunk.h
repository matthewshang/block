#pragma once

#include <cstdint>

#include <glm/glm.hpp>
#include <glad/glad.h>

const int CHUNK_SIZE = 16;

class Chunk
{
public:
    enum Neighbor
    {
        Front, Back, Left, Right, Top, Bottom
    };

    Chunk(glm::ivec3 pos);
    ~Chunk();

    void buildMesh();

    void bind();
    int getVertexCount();
    bool isEmpty();
    void setBlock(int x, int y, int z, uint8_t type);
    void hookNeighbor(Neighbor n, Chunk *chunk);
    int getNumNeighbors() { return m_numNeighbors; };

    const glm::ivec3 &getCoords() { return m_pos; };
    const glm::vec3 &getCenter() { return m_worldCenter; };

private:
    void initBlocks();

    GLuint m_vao;
    GLuint m_vbo;
    int m_vertexCount;
    bool m_empty;
    bool m_dirty;

    Chunk *m_neighbors[6];
    int m_numNeighbors;

    glm::ivec3 m_pos;
    glm::vec3 m_worldCenter;
    uint8_t m_blocks[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];
};