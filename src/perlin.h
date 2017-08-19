#pragma once

#include <cstdint>
#include <vector>

class Perlin
{
public:
    Perlin();
    Perlin(int octaves, double frequency, double amplitude, double lacunarity, double persistence);

    double perlin3(double x, double y, double z);
    double noise3(double x, double y, double z);

private:
    std::vector<std::uint8_t> m_p;

    int m_octaves;
    double m_frequency;
    double m_amplitude;
    double m_lacunarity;
    double m_persistence;
};