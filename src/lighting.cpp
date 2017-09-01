#include "lighting.h"

#include <algorithm>
#include <array>

#include "blocks.h"
#include "chunk.h"

void Lighting::pushUpdate(bool isBlock, const glm::ivec3 &min, const glm::ivec3 &max)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_ops.push_back(LightOp(isBlock, min, max));
}

void Lighting::lightNext()
{
    static const glm::ivec3 neighbors[6] = {
        glm::ivec3(0, 0, 1), glm::ivec3(0, 0, -1), glm::ivec3(1, 0, 0),
        glm::ivec3(-1, 0, 0), glm::ivec3(0, 1, 0), glm::ivec3(0, -1, 0)
    };

    if (m_ops.empty())
        return;

    LightOp op = m_ops.front();
    m_ops.pop_front();

    for (int x = op.min.x; x < op.max.x; x++)
    for (int y = op.min.y; y < op.max.y; y++)
    for (int z = op.min.z; z < op.max.z; z++)
    {
        glm::ivec3 voxel;
        glm::ivec3 pos(x, y, z);
        Chunk *chunk = m_world.getChunk(pos, &voxel);
        if (chunk == nullptr)
            continue;

        int current = op.isBlock ? chunk->getLight(voxel) : chunk->getSunlight(voxel);
        int newLight = 0;

        int blockType = chunk->getBlock(voxel);
        int opacity = std::max(Blocks::opacity(blockType), 1);

        int emission = Blocks::luminance(blockType);
        if (!op.isBlock)
        {
            int h = m_world.getHeight(x, z);
            if (y > h)
            {
                emission = 15;
            }
        }

        if (opacity < 15 || emission != 0)
        {
            int max = 0;
            for (int i = 0; i < 6; i++)
            {
                max = (std::max)(max, op.isBlock ? m_world.getLight(pos + neighbors[i]) :
                    m_world.getSunlight(pos + neighbors[i]));
            }

            newLight = (std::max)(max - opacity, emission);
            newLight = (std::max)(newLight, 0);
        }

        if (newLight != current)
        {
            if (op.isBlock)
                chunk->setLight(voxel.x, voxel.y, voxel.z, newLight);
            else
                chunk->setSunlight(voxel.x, voxel.y, voxel.z, newLight);

            dirty(*chunk, voxel);
            int val = (std::max)(newLight - 1, 0);

            propegate(x - 1, y,     z, val, op);
            propegate(x,     y - 1, z, val, op);
            propegate(x,     y,     z - 1, val, op);

            if (x + 1 >= op.max.x)
                propegate(x + 1, y, z, val, op);
            if (y + 1 >= op.max.y)
                propegate(x, y + 1, z, val, op);
            if (z + 1 >= op.max.z)
                propegate(x, y, z + 1, val, op);
        }
    }
}

bool Lighting::empty()
{
    return m_ops.empty();
}

void Lighting::propegate(int x, int y, int z, int val, const LightOp &op)
{
    glm::ivec3 voxel;
    glm::ivec3 coords(x, y, z);
    Chunk *chunk = m_world.getChunk(coords, &voxel);
    if (chunk == nullptr)
        return;

    int current = op.isBlock ? chunk->getLight(voxel): chunk->getSunlight(voxel);
    if (val == current)
        return;
    
    int blockType = chunk->getBlock(voxel);
    int emission = Blocks::luminance(blockType);

    val = (std::max)(emission, val);

    if (val != current)
    {
        pushUpdate(op.isBlock, coords, coords + glm::ivec3(1));
    }
}

void Lighting::dirty(Chunk &chunk, const glm::ivec3 &pos)
{
    chunk.setDirty(true);

    glm::ivec3 neighbors[3] = { glm::ivec3(0) };

    if (pos.x == 0)
        neighbors[0] = glm::ivec3(-1, 0, 0);
    else if (pos.x == 15)
        neighbors[0] = glm::ivec3(1, 0, 0);

    if (pos.y == 0)
        neighbors[1] = glm::ivec3(0, -1, 0);
    else if (pos.y == 15)
        neighbors[1] = glm::ivec3(0, 1, 0);

    if (pos.z == 0)
        neighbors[2] = glm::ivec3(0, 0, -1);
    else if (pos.z == 15)
        neighbors[2] = glm::ivec3(0, 0, 1);

    for (int i = 0; i < 3; i++)
    {
        if (neighbors[i] != glm::ivec3(0))
        {
            Chunk *c = m_world.getChunkFromCoords(chunk.getCoords() + neighbors[i]);
            if (c == nullptr)
                return;

            c->setDirty(true);
        }
    }
}