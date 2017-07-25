#pragma once

#include <memory>
#include <shared_mutex>
#include <map>

#include "chunk.h"
#include "chunkcompare.h"

class ChunkList
{
public:
    ChunkList() : m_chunks() {};

    void push(std::unique_ptr<Chunk> &c);
    bool hasChunk(const glm::ivec3 &coords);
    int size();
    void moveChunks(std::map<glm::ivec3, std::unique_ptr<Chunk>, ChunkCompare> &chunks);

private:
    std::map<glm::ivec3, std::unique_ptr<Chunk>, ChunkCompare> m_chunks;
    std::shared_mutex m_mutex;
};