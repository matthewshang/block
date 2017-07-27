#include "terraingenerator.h"

#include <algorithm>

#include <glm/glm.hpp>

#include "blocks.h"

TerrainGenerator::TerrainGenerator() :
    m_highNoise(2, 0.023, 14, 2, 1), m_lowNoise(2, 0.023, 5, 2, 1)
{
    
}

void TerrainGenerator::generate(Chunk &c)
{
    const glm::ivec3 &coords = c.getCoords();

    for (int x = 0; x < CHUNK_SIZE; x++)
    {
        for (int y = 0; y < CHUNK_SIZE; y++)
        {
            for (int z = 0; z < CHUNK_SIZE; z++)
            {
                int rx = coords.x * 16 + x;
                int ry = coords.y * 16 + y;
                int rz = coords.z * 16 + z;

                double density = m_noise.noise3(
                    static_cast<double>(rx) * 0.07,
                    static_cast<double>(ry) * 0.07,
                    static_cast<double>(rz) * 0.07);

                double depth_falloff = 1.0 - (std::max)(0.0, 
                    (std::min)((static_cast<double>(ry) + 64.0) / 128.0, 1.0));

                double n = density * depth_falloff;

                if (n < 0.3 && n > 0.2)
                {
                    c.setBlock(x, y, z, Blocks::Grass);
                }
                else if (n  > 0.3)
                {
                    c.setBlock(x, y, z, Blocks::Stone);
                }
            }
        }
    }
}