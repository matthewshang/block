#include "world.h"

#include "blocks.h"

World::World() : m_chunks(), m_heights(), m_loaded()
{

}

Chunk *World::getChunk(const glm::vec3 &pos)
{
    glm::ivec3 coords = glm::floor(static_cast<glm::vec3>(pos) / 16.0f);
    auto c = m_chunks.find(coords);
    if (c == m_chunks.end())
        return nullptr;

    return c->second.get();
}

Chunk *World::getChunk(const glm::vec3 &pos, glm::ivec3 *local)
{
    glm::ivec3 coords = glm::floor(static_cast<glm::vec3>(pos) / 16.0f);
    auto c = m_chunks.find(coords);
    if (c == m_chunks.end())
        return nullptr;

    if (local != nullptr)
    {
        glm::vec3 n = pos;
        *local = glm::mod(n, glm::vec3(16.0f));
    }

    return c->second.get();
}

int World::getBlockType(const glm::vec3 &pos)
{
    glm::ivec3 local;
    Chunk *chunk = getChunk(pos, &local);
    if (chunk != nullptr)
    {
        return chunk->getBlock(local);
    }
    
    return Blocks::Bedrock;
}

void World::setBlockType(const glm::vec3 &pos, uint8_t type)
{
    glm::ivec3 local;
    Chunk *chunk = getChunk(pos, &local);
    if (chunk != nullptr)
    {
        chunk->setBlock(local.x, local.y, local.z, type);
    }
}

int World::getLight(const glm::vec3 & pos)
{
    glm::ivec3 local;
    Chunk *chunk = getChunk(pos, &local);
    if (chunk != nullptr)
    {
        return chunk->getLight(local);
    }

    return 0;
}

int World::getSunlight(const glm::vec3 & pos)
{
    glm::ivec3 local;
    Chunk *chunk = getChunk(pos, &local);
    if (chunk != nullptr)
    {
        return chunk->getSunlight(local);
    }

    return 0;
}

void World::insert(const glm::ivec3 &coords, std::unique_ptr<Chunk> &chunk)
{
    m_chunks.insert(std::make_pair(coords, std::move(chunk)));
}

Chunk *World::getChunkFromCoords(const glm::ivec3 &coords)
{
    auto c = m_chunks.find(coords);
    if (c != m_chunks.end())
    {
        return c->second.get();
    }

    return nullptr;
}

bool World::hasChunk(const glm::ivec3 &coords)
{
    return m_loaded.find(coords) != m_loaded.end();
}

void World::loaded(const glm::ivec3 &coords)
{
    m_loaded.insert(coords);
}

void World::unloadChunk(const glm::ivec3 &coords)
{
    m_chunks.erase(coords);
    m_loaded.erase(coords);
}