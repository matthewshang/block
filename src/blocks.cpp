#include "blocks.h"

int Blocks::faces[256][6] = {
    { -1, -1, -1, -1, -1, -1 },   // Air
    { 1, 1, 1, 1, 1, 1 },         // Stone
    { 2, 2, 2, 2, 2, 2 },         // Dirt
    { 3, 3, 3, 3, 2, 0 },         // Grass
    { 17, 17, 17, 17, 17, 17 },   // Bedrock
    { 18, 18, 18, 18, 18, 18 },   // Sand
    { 20, 20, 20, 20, 21, 21 },   // Log
    { 52, 52, 52, 52, 52, 52 },   // Leaves
};

bool Blocks::isTransparent(int type)
{
    switch (type)
    {
    case Air:
    case Leaves:
        return true;
    default:
        return false;
    }
}