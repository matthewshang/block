#pragma once

#include <cstdint>

namespace Blocks
{
    enum
    {
        Air,
        Stone,
        Dirt,
        Grass,
        Bedrock,
        Sand,
        Log,
        Leaves,
        RedFlower,
        YellowFlower,
        GrassPlant,
        Glowstone,
        TNT
    };

    extern int faces[256][6];

    bool isTransparent(int type);
    bool isPlant(int type);
    bool isLight(int type);
    bool isSolid(int type);

    uint8_t luminance(int type);
    uint8_t opacity(int type);
}