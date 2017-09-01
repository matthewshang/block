#ifndef NOMINMAX
#define NOMINMAX
#endif

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
    HeightMap *map = getHeightMap(coords.x * 16, coords.z * 16, nullptr, true);
    if (!chunk->isEmpty())
    {
        map->chunkHeight = std::max(map->chunkHeight, coords.y);
        updateHeightMap(glm::ivec2(coords.x, coords.z), *map, *chunk);
    }

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

HeightMap *World::getHeightMap(float x, float z, glm::ivec2 *local, bool generate)
{
    glm::vec2 n(x, z);
    if (local != nullptr)
        *local = glm::mod(glm::floor(n), glm::vec2(16.0f));
    glm::ivec2 coords = glm::floor(n / 16.0f);
    auto hm = m_heights.find(coords);
    if (hm == m_heights.end())
    {
        auto entry = m_heights.insert(std::make_pair(coords, std::make_unique<HeightMap>()));
        HeightMap &map = *entry.first->second;
        return &map;
    }

    return hm->second.get();
}

void World::updateHeightMap(const glm::ivec2 &coords, HeightMap &map, Chunk &chunk)
{
    for (int x = 0; x < 16; x++)
    for (int z = 0; z < 16; z++)
    {
        int current = map.get(x, z);
        int newHeight = 15;
        if (newHeight + chunk.getCoords().y * 16 < current)
            continue;

        while (newHeight >= 0)
        {
            if (chunk.getBlock(x, newHeight, z) != Blocks::Air)
                break;
            newHeight--;
        }

        if (newHeight == 0 && chunk.getBlock(x, 0, z) == Blocks::Air)
            continue;
        
        newHeight += chunk.getCoords().y * 16;
        if (newHeight > current)
            map.set(x, z, newHeight);
    }
}

int World::getHeight(float x, float z)
{
    glm::ivec2 local;
    HeightMap *map = getHeightMap(x, z, &local, true);

    return map->get(local.x, local.y);
}