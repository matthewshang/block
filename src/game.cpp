#include "game.h"

#include <algorithm>
#include <chrono>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/integer.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include "blocks.h"
#include "chunk.h"

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    InputManager *input = reinterpret_cast<InputManager *>(glfwGetWindowUserPointer(window));
    input->handleKey(key, action);
}

Game::Game(GLFWwindow *window) : m_window(window), m_camera(glm::vec3(-88, 55, -28)),
    m_lastX(960), m_lastY(540), m_firstMouse(true), m_chunkGenerator(), m_processed(),
    m_renderer(m_chunks), m_pos(-88, 54, -28), m_vel(0), m_flying(false)
{
    m_eraseDistance = sqrtf(3 * (pow(16 * m_loadDistance, 2))) + 32.0f;
    glfwSetWindowUserPointer(window, &m_input);
    glfwSetKeyCallback(window, key_callback);

    m_frustum.setInternals(45.0f, 1920.0f / 1080.0f, 0.1f, m_eraseDistance);
}

void Game::run()
{
    glm::vec3 skyColor(135.0f, 206.0f, 250.0f);
    skyColor /= 255.0f;

    float dt = 0.0f;
    double lastFrame = 0.0f;
    long nFrames = 0;
    double lastTime = 0.0f;

    while (!glfwWindowShouldClose(m_window))
    {
        glfwPollEvents();

        processInput(dt);
        updatePlayer(dt);

        m_frustum.setCam(m_camera.getPos(), m_camera.getPos() + m_camera.getFront(), glm::vec3(0, 1, 0));

        updateChunks();

        //float daylight = (1.0f + sinf(glfwGetTime() * 0.5f)) * 0.5f * 0.7f;
        float daylight = 0.25f;
        m_renderer.setSkyColor(skyColor * daylight);
        m_renderer.setDaylight(daylight);
        m_renderer.render(m_camera, m_frustum);

        glfwSwapBuffers(m_window);

        double current = glfwGetTime();
        dt = static_cast<float>(current - lastFrame);
        lastFrame = current;

        nFrames++;
        if (current - lastTime > 1.0f)
        {
            glm::vec3 toChunk = m_camera.getPos() / 16.0f;
            int x = static_cast<int>(std::floorf(toChunk.x));
            int y = static_cast<int>(std::floorf(toChunk.y));
            int z = static_cast<int>(std::floorf(toChunk.z));
            char title[256];
            title[255] = '\0';
            snprintf(title, 255, "block - [FPS: %ld] [%zd chunks] [%d jobs queued] [pos: %f, %f, %f] [chunk: %d %d %d]", nFrames, m_chunks.size(), m_pool.getJobsAmount(),
                m_camera.getPos().x, m_camera.getPos().y, m_camera.getPos().z, x, y, z);
            glfwSetWindowTitle(m_window, title);
            lastTime += 1.0f;
            nFrames = 0;
        }
    }
}

static Chunk *getChunk(ChunkMap &chunks, glm::ivec3 coords)
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

int Game::getVoxel(const glm::ivec3 &i)
{
    glm::ivec3 coords = glm::floor(static_cast<glm::vec3>(i) / 16.0f);
    Chunk *c = getChunk(m_chunks, coords);
    if (c == nullptr)
        return Blocks::Air;
   
    glm::vec3 integral(16.0f);
    glm::vec3 n = i;
    glm::ivec3 ipos = glm::mod(n, integral);
    return c->getBlock(ipos.x, ipos.y, ipos.z);
}

int Game::traceRay(glm::vec3 p, glm::vec3 dir, float range, glm::ivec3 &hitNorm, glm::ivec3 &hitIpos)
{
    // github.com/andyhall/fast-voxel-raycast/blob/master/index.js
    float inf = std::numeric_limits<float>::infinity();
    float t = 0.0f;
    glm::ivec3 i = glm::floor(p);
    glm::ivec3 step(dir.x > 0 ? 1 : -1, 
                    dir.y > 0 ? 1 : -1, 
                    dir.z > 0 ? 1 : -1);
    glm::vec3 delta((dir.x == 0.0f) ? inf : std::fabsf(1.0f / dir.x),
                    (dir.y == 0.0f) ? inf : std::fabsf(1.0f / dir.y),
                    (dir.z == 0.0f) ? inf : std::fabsf(1.0f / dir.z));
    glm::vec3 dist((step.x > 0) ? (i.x + 1.0f - p.x) : (p.x - i.x),
                   (step.y > 0) ? (i.y + 1.0f - p.y) : (p.y - i.y),
                   (step.z > 0) ? (i.z + 1.0f - p.z) : (p.z - i.z));
    glm::vec3 max((delta.x < inf) ? delta.x * dist.x : inf,
                  (delta.y < inf) ? delta.y * dist.y : inf,
                  (delta.z < inf) ? delta.z * dist.z : inf);
    int idx = -1;

    while (t <= range)
    {
        int b = getVoxel(i);
        if (b != Blocks::Air)
        {
            hitIpos = i;
            hitNorm = glm::ivec3(0);
            if (idx == 0) hitNorm.x = -step.x;
            if (idx == 1) hitNorm.y = -step.y;
            if (idx == 2) hitNorm.z = -step.z;
            return b;
        }

        if (max.x < max.y)
        {
            if (max.x < max.z)
            {
                i.x += step.x;
                t = max.x;
                max.x += delta.x;
                idx = 0;
            }
            else
            {
                i.z += step.z;
                t = max.z;
                max.z += delta.z;
                idx = 2;
            }
        }
        else
        {
            if (max.y < max.z)
            {
                i.y += step.y;
                t = max.y;
                max.y += delta.y;
                idx = 1;
            }
            else
            {
                i.z += step.z;
                t = max.z;
                max.z += delta.z;
                idx = 2;
            }
        }
    }

    hitNorm = glm::ivec3(0);
    hitIpos = i;

    return Blocks::Air;
}

void Game::raycast(glm::vec3 p, glm::vec3 dir, float range, int block)
{
    float ds = glm::length(dir);
    if (ds == 0.0f)
        return;

    dir /= ds;
    glm::ivec3 norm, ipos;
    int type = traceRay(p, dir, range, norm, ipos);
    //std::cout << "raycast: " << (type != Blocks::Air) << std::endl;
    //if (type) std::cout << "raycast pos: " << glm::to_string(pos) << std::endl;
    //if (type) std::cout << "raycast ipos: " << glm::to_string(ipos) << std::endl;
    //if (type) std::cout << "raycast norm: " << glm::to_string(norm) << std::endl;

    if (type != Blocks::Air)
    {
        glm::vec3 rpos = block == Blocks::Air ? ipos : ipos + norm;
        glm::ivec3 coords = glm::floor(rpos / 16.0f);
        Chunk *c = getChunk(m_chunks, coords);
        if (c == nullptr)
            return;

        glm::vec3 integral(16.0f);
        glm::ivec3 ipos2 = glm::mod(rpos, integral);
        c->setBlock(ipos2.x, ipos2.y, ipos2.z, block);
        dirtyChunks(coords);
    }
}

void Game::processInput(float dt)
{
    m_input.update(dt);
    m_cooldown += dt;
    m_flyCooldown += dt;

    if (m_input.keyPressed(GLFW_KEY_ESCAPE))
    {
        glfwSetWindowShouldClose(m_window, true);
    }
    if (m_input.keyPressed(GLFW_KEY_R))
    {
        m_pos = glm::vec3(-88, 54, -28);
    }

    double xpos, ypos;
    glfwGetCursorPos(m_window, &xpos, &ypos);
    if (m_firstMouse)
    {
        m_lastX = xpos;
        m_lastY = ypos;
        m_firstMouse = false;
    }
    float dx = static_cast<float>(xpos - m_lastX);
    float dy = static_cast<float>(m_lastY - ypos);
    m_lastX = xpos;
    m_lastY = ypos;

    m_camera.processMouse(dx, dy);

    if (glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS ||
        glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
    {
        if (m_cooldown > 0.2f)
        {
            bool left = glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;

            raycast(m_pos, m_camera.getFront(), 12, left ? Blocks::Air : Blocks::Glowstone);
            m_cooldown = 0.0f;
        }
    }
}

void Game::loadNearest(const glm::ivec3 &center, int maxJobs)
{
    for (int i = 0; i < maxJobs; i++)
    {
        bool found = false;
        int bestScore = std::numeric_limits<int>::max();
        glm::ivec3 bestCoords;

        for (int x = -m_loadDistance; x <= m_loadDistance; x++)
        {
            for (int y = -m_loadDistance; y <= m_loadDistance; y++)
            {
                for (int z = -m_loadDistance; z <= m_loadDistance; z++)
                {
                    glm::ivec3 coords = center + glm::ivec3(x, y, z);
                    if (m_loadedChunks.find(coords) != m_loadedChunks.end())
                        continue;

                    int visible = !m_frustum.boxInFrustum(static_cast<glm::vec3>(coords * 16), glm::vec3(16));
                    int distance = abs(x) + abs(y) + abs(z);
                    int score = visible << 8 | distance;
                    if (score < bestScore)
                    {
                        bestScore = score;
                        bestCoords = coords;
                        found = true;
                    }
                }
            }
        }

        if (found)
        {
            Chunk *c = new Chunk(bestCoords);
            m_loadedChunks.insert(bestCoords);
            auto lambda = [c, this]() -> void
            {
                m_chunkGenerator.generate(*c);
                c->compute(m_chunks);
                std::unique_ptr<Chunk> ptr(c);
                m_processed.push_back(ptr);
            };
            m_pool.addJob(lambda);
        }
        else
        {
            break;
        }
    }
}

void Game::updateNearest(const glm::ivec3 &center, int maxJobs)
{
    for (int i = 0; i < maxJobs; i++)
    {
        bool found = false;
        int bestScore = std::numeric_limits<int>::max();
        Chunk *bestChunk = nullptr;

        for (const auto& it : m_chunks)
        {
            auto& chunk = it.second;
            if (!chunk->isDirty())
                continue;

            const glm::ivec3 &coords = chunk->getCoords();
            int visible = !m_frustum.boxInFrustum(static_cast<glm::vec3>(coords * 16), glm::vec3(16));
            int distance = abs(coords.x - center.x) + abs(coords.y - center.y) + abs(coords.z - center.z);
            int score = visible << 8 | distance;
            if (score < bestScore)
            {
                bestScore = score;
                bestChunk = chunk.get();
                found = true;
            }
        }

        if (found)
        {
            bestChunk->setDirty(false);
            auto update = [this, bestChunk]() -> void
            {
                auto compute = std::make_unique<ComputeJob>(*bestChunk, m_chunks);
                compute->execute();
                m_updates.push_back(compute);
            };
            m_pool.addJob(update);
        }
        else
        {
            break;
        }
    }
}

void Game::updateChunks()
{
    glm::ivec3 current = static_cast<glm::vec3>(glm::floor(m_camera.getPos() / 16.0f));
    int maxJobs = std::max(m_pool.getWorkerAmount() - m_pool.getJobsAmount(), 1);

    loadNearest(current, maxJobs);

    for (const auto &it : m_chunks)
    {
        auto &chunk = it.second;
        if (glm::distance(chunk->getCenter(), m_camera.getPos()) > m_eraseDistance)
        {
            m_toErase.push_back(chunk->getCoords());
        }
    }

    for (const auto &chunk : m_toErase)
    {
        m_chunks.erase(chunk);
        m_loadedChunks.erase(chunk);
    }
    m_toErase.clear();

    updateNearest(current, maxJobs);

    auto move = [this](std::unique_ptr<Chunk> &c) -> void
    {
        glm::ivec3 coords = c->getCoords();
        m_chunks.insert(std::make_pair(coords, std::move(c)));

        for (int x = -1; x < 2; x++)
        {
            for (int y = -1; y < 2; y++)
            {
                for (int z = -1; z < 2; z++)
                {
                    auto neighbor = m_chunks.find(coords + glm::ivec3(x, y, z));
                    if (neighbor != m_chunks.end())
                    {
                        neighbor->second->setDirty(true);
                    }
                }
            }
        }
    };

    m_processed.for_each(move);
    m_processed.clear();

    auto update = [this](std::unique_ptr<ComputeJob> &job) -> void
    {
        job->transfer();
    };

    m_updates.for_each(update);
    m_updates.clear();
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

bool Game::collide(glm::vec3 &pos)
{
    bool hitY = false;
    glm::ivec3 coords = glm::floor(glm::round(pos) / 16.0f);
    Chunk *c = getChunk(m_chunks, coords);
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
    if (ipos.x == 0) neighbors[1] = getChunk(m_chunks, c->getCoords() - glm::ivec3(1, 0, 0));
    if (ipos.x == 15) neighbors[2] = getChunk(m_chunks, c->getCoords() + glm::ivec3(1, 0, 0));
    if (ipos.y <= h - 1) neighbors[3] = getChunk(m_chunks, c->getCoords() - glm::ivec3(0, 1, 0));
    if (ipos.y == 15) neighbors[4] = getChunk(m_chunks, c->getCoords() + glm::ivec3(0, 1, 0));
    if (ipos.z == 0) neighbors[5] = getChunk(m_chunks, c->getCoords() - glm::ivec3(0, 0, 1));
    if (ipos.z == 15) neighbors[6] = getChunk(m_chunks, c->getCoords() + glm::ivec3(0, 0, 1));
    
    if (ipos.y < h - 1)
    {
        edges[0] = getChunk(m_chunks, c->getCoords() + glm::ivec3(-1, -1, 0));
        edges[1] = getChunk(m_chunks, c->getCoords() + glm::ivec3(1, -1, 0));
        edges[2] = getChunk(m_chunks, c->getCoords() + glm::ivec3(0, -1, -1));
        edges[3] = getChunk(m_chunks, c->getCoords() + glm::ivec3(0, -1, 1));
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


void Game::updatePlayer(float dt)
{
    glm::vec3 vel;
    glm::vec3 front = -glm::cross(m_camera.getRight(), glm::vec3(0, 1, 0));
    bool switched = false;

    if (m_input.keyDoublePressed(GLFW_KEY_SPACE, 0.25f) && m_flyCooldown > 0.4f)
    {
        m_flying = !m_flying;
        m_flyCooldown = 0.0f;
        switched = true;
    }

    if (m_input.keyPressed(GLFW_KEY_W))
    {
        vel += front;
    }
    if (m_input.keyPressed(GLFW_KEY_S))
    {
        vel -= front;
    }
    if (m_input.keyPressed(GLFW_KEY_A))
    {
        vel -= m_camera.getRight();
    }
    if (m_input.keyPressed(GLFW_KEY_D))
    {
        vel += m_camera.getRight();
    }
    if (m_input.keyPressed(GLFW_KEY_SPACE))
    {
        if (m_flying)
        {
            vel += glm::vec3(0.0f, 1.0f, 0.0f);
        }
        else if (m_vel.y == 0.0f && !switched)
        {
            m_vel.y += 8.0f;
        }
    }
    if (m_input.keyPressed(GLFW_KEY_LEFT_SHIFT))
    {
        if (m_flying)
        {
            vel -= glm::vec3(0.0f, 1.0f, 0.0f);
        }
    }

    if (glm::length(vel) > 0)
    {
        vel = glm::normalize(vel);
    }

    // based on github.com/fogleman/Craft
    float speed = m_flying ? 15.0f : 5.0f;
    int steps = 8;
    float ut = dt / static_cast<float>(steps);
    vel *= ut * speed;
    for (int i = 0; i < steps; i++)
    {
        if (m_flying)
        {
            m_vel.y = 0.0f;
        }
        else
        {
            m_vel.y -= ut * 20.0f;
            m_vel.y = (std::max)(m_vel.y, -250.0f);
        }

        m_pos += vel + m_vel * ut;

        if (collide(m_pos))
            m_vel.y = 0.0f;
    }

    m_camera.setPos(m_pos);
}

void Game::dirtyChunks(glm::ivec3 center)
{
    for (int x = -1; x < 2; x++)
    {
        for (int y = -1; y < 2; y++)
        {
            for (int z = -1; z < 2; z++)
            {
                auto neighbor = m_chunks.find(center + glm::ivec3(x, y, z));
                if (neighbor != m_chunks.end())
                    neighbor->second->setDirty(true);
            }
        }
    }
}


Chunk *Game::chunkFromWorld(const glm::vec3 &pos)
{
    //glm::vec3 toChunk = pos / 16.0f;
    //int x = static_cast<int>(std::floorf(toChunk.x));
    //int y = static_cast<int>(std::floorf(toChunk.y));
    //int z = static_cast<int>(std::floorf(toChunk.z));

    glm::ivec3 coords = glm::floor(glm::round(pos) / 16.0f);


    auto chunk = m_chunks.find(coords);
    if (chunk != m_chunks.end())
    {
        return chunk->second.get();
    }
    else
    {
        return nullptr;
    }
}