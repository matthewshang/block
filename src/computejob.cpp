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
    //for (int x = 0; x < CHUNK_SIZE; x++)
    //{
    //    for (int y = 0; y < CHUNK_SIZE; y++)
    //    {
    //        for (int z = 0; z < CHUNK_SIZE; z++)
    //        {
    //            m_chunk.setLight(x, y, z, m_data.getLight(x + CHUNK_SIZE, y + CHUNK_SIZE, z + CHUNK_SIZE));
    //        }
    //    }
    //}

    m_chunk.m_vertices = std::move(m_vertices);
    m_chunk.m_dirty = false;
    m_chunk.m_glDirty = true;
    m_chunk.m_empty = m_empty;
}

void ComputeJob::smoothLighting(int x, int y, int z, float light[6][4])
{
    static const int indices[6][4] = {
        { 0, 1, 2, 3 },{ 7, 6, 5, 4 },{ 4, 5, 1, 0 },
        { 3, 2, 6, 7 },{ 1, 5, 6, 2 },{ 4, 0, 3, 7 }
    };

    static const glm::ivec3 start[8] = {
        glm::ivec3(-1, -1, 0), glm::ivec3(-1, 0, 0), glm::ivec3(0, 0, 0), glm::ivec3(0, -1, 0),
        glm::ivec3(-1, -1, -1), glm::ivec3(-1, 0, -1), glm::ivec3(0, 0, -1), glm::ivec3(0, -1, -1)
    };

    static const glm::ivec3 off[8] = {
        glm::ivec3(0, 0, 0), glm::ivec3(1, 0, 0), glm::ivec3(0, 1, 0), glm::ivec3(0, 0, 1),
        glm::ivec3(1, 1, 0), glm::ivec3(1, 0, 1), glm::ivec3(0, 1, 1), glm::ivec3(1, 1, 1)
    };

    float corners[8] = { 0.0f };

    for (int i = 0; i < 8; i++)
    {
        glm::ivec3 pos = start[i] + glm::ivec3(x, y, z);
        for (int j = 0; j < 8; j++)
        {
            const glm::ivec3 &d = off[j];
            //corners[i] += static_cast<float>(
            //    m_data.getLight(pos.x + d.x, pos.y + d.y, pos.z + d.z));
        }
        corners[i] /= 8.0f;
    }

    for (int i = 0; i < 6; i++)
    {
        for (int j = 0; j < 4; j++)
            light[i][j] = corners[indices[i][j]];
    }
}

void ComputeJob::smoothLighting2(const glm::ivec3 &worldPos, float light[6][4], float sunlight[6][4])
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
            glm::ivec3 pos = start[i][j] + worldPos;
            float blockVal = 0.0f;
            float sunVal = 0.0f;
            for (int k = 0; k < 4; k++)
            {
                const glm::ivec3 &d = off[i][k];
                blockVal += static_cast<float>(m_world.getLight(pos + d));
                sunVal += static_cast<float>(m_world.getSunlight(pos + d));
            }
            light[i][j] = blockVal / 4.0f;
            sunlight[i][j] = sunVal / 4.0f;
        }
    }
}

void ComputeJob::faceLighting(const glm::ivec3 &worldPos, float light[6][4])
{
    static const glm::ivec3 off[6] = {
        glm::ivec3(0, 0, 1), glm::ivec3(0, 0, -1), glm::ivec3(-1, 0, 0),
        glm::ivec3(1, 0, 0), glm::ivec3(0, 1, 0), glm::ivec3(0, -1, 0)
    };

    for (int i = 0; i < 6; i++)
    {
        const glm::ivec3 &d = off[i];
        for (int j = 0; j < 4; j++)
            light[i][j] = static_cast<float>(m_world.getLight(worldPos + d));
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

    for (int x = 0; x < CHUNK_SIZE; x++)
    for (int y = 0; y < CHUNK_SIZE; y++)
    for (int z = 0; z < CHUNK_SIZE; z++)
    {
        if (m_chunk.getBlock(x, y, z) == Blocks::Air)
            continue;

        bool visible[6] = { true, true, true, true, true, true };
        float light[6][4] = { 0.0f };
        float sunlight[6][4] = { 0.0f };

        glm::ivec3 local = glm::ivec3(x, y, z);
        glm::ivec3 pos = (m_chunk.getCoords() * 16) + glm::ivec3(x, y, z);

        //smoothLighting(dx, dy, dz, light);
        smoothLighting2(pos, light, sunlight);
        //faceLighting(pos, light); 

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