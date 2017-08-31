#pragma once

#include <array>
#include <map>
#include <memory>
#include <set>

#include <glm/glm.hpp>

#include "chunk.h"
#include "common.h"
class World
{
public:
    //typedef std::map<glm::ivec3, std::unique_ptr<Chunk>, ChunkCompare> ChunkMap;
    
    World();

    void insert(const glm::ivec3 &coords, std::unique_ptr<Chunk> &chunk);
    Chunk *getChunkFromCoords(const glm::ivec3 &coords);
    bool hasChunk(const glm::ivec3 &coords);
    void loaded(const glm::ivec3 &coords);
    void unloadChunk(const glm::ivec3 &coords);

    Chunk *getChunk(const glm::vec3 &pos);
    Chunk *getChunk(const glm::vec3 &pos, glm::ivec3 *local);
    
    int getBlockType(const glm::vec3 &pos);
    void setBlockType(const glm::vec3 &pos, uint8_t type);
    
    int getLight(const glm::vec3 &pos);
    int getSunlight(const glm::vec3 &pos);

    ChunkMap &getMap() { return m_chunks; };

private:
    struct Vec3Comp
    {
        bool operator() (const glm::ivec3 &a, const glm::ivec3 &b) const
        {
            if (a.x != b.x)
                return a.x < b.x;
            else if (a.y != b.y)
                return a.y < b.y;
            else if (a.z != b.z)
                return a.z < b.z;
            else
                return false;
        }
    };

    struct Vec2Comp
    {
        bool operator() (const glm::ivec2 &a, const glm::ivec2 &b) const
        {
            if (a.x != b.x)
                return a.x < b.x;
            else if (a.y != b.y)
                return a.y < b.y;
            else
                return false;
        }
    };

    struct HeightMap
    {
        HeightMap() : m_data{ 0 } {};
        void set(int x, int z, int height)
        {
            m_data[x * 16 + z] = height;
        }

        uint16_t get(int x, int z)
        {
            return m_data[x * 16 + z];
        }

    private:
        std::array<uint16_t, 16 * 16> m_data;
    };

    ChunkMap m_chunks;
    std::set<glm::ivec3, Vec3Comp> m_loaded;
    std::map<glm::ivec2, std::unique_ptr<HeightMap>, Vec2Comp> m_heights;
};