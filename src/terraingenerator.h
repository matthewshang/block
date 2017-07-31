#pragma once

#include "chunk.h"
#include "perlin.h"

class TerrainGenerator
{
public:
    TerrainGenerator();

    void generate(Chunk &c);

private:
    void putTree(Chunk &c, int x, int y, int z);
    double getHeight(double x, double z);

    Perlin m_highNoise;
    Perlin m_lowNoise;
    Perlin m_trees;
};