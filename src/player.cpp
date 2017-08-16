#include "player.h"

#include <iostream>

#include <glm/gtx/string_cast.hpp>

#include "blocks.h"
#include "chunk.h"
#include "inputmanager.h"

void Player::update(float dt, ChunkMap &chunks, InputManager &input)
{
    m_flyTimer += dt;

    glm::vec3 vel;
    glm::vec3 front = -glm::cross(m_camera.getRight(), glm::vec3(0, 1, 0));
    bool switched = false;

    if (input.keyDoublePressed(GLFW_KEY_SPACE, 0.25f) && m_flyTimer > 0.4f)
    {
        m_flying = !m_flying;
        m_flyTimer = 0.0f;
        switched = true;
    }

    if (input.keyPressed(GLFW_KEY_W))
    {
        vel += front;
    }
    if (input.keyPressed(GLFW_KEY_S))
    {
        vel -= front;
    }
    if (input.keyPressed(GLFW_KEY_A))
    {
        vel -= m_camera.getRight();
    }
    if (input.keyPressed(GLFW_KEY_D))
    {
        vel += m_camera.getRight();
    }
    if (input.keyPressed(GLFW_KEY_SPACE))
    {
        if (m_flying)
        {
            vel += glm::vec3(0.0f, 1.0f, 0.0f);
        }
        else if (m_moveVel.y == 0.0f && !switched)
        {
            m_moveVel.y += 8.0f;
        }
    }
    if (input.keyPressed(GLFW_KEY_LEFT_SHIFT))
    {
        if (m_flying)
        {
            vel -= glm::vec3(0.0f, 1.0f, 0.0f);
        }
    }

    // based on github.com/fogleman/Craft
    float speed = m_flying ? 14.0f : 7.0f;
    float friction = m_flying ? 0.95f : 0.85f;
    const int steps = 8;
    float ut = dt / static_cast<float>(steps);
    if (glm::length(vel) > 0)
    {
        vel = glm::normalize(vel);
    }

    m_moveVel += vel;
    if (m_flying)
    {
        if (glm::length(m_moveVel) > speed)
        {
            m_moveVel = glm::normalize(m_moveVel) * speed;
        }
    }
    else
    {
        glm::vec2 planeVel(m_moveVel.x, m_moveVel.z);
        if (glm::length(planeVel) > speed)
        {
            planeVel = glm::normalize(planeVel) * speed;
            m_moveVel.x = planeVel.x;
            m_moveVel.z = planeVel.y;
        }
    }

    m_moveVel *= glm::vec3(friction, m_flying ? friction : 1.0f, friction);

    for (int i = 0; i < steps; i++)
    {
        if (!m_flying)
        {
            m_moveVel.y -= ut * 23.0f;
            m_moveVel.y = (std::max)(m_moveVel.y, -250.0f);
        }

        m_pos += m_moveVel * ut;

        if (collide(m_pos, chunks))
        {
            m_moveVel.y = 0.0f;
        }
    }

    m_camera.setPos(m_pos);
}

Chunk *getChunk(ChunkMap &chunks, const glm::ivec3 &coords);
int getBlock(Chunk *chunks[7], Chunk *edges[4], int x, int y, int z);


bool Player::collide(glm::vec3 &pos, ChunkMap &chunks)
{
    bool hitY = false;
    glm::ivec3 coords = glm::floor(glm::round(pos) / 16.0f);
    Chunk *c = getChunk(chunks, coords);
    if (c == nullptr)
        return hitY;

    Chunk *neighbors[7] = { nullptr };
    Chunk *edges[4] = { nullptr };

    glm::vec3 integral(16.0f);
    glm::vec3 n = glm::round(pos);
    glm::ivec3 ipos = glm::mod(n, integral);
    glm::vec3 f = pos - n;
    float pad = 0.25f;
    int h = 2;

    neighbors[0] = c;
    if (ipos.x == 0) neighbors[1] = getChunk(chunks, c->getCoords() - glm::ivec3(1, 0, 0));
    if (ipos.x == 15) neighbors[2] = getChunk(chunks, c->getCoords() + glm::ivec3(1, 0, 0));
    if (ipos.y <= h - 1) neighbors[3] = getChunk(chunks, c->getCoords() - glm::ivec3(0, 1, 0));
    if (ipos.y == 15) neighbors[4] = getChunk(chunks, c->getCoords() + glm::ivec3(0, 1, 0));
    if (ipos.z == 0) neighbors[5] = getChunk(chunks, c->getCoords() - glm::ivec3(0, 0, 1));
    if (ipos.z == 15) neighbors[6] = getChunk(chunks, c->getCoords() + glm::ivec3(0, 0, 1));

    if (ipos.y < h - 1)
    {
        edges[0] = getChunk(chunks, c->getCoords() + glm::ivec3(-1, -1, 0));
        edges[1] = getChunk(chunks, c->getCoords() + glm::ivec3(1, -1, 0));
        edges[2] = getChunk(chunks, c->getCoords() + glm::ivec3(0, -1, -1));
        edges[3] = getChunk(chunks, c->getCoords() + glm::ivec3(0, -1, 1));
    }

    for (int y = 0; y < h; y++)
    {
        if (f.x < -pad &&
            Blocks::isSolid(getBlock(neighbors, edges, ipos.x - 1, ipos.y - y, ipos.z)))
        {
            pos.x = n.x - pad;
        }
        if (f.x > pad &&
            Blocks::isSolid(getBlock(neighbors, edges, ipos.x + 1, ipos.y - y, ipos.z)))
        {
            pos.x = n.x + pad;
        }

        if (f.y < -pad &&
            Blocks::isSolid(getBlock(neighbors, edges, ipos.x, ipos.y - y - 1, ipos.z)))
        {
            pos.y = n.y - pad;
            hitY = true;
        }
        if (f.y > pad &&
            Blocks::isSolid(getBlock(neighbors, edges, ipos.x, ipos.y - y + 1, ipos.z)))
        {
            pos.y = n.y + pad;
            hitY = true;
        }

        if (f.z < -pad &&
            Blocks::isSolid(getBlock(neighbors, edges, ipos.x, ipos.y - y, ipos.z - 1)))
        {
            pos.z = n.z - pad;
        }
        if (f.z > pad &&
            Blocks::isSolid(getBlock(neighbors, edges, ipos.x, ipos.y - y, ipos.z + 1)))
        {
            pos.z = n.z + pad;
        }
    }

    return hitY;
}

static Chunk *getChunk(ChunkMap &chunks, const glm::ivec3 &coords)
{
    auto chunk = chunks.find(coords);
    if (chunk != chunks.end())
    {
        return chunk->second.get();
    }
    else
    {
        return nullptr;
    }
}

static int getBlock(Chunk *chunks[7], Chunk *edges[4], int x, int y, int z)
{
    if (y < 0)
    {
        if (x < 0)       return edges[0] ? edges[0]->getBlock(16 + x, 16 + y, z) : Blocks::Bedrock;
        else if (x > 15) return edges[1] ? edges[1]->getBlock(16 - x, 16 + y, z) : Blocks::Bedrock;
        else if (z < 0)  return edges[2] ? edges[2]->getBlock(x, 16 + y, 16 + z) : Blocks::Bedrock;
        else if (z > 15) return edges[3] ? edges[3]->getBlock(x, 16 + y, 16 - z) : Blocks::Bedrock;
        else             return chunks[3] ? chunks[3]->getBlock(x, 16 + y, z) : Blocks::Bedrock;
    }
    else if (x < 0)
    {
        return chunks[1] ? chunks[1]->getBlock(16 + x, y, z) : Blocks::Bedrock;
    }
    else if (x > 15)
    {
        return chunks[2] ? chunks[2]->getBlock(16 - x, y, z) : Blocks::Bedrock;
    }
    else if (y > 15)
    {
        return chunks[4] ? chunks[4]->getBlock(x, 16 - y, z) : Blocks::Bedrock;
    }
    else if (z < 0)
    {
        return chunks[5] ? chunks[5]->getBlock(x, y, 16 + z) : Blocks::Bedrock;
    }
    else if (z > 15)
    {
        return chunks[6] ? chunks[6]->getBlock(x, y, 16 - z) : Blocks::Bedrock;
    }

    return chunks[0]->getBlock(x, y, z);
}