#pragma once

class ChunkCoord
{
public:
    int x;
    int y;
    int z;

    ChunkCoord() = default;
    ChunkCoord(int _x, int _y, int _z) : x(_x), y(_y), z(_z) {};

    int getXWorld() { return x * 16; };
    int getYWorld() { return y * 16; };
    int getZWorld() { return z * 16; };

    bool operator <(const ChunkCoord &c) const
    {
        if (x != c.x)
            return x < c.x;
        else if (y != c.y)
            return y < c.y;
        else if (z != c.z)
            return z < c.z;
        else
            return false;
    }
};