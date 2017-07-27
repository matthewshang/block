#pragma once

#include "chunk.h"
#include "perlin.h"

class TerrainGenerator
{
public:
    TerrainGenerator();

    void generate(Chunk &c);

private:
    double getHeight(double x, double z);

    Perlin m_highNoise;
    Perlin m_lowNoise;
};