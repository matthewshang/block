#pragma once

#include <cstdint>
#include <vector>

class Perlin
{
public:
    Perlin::Perlin();

    double perlin3(double x, double y, double z, int octaves, double persistence);
    double noise3(double x, double y, double z);

private:
    std::vector<std::uint8_t> m_p;
};