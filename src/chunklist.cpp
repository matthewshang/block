#include "chunklist.h"

#include <iostream>
#include <mutex>

void ChunkList::push(std::unique_ptr<Chunk> &c)
{
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    m_chunks.insert(std::make_pair(c->getCoords(), std::move(c)));
}

bool ChunkList::hasChunk(const glm::ivec3 &coords)
{
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    return m_chunks.find(coords) != m_chunks.end();
}

void ChunkList::moveChunks(std::map<glm::ivec3, std::unique_ptr<Chunk>, ChunkCompare>& chunks)
{
    std::unique_lock<std::shared_mutex> lock(m_mutex);

    if (m_chunks.size() > 0)
    {
        for (auto& c : m_chunks)
        {
            chunks.insert(std::make_pair(c.first, std::move(c.second)));
        }

        //std::cout << m_chunks.size() << std::endl;
        m_chunks.clear();
    }
}