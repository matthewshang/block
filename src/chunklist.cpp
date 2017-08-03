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
    bool found = m_chunks.find(coords) != m_chunks.end();
    lock.unlock();
    return found;
}

int ChunkList::size()
{
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    int size = m_chunks.size();
    lock.unlock();
    return size;
}

void ChunkList::moveChunks(ChunkMap& chunks)
{
    std::unique_lock<std::shared_mutex> lock(m_mutex);

    if (m_chunks.size() > 0)
    {
        for (auto& c : m_chunks)
        {
            c.second->bufferData();
            glm::ivec3 coords = c.first;
            chunks.insert(std::make_pair(coords, std::move(c.second)));

            for (int x = -1; x < 2; x++)
            {
                for (int y = -1; y < 2; y++)
                {
                    for (int z = -1; z < 2; z++)
                    {
                        auto neighbor = chunks.find(coords + glm::ivec3(x, y, z));
                        if (neighbor != chunks.end())
                        {
                            neighbor->second->dirty();
                        }
                        else
                        {
                            neighbor = m_chunks.find(coords + glm::ivec3(x, y, z));
                            if (neighbor != m_chunks.end())
                                neighbor->second->dirty();
                        }
                    }
                }
            }
        }

        m_chunks.clear();
    }
}