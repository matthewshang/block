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
    struct LightData
    {
        void setLight(int x, int y, int z, uint8_t val)
        {
            int idx = x * 18 * 18 + z * 18 + y;
            m_data[idx] = (m_data[idx] & 0xF0) | val;
        }

        uint8_t getLight(const glm::ivec3 &pos)
        {
            return m_data[pos.x * 18 * 18 + pos.z * 18 + pos.y] & 0xF;
        }

        void setSunlight(int x, int y, int z, uint8_t val)
        {
            int idx = x * 18 * 18 + z * 18 + y;
            m_data[idx] = (m_data[idx] & 0xF) | (val << 4);
        }

        int getSunlight(const glm::ivec3 &pos)
        {
            return (m_data[pos.x * 18 * 18 + pos.z * 18 + pos.y] >> 4) & 0xF;
        }
    private:
        std::array<uint8_t, 18 * 18 * 18> m_data;
    };

    void smoothLighting(const glm::ivec3 &localPos, LightData &ld, float light[6][4], float sunlight[6][4]);
    void faceLighting(const glm::ivec3 &worldPos, LightData &ld, float light[6][4], float sunlight[6][4]);
    int getBlockType(const glm::ivec3 &worldPos);
    void buildMesh();

    World &m_world;
    Chunk &m_chunk;
    std::vector<float> m_vertices;
    bool m_empty;
};