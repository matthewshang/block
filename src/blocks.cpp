#include "blocks.h"

int Blocks::faces[256][6] = {
    { -1, -1, -1, -1, -1, -1 },       // Air
    { 1, 1, 1, 1, 1, 1 },             // Stone
    { 2, 2, 2, 2, 2, 2 },             // Dirt
    { 3, 3, 3, 3, 0, 2 },             // Grass
    { 17, 17, 17, 17, 17, 17 },       // Bedrock
    { 18, 18, 18, 18, 18, 18 },       // Sand
    { 20, 20, 20, 20, 21, 21 },       // Log
    { 52, 52, 52, 52, 52, 52 },       // Leaves
    { 12, 12, 12, 12, 12, 12 },       // Red Flower
    { 13, 13, 13, 13, 13, 13 },       // Yellow Flower
    { 39, 39, 39, 39, 39, 39 },       // Grass Plant
    { 105, 105, 105, 105, 105, 105 }, // Glowstone
    { 8, 8, 8, 8, 9, 10 },            // TNT
};

bool Blocks::isTransparent(int type)
{
    if (isPlant(type))
        return true;

    switch (type)
    {
    case Air:
    case Leaves:
        return true;
    default:
        return false;
    }
}

bool Blocks::isPlant(int type)
{
    switch (type)
    {
    case RedFlower:
    case YellowFlower:
    case GrassPlant:
        return true;
    default:
        return false;
    }
}

bool Blocks::isLight(int type)
{
    switch (type)
    {
    case Glowstone:
        return true;
    default:
        return false;
    }
}

bool Blocks::isSolid(int type)
{
    if (isPlant(type) || type == Air)
        return false;

    return true;
}

int Blocks::luminance(int type)
{
    switch (type)
    {
    case Glowstone:
        return 15;
    default:
        return 0;
    }
}

int Blocks::opacity(int type)
{
    switch (type)
    {
    case Air:
    case RedFlower:
    case YellowFlower:
    case GrassPlant:
    case Leaves:
    case Glowstone:
        return 1;
    default:
        return 255;
    }
}
