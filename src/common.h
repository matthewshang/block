#pragma once

#include <map>
#include <memory>

#include <glm/glm.hpp>

#include "chunkcompare.h"

class Chunk;
typedef std::map<glm::ivec3, std::unique_ptr<Chunk>, ChunkCompare> ChunkMap;