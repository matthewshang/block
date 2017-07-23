#pragma once

#include <cstdint>

class Perlin
{
public:
    static void initPermutation();

    static double perlin3(double x, double y, double z, int octaves, double persistence);

private:
    static double noise3(double x, double y, double z);
    static double improved(double x, double y, double z);

    static int inc(int n);

    static const uint8_t permutation[256];
    static uint8_t p[512];
    static const double repeat;
};