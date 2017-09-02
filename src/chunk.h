#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <vector>

#include <glm/glm.hpp>

#include "mesh.h"

class World;

const int CHUNK_SIZE = 16;

class Chunk
{
public:
    friend class ComputeJob;

    Chunk(glm::ivec3 pos);

    void compute(World &world);
    void bufferData();

    Mesh &getMesh() const { return *m_mesh; };
    void setDirty(bool dirty) { m_dirty = dirty; };
    bool isDirty() { return m_dirty; };
    bool isEmpty();
    void setBlock(int x, int y, int z, uint8_t type);
    uint8_t getBlock(int x, int y, int z);
    uint8_t getBlock(const glm::ivec3 &pos);
    void setSunlight(int x, int y, int z, int val);
    int getSunlight(int x, int y, int z);
    int getSunlight(const glm::ivec3 &pos);
    void setLight(int x, int y, int z, int val);
    int getLight(int x, int y, int z);
    int getLight(const glm::ivec3 &pos);

    const glm::ivec3 &getCoords() { return m_pos; };
    const glm::vec3 &getCenter() { return m_worldCenter; };

private:
    void initBlocks();

    std::unique_ptr<Mesh> m_mesh;
    bool m_empty;
    bool m_dirty;
    bool m_glDirty;

    glm::ivec3 m_pos;
    glm::vec3 m_worldCenter;
    std::array<uint8_t, 16 * 16 * 16> m_blocks;
    std::array<uint8_t, 16 * 16 * 16> m_lightmap;
    //uint8_t m_blocks[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];
    //uint8_t m_lightmap[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];
    std::vector<float> m_vertices;
};