#pragma once

#include <queue>
#include <vector>

#include "chunk.h"

class ComputeJob
{
public:
    ComputeJob(Chunk &chunk, ChunkMap &map);

    void execute();

    void transfer();

private:
    struct ChunkData
    {
        uint8_t lightMap[3 * CHUNK_SIZE][3 * CHUNK_SIZE][3 * CHUNK_SIZE];
        bool opaqueMap[48][48][48];
    };

    struct LightNode
    {
        LightNode(int _x, int _y, int _z, int _light)
            : x(_x), y(_y), z(_z), light(_light) {};

        int x, y, z;
        uint8_t light;
    };

    void getLights(Chunk &c, glm::ivec3 &delta, std::queue<LightNode> &queue);
    void calcLighting(ChunkMap &chunks);
    void smoothLighting(int x, int y, int z, float light[6][4]);
    void smoothLighting2(int x, int y, int z, float light[6][4]);
    void faceLighting(int x, int y, int z, float light[6][4]);
    void buildMesh();

    ChunkMap &m_chunkmap;
    Chunk &m_chunk;
    std::vector<float> m_vertices;
    ChunkData m_data;
    bool m_empty;
};