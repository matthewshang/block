#pragma once

#include "chunk.h"
#include "perlin.h"

class TerrainGenerator
{
public:
    TerrainGenerator();

    void generate(Chunk &c);

private:

    Perlin m_highNoise;
    Perlin m_lowNoise;
};