#include "blocks.h"

int Blocks::faces[256][6] = {
    { -1, -1, -1, -1, -1, -1 },   // Air
    { 1, 1, 1, 1, 1, 1 },         // Stone
    { 2, 2, 2, 2, 2, 2 },         // Dirt
    { 3, 3, 3, 3, 2, 0 }          // Grass
};