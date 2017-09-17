#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <queue>
#include <set>
#include <stack>
#include <vector>

#include "chunk.h"
#include "world.h"

class ComputeJob
{
public:
    ComputeJob(Chunk *chunk, World &world, bool doLighting, bool init);

    void execute();

    void transfer();

    const std::set<glm::ivec3, Vec3Comp> getGlobalSpread();
    Chunk *getChunk() const { return m_chunk; };
    bool isInit() const { return m_init; };

private:
    struct LightData
    {
        LightData() : m_data{ 0 }, m_blocks{ 0 } {}

        void setLight(const glm::ivec3 &pos, uint8_t val)
        {
            int idx = pos.x * 18 * 18 + pos.z * 18 + pos.y;
            m_data[idx] = (m_data[idx] & 0xF0) | val;
        }

        uint8_t getLight(const glm::ivec3 &pos)
        {
            return m_data[pos.x * 18 * 18 + pos.z * 18 + pos.y] & 0xF;
        }

        void setSunlight(const glm::ivec3 &pos, uint8_t val)
        {
            int idx = pos.x * 18 * 18 + pos.z * 18 + pos.y;
            m_data[idx] = (m_data[idx] & 0xF) | (val << 4);
        }

        uint8_t getSunlight(const glm::ivec3 &pos)
        {
            return (m_data[pos.x * 18 * 18 + pos.z * 18 + pos.y] >> 4) & 0xF;
        }

        void setBlock(const glm::ivec3 &pos, uint8_t val)
        {
            int idx = pos.x * 18 * 18 + pos.z * 18 + pos.y;
            m_blocks[idx] = val;
        }

        uint8_t getBlock(const glm::ivec3 &pos)
        {
            return m_blocks[pos.x * 18 * 18 + pos.z * 18 + pos.y];
        }

    private:
        std::array<uint8_t, 18 * 18 * 18> m_data;
        std::array<uint8_t, 18 * 18 * 18> m_blocks;
    };

    struct LightOp
    {
        // Stupid Windows min/max macros
        LightOp(bool isBlock_, const glm::ivec3 &min_, const glm::ivec3 &max_) :
            isBlock(isBlock_), _min(min_), _max(max_) {};
        bool isBlock;
        glm::ivec3 _min;
        glm::ivec3 _max;
    };

    void copyLighting(LightData &ld);
    void doLighting(LightData &ld);
    void lightNext(LightData &ld, std::stack<LightOp> &ops);
    void propegate(int x, int y, int z, uint8_t val, LightData &ld, std::stack<LightOp> &ops, const LightOp &op);
    void globalProp(const glm::ivec3 &local);

    void smoothLighting(const glm::ivec3 &localPos, LightData &ld, float light[6][4], float sunlight[6][4]);
    void faceLighting(const glm::ivec3 &worldPos, LightData &ld, float light[6][4], float sunlight[6][4]);
    int getBlockType(const glm::ivec3 &worldPos);
    void buildMesh(LightData &ld);

    bool m_doLighting;
    bool m_init;
    LightData m_ld;
    World &m_world;
    Chunk *m_chunk;
    std::vector<float> m_vertices;
    bool m_empty;
    std::set<glm::ivec3, Vec3Comp> m_lightSpread;
};