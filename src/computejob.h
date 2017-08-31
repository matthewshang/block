#pragma once

#include <queue>
#include <vector>

#include "chunk.h"
#include "world.h"

class ComputeJob
{
public:
    ComputeJob(Chunk &chunk, World &world);

    void execute();

    void transfer();

private:

    void smoothLighting(int x, int y, int z, float light[6][4]);
    void smoothLighting2(int x, int y, int z, float light[6][4], float sunlight[6][4]);
    void faceLighting(const glm::ivec3 &worldPos, float light[6][4]);
    int getBlockType(const glm::ivec3 &worldPos);
    void buildMesh();

    World &m_world;
    ChunkMap &m_chunks;
    Chunk &m_chunk;
    std::vector<float> m_vertices;
    bool m_empty;
};