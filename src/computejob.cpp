#include "computejob.h"

#include <iostream>

#include <glm/gtc/matrix_transform.hpp>

#include "blocks.h"
#include "geometry.h"
#include "timer.h"

ComputeJob::ComputeJob(Chunk &chunk, World &world) :
    m_chunk(chunk), m_world(world)
{
}

void ComputeJob::execute()
{
    buildMesh();
}

void ComputeJob::transfer()
{
    m_chunk.m_vertices = std::move(m_vertices);
    m_chunk.m_dirty = false;
    m_chunk.m_glDirty = true;
    m_chunk.m_empty = m_empty;
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
            int sunVal = 0.0f;
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

void ComputeJob::buildMesh()
{
    int total = 0;
    m_vertices.clear();

    LightData ld;

    for (int x = -1; x < CHUNK_SIZE + 1; x++)
    for (int z = -1; z < CHUNK_SIZE + 1; z++)
    for (int y = -1; y < CHUNK_SIZE + 1; y++)
    {
        glm::ivec3 worldPos = glm::ivec3(x, y, z) + m_chunk.getCoords() * 16;
        ld.setLight(x + 1, y + 1, z + 1, m_world.getLight(worldPos));
        ld.setSunlight(x + 1, y + 1, z + 1, m_world.getSunlight(worldPos));
    }

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
            float light = static_cast<float>(m_chunk.getLight(local)) / 16.0f;
            float sunlight = static_cast<float>(m_chunk.getSunlight(local)) / 16.0f;

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