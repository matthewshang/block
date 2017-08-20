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
        uint8_t typeMap[48][48][48];

        void setLight(int x, int y, int z, int val)
        {
            lightMap[x][y][z] = (lightMap[x][y][z] & 0xF0) | val;
        }

        int getLight(int x, int y, int z)
        {
            return (lightMap[x][y][z]) & 0xF;
        }

        void setSunlight(int x, int y, int z, int val)
        {
            lightMap[x][y][z] = (lightMap[x][y][z] & 0xF) | (val << 4);
        }

        int getSunlight(int x, int y, int z)
        {
            return (lightMap[x][y][z] >> 4) & 0xF;
        }
    };

    struct LightNode
    {
        LightNode(int _x, int _y, int _z, int _light)
            : x(_x), y(_y), z(_z), light(_light) {};

        int x, y, z;
        uint8_t light;
    };

    void getLights(Chunk &c, const glm::ivec3 &delta, std::queue<LightNode> &queue);
    void calcLighting();
    void calcSunlight();
    void smoothLighting(int x, int y, int z, float light[6][4]);
    void smoothLighting2(int x, int y, int z, float light[6][4], float sunlight[6][4]);
    void faceLighting(int x, int y, int z, float light[6][4]);
    void buildMesh();

    ChunkMap &m_chunkmap;
    Chunk &m_chunk;
    std::vector<float> m_vertices;
    ChunkData m_data;
    bool m_empty;
};