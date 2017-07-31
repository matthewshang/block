#include "terraingenerator.h"

#include <algorithm>

#include <glm/glm.hpp>

#include "blocks.h"

TerrainGenerator::TerrainGenerator() :
    m_highNoise(2, 0.023, 14, 2, 0.5), m_lowNoise(2, 0.017, 5, 2, 0.9),
    m_trees(2, 1.71, 1, 2, 0.7)
{
    
}

bool canPutTree(int x, int y, int z);

void TerrainGenerator::generate(Chunk &c)
{
    const glm::ivec3 &coords = c.getCoords();

    if (coords.y < 0 || coords.y > 16) 
        return;

    for (int x = 0; x < CHUNK_SIZE; x++)
    {
        for (int z = 0; z < CHUNK_SIZE; z++)
        {
            int rx = coords.x * 16 + x;
            int ry = coords.y * 16;
            int rz = coords.z * 16 + z;

            int h = static_cast<int>(std::floor(getHeight(rx, rz)));
            int dh = h - ry;
            if (dh < 0)
                continue;

            dh = (std::min)(16, dh);
            for (int y = 0; y < dh; y++)
            {
                if (ry == 0)
                {
                    c.setBlock(x, y, z, Blocks::Bedrock);
                }
                else if (ry == h - 1)
                {
                    c.setBlock(x, y, z, Blocks::Grass);
                }
                else
                {
                    if (ry < h && ry > h - 2)
                    {
                        c.setBlock(x, y, z, Blocks::Dirt);
                    }
                    else
                    {
                        c.setBlock(x, y, z, Blocks::Stone);
                    }
                }
                ry++;
            }

            if (canPutTree(x, dh, z) && m_trees.perlin3(rx, ry, rz) > 0.8)
            {
                putTree(c, x, dh, z);
            }
        }
    }
}

static bool canPutTree(int x, int y, int z)
{
    return y < 9 && x > 2 && x < 13 && z > 2 && z < 13;
}

void TerrainGenerator::putTree(Chunk &c, int x, int y, int z)
{
    for (int ty = 0; ty < 5; ty++)
    {
        for (int tx = -3; tx < 3; tx++)
        {
            for (int tz = -3; tz < 3; tz++)
            {
                int d = tx * tx + (ty - 1) * (ty - 1) + tz * tz;
                if (d < 9)
                {
                    c.setBlock(x + tx, y + ty + 3, z + tz, Blocks::Leaves);
                }
            }
        }
    }

    for (int ty = 0; ty < 5; ty++)
    {
        c.setBlock(x, y + ty, z, Blocks::Log);
    }
}

double TerrainGenerator::getHeight(double x, double z)
{
    double val = m_lowNoise.perlin3(x, z, 0.31415) + m_highNoise.perlin3(x, z, 0.271828) + 50;
    if (val < 0)
        val = 50;
    if (val > 256)
        val = 255;
    return val;
}