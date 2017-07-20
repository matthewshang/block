#pragma once


class Perlin
{
public:
    static void initPermutation();

    static double perlin3(double x, double y, double z, int octaves, double persistence);

private:
    static double noise3(double x, double y, double z);

    static int inc(int n);

    static const int permutation[256];
    static int p[512];
    static const double repeat;
};