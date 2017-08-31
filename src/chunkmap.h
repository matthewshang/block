#pragma once

#include <map>
#include <memory>

#include <glm/glm.hpp>

#include "chunk.h"

struct Vec3Comp
{
    bool operator() (const glm::ivec3 &a, const glm::ivec3 &b) const
    {
        if (a.x != b.x)
            return a.x < b.x;
        else if (a.y != b.y)
            return a.y < b.y;
        else if (a.z != b.z)
            return a.z < b.z;
        else
            return false;
    }
};

typedef std::map<glm::ivec3, std::unique_ptr<Chunk>, Vec3Comp> ChunkMap;