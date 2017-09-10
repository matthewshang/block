#include "computejob.h"

#include <iostream>

#include <glm/gtc/matrix_transform.hpp>

#include "blocks.h"
#include "geometry.h"
#include "timer.h"

ComputeJob::ComputeJob(Chunk &chunk, World &world, bool doLighting) :
    m_chunk(chunk), m_world(world), m_doLighting(doLighting), m_ld()
{
}

void ComputeJob::execute()
{
    Timer t;
    t.start();

    copyLighting(m_ld);

    if (m_doLighting)
        doLighting(m_ld);

    buildMesh(m_ld);

    t.log("Compute time: ");
}

void ComputeJob::transfer()
{
    for (int x = 0; x < CHUNK_SIZE; x++)
    for (int z = 0; z < CHUNK_SIZE; z++)
    for (int y = 0; y < CHUNK_SIZE; y++)
    {
        glm::ivec3 local = glm::ivec3(x, y, z);
        m_chunk.setLight(local.x, local.y, local.z, m_ld.getLight(local + 1));
        m_chunk.setSunlight(local.x, local.y, local.z, m_ld.getSunlight(local + 1));
    }

    m_chunk.m_vertices = std::move(m_vertices);
    m_chunk.m_dirty = false;
    m_chunk.m_glDirty = true;
    m_chunk.m_empty = m_empty;
}

void ComputeJob::copyLighting(LightData &ld)
{
    for (int x = -1; x < CHUNK_SIZE + 1; x++)
    for (int z = -1; z < CHUNK_SIZE + 1; z++)
    for (int y = -1; y < CHUNK_SIZE + 1; y++)
    {
        glm::ivec3 local = glm::ivec3(x, y, z);
        glm::ivec3 worldPos = local + m_chunk.getCoords() * 16;
        ld.setLight(local + 1, m_world.getLight(worldPos));
        ld.setSunlight(local + 1, m_world.getSunlight(worldPos));
        ld.setBlock(local + 1, m_world.getBlockType(worldPos));
    }
}

struct LightOp
{
    LightOp(bool _isBlock, const glm::ivec3 &_min, const glm::ivec3 &_max) :
        isBlock(_isBlock), min(_min), max(_max) {};
    bool isBlock;
    glm::ivec3 min;
    glm::ivec3 max;
};

void ComputeJob::doLighting(LightData &ld)
{
    std::stack<LightOp> ops;

    ops.emplace(true, glm::ivec3(1), glm::ivec3(17));
    //ops.emplace(false, glm::ivec3(1), glm::ivec3(17));

    while (!ops.empty())
    {
        lightNext(ld, ops);
    }
}

void ComputeJob::lightNext(LightData &ld, std::stack<LightOp> &ops)
{
    static const glm::ivec3 neighbors[6] = {
        glm::ivec3(0, 0, 1), glm::ivec3(0, 0, -1), glm::ivec3(1, 0, 0),
        glm::ivec3(-1, 0, 0), glm::ivec3(0, 1, 0), glm::ivec3(0, -1, 0)
    };

    LightOp op = ops.top();
    ops.pop();

    if (op._min.x < 1 || op._max.x > 17 ||
        op._min.y < 1 || op._max.y > 17 ||
        op._min.z < 1 || op._max.z > 17)
    {
        return;
    }

    for (int x = op._min.x; x < op._max.x; x++)
    for (int y = op._min.y; y < op._max.y; y++)
    for (int z = op._min.z; z < op._max.z; z++)
    {
        glm::ivec3 local(x, y, z);
        
        uint8_t current = op.isBlock ? ld.getLight(local) : ld.getSunlight(local);
        uint8_t newLight = 0;

        int type = ld.getBlock(local);
        uint8_t opacity = std::max(Blocks::opacity(type), (uint8_t)1);

        uint8_t emission = Blocks::luminance(type);

        if (opacity < 15 || emission != 0)
        {
            uint8_t max = 0;
            for (int i = 0; i < 6; i++)
            {
                max = std::max(max, op.isBlock ? ld.getLight(local + neighbors[i]) :
                    ld.getSunlight(local + neighbors[i]));
            }

            newLight = std::max((int)max - (int)opacity, (int)emission);
        }

        if (newLight != current)
        {
            if (op.isBlock)
                ld.setLight(local, newLight);
            else
                ld.setSunlight(local, newLight);

            uint8_t val = static_cast<uint8_t>(std::max((int)newLight - 1, 0));
            propegate(x - 1, y, z, val, ld, ops, op);
            propegate(x, y - 1, z, val, ld, ops, op);
            propegate(x, y, z - 1, val, ld, ops, op);

            if (x + 1 >= op._max.x)
                propegate(x + 1, y, z, val, ld, ops, op);
            if (y + 1 >= op._max.y)          ld, ops,
                propegate(x, y + 1, z, val, ld, ops, op);
            if (z + 1 >= op._max.z)          ld, ops,
                propegate(x, y, z + 1, val, ld, ops, op);
        }
    }
}

void ComputeJob::propegate(int x, int y, int z, uint8_t val, LightData &ld, std::stack<LightOp> &ops, const LightOp &op)
{
    glm::ivec3 local(x, y, z);
    uint8_t current = op.isBlock ? ld.getLight(local) : ld.getSunlight(local);
    if (val == current)
        return;

    int type = ld.getBlock(local);
    uint8_t emission = Blocks::luminance(type);

    val = std::max(emission, val);

    if (val != current)
    {
        ops.emplace(op.isBlock, local, local + 1);
    }
}

void ComputeJob::smoothLighting(const glm::ivec3 &localPos, LightData &ld, float light[6][4], float sunlight[6][4])
{
    static const glm::ivec3 start[6][4] = {
        { glm::ivec3(-1, -1, 1), glm::ivec3(-1, 0, 1), glm::ivec3(0, 0, 1), glm::ivec3(0, -1, 1) },
        { glm::ivec3(0, -1, -1), glm::ivec3(0, 0, -1), glm::ivec3(-1, 0, -1), glm::ivec3(-1, -1, -1) },
        { glm::ivec3(-1, -1, -1), glm::ivec3(-1, 0, -1), glm::ivec3(-1, 0, 0), glm::ivec3(-1, -1, 0) },
        { glm::ivec3(1, -1, 1), glm::ivec3(1, 0, 1), glm::ivec3(1, 0, 0), glm::ivec3(1, -1, 0) },
        { glm::ivec3(-1, 1, 1), glm::ivec3(-1, 1, 0), glm::ivec3(0, 1, 0), glm::ivec3(0, 1, 1) },
        { glm::ivec3(-1, -1, -1), glm::ivec3(-1, -1, 0), glm::ivec3(0, -1, 0), glm::ivec3(0, -1, -1) }
    };

    static const glm::ivec3 off[6][4] = {
        { glm::ivec3(0, 0, 0), glm::ivec3(0, 1, 0), glm::ivec3(1, 1, 0), glm::ivec3(1, 0, 0) },
        { glm::ivec3(0, 0, 0), glm::ivec3(0, 1, 0), glm::ivec3(1, 1, 0), glm::ivec3(1, 0, 0) },
        { glm::ivec3(0, 0, 0), glm::ivec3(0, 1, 0), glm::ivec3(0, 1, 1), glm::ivec3(0, 0, 1) },
        { glm::ivec3(0, 0, 0), glm::ivec3(0, 1, 0), glm::ivec3(0, 1, -1), glm::ivec3(0, 0, -1) },
        { glm::ivec3(0, 0, 0), glm::ivec3(0, 0, -1), glm::ivec3(1, 0, -1), glm::ivec3(1, 0, 0) },
        { glm::ivec3(0, 0, 0), glm::ivec3(0, 0, 1), glm::ivec3(1, 0, 1), glm::ivec3(1, 0, 0) }
    };

    for (int i = 0; i < 6; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            glm::ivec3 pos = start[i][j] + localPos + 1;
            int blockVal = 0;
            int sunVal = 0;
            for (int k = 0; k < 4; k++)
            {
                const glm::ivec3 &d = off[i][k];
                blockVal += ld.getLight(pos + d);
                sunVal += ld.getSunlight(pos + d);
            }
            light[i][j] = static_cast<float>(blockVal) / 4.0f;
            sunlight[i][j] = static_cast<float>(sunVal) / 4.0f;
        }
    }
}

void ComputeJob::faceLighting(const glm::ivec3 &localPos, LightData &ld, float light[6][4], float sunlight[6][4])
{
    static const glm::ivec3 off[6] = {
        glm::ivec3(0, 0, 1), glm::ivec3(0, 0, -1), glm::ivec3(-1, 0, 0),
        glm::ivec3(1, 0, 0), glm::ivec3(0, 1, 0), glm::ivec3(0, -1, 0)
    };

    for (int i = 0; i < 6; i++)
    {
        const glm::ivec3 &d = off[i] + 1;
        for (int j = 0; j < 4; j++)
        {
            light[i][j] = static_cast<float>(ld.getLight(localPos + d));
            sunlight[i][j] = static_cast<float>(ld.getSunlight(localPos + d));
        }
    }
}

int ComputeJob::getBlockType(const glm::ivec3 &worldPos)
{
    glm::ivec3 coords = glm::floor(static_cast<glm::vec3>(worldPos) / 16.0f);
    if (coords == m_chunk.getCoords())
    {
        glm::vec3 n = worldPos;
        return m_chunk.getBlock(glm::mod(n, glm::vec3(16)));
    }
    else
    {
        return m_world.getBlockType(worldPos);
    }
}

bool isVisible(int type)
{
    return !Blocks::isSolid(type) || type == Blocks::Leaves;
}

void ComputeJob::buildMesh(LightData &ld)
{
    int total = 0;
    m_vertices.clear();

    for (int x = 0; x < CHUNK_SIZE; x++)
    for (int z = 0; z < CHUNK_SIZE; z++)
    for (int y = 0; y < CHUNK_SIZE; y++)
    {
        if (m_chunk.getBlock(x, y, z) == Blocks::Air)
            continue;

        bool visible[6] = { true, true, true, true, true, true };
        float light[6][4] = { 0.0f };
        float sunlight[6][4] = { 0.0f };

        glm::ivec3 local = glm::ivec3(x, y, z);
        glm::ivec3 pos = (m_chunk.getCoords() * 16) + glm::ivec3(x, y, z);

        smoothLighting(local, ld, light, sunlight);
        // faceLighting(local, ld, light, sunlight); 

        int type = m_chunk.getBlock(local);

        if (Blocks::isPlant(type))
        {
            //float light = std::powf(0.8f, 15 - m_chunk.getLight(local));
            //float sunlight = std::powf(0.8f, 15 - m_chunk.getSunlight(local));
            float light = static_cast<float>(ld.getLight(local + 1)) / 16.0f;
            float sunlight = static_cast<float>(ld.getSunlight(local + 1)) / 16.0f;

            Geometry::makePlant(m_vertices, pos.x, pos.y, pos.z,
                type, light, sunlight);
        }
        else
        {
            for (int i = 0; i < 6; i++)
            {
                for (int j = 0; j < 4; j++)
                {
                    light[i][j] /= 16.0f;
                    sunlight[i][j] /= 16.0f;
                    //light[i][j] = std::powf(0.9f, 15.0f - light[i][j]);
                    //sunlight[i][j] = std::powf(0.9f, 15.0f - sunlight[i][j]);
                }
            }

            visible[0] = isVisible(getBlockType(pos + glm::ivec3(0, 0, 1)));
            visible[1] = isVisible(getBlockType(pos + glm::ivec3(0, 0, -1)));
            visible[2] = isVisible(getBlockType(pos + glm::ivec3(-1, 0, 0)));
            visible[3] = isVisible(getBlockType(pos + glm::ivec3(1, 0, 0)));
            visible[4] = isVisible(getBlockType(pos + glm::ivec3(0, 1, 0)));
            visible[5] = isVisible(getBlockType(pos + glm::ivec3(0, -1, 0)));

            Geometry::makeCube(m_vertices, pos.x, pos.y, pos.z, visible,
                type, light, sunlight);
        }

        for (int i = 0; i < 6; i++) total += visible[i] ? 1 : 0;
    }

    m_empty = total == 0;
}