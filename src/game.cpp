#include "game.h"

#include <algorithm>
#include <chrono>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include "blocks.h"
#include "chunk.h"
#include "shader.h"
#include "texture.h"

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
        m_input.update(dt);

        processInput(dt);

        updateChunks();
        updatePlayer(dt);

        //float daylight = (1.0f + sinf(glfwGetTime() * 0.5f)) * 0.5f * 0.7f;
        float daylight = 0.25f;
        m_renderer.setSkyColor(skyColor * daylight);
        m_renderer.setDaylight(daylight);
        m_renderer.render(m_camera);

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

void Game::processInput(float dt)
{
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
        const glm::vec3 &pos = m_camera.getPos();
        Chunk *c = chunkFromWorld(pos);
        if (c != nullptr)
        {
            glm::vec3 a = glm::floor(pos);
            glm::ivec3 local(static_cast<int>(a.x), static_cast<int>(a.y), static_cast<int>(a.z));
            local = glm::ivec3((local.x % 16 + 16) % 16, (local.y % 16 + 16) % 16, (local.z % 16 + 16) % 16);
            //std::cout << "place: " << local.x << ", " << local.y << ", " << local.z << std::endl;
            bool left = glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
            c->setBlock(local.x, 
                        local.y,
                        local.z, left ? Blocks::Air : Blocks::Glowstone);
            dirtyChunks(c->getCoords());
        }
    }
}

void Game::updateChunks()
{
    loadChunks();

    for (const auto& it : m_chunks)
    {
        auto& chunk = it.second;
        updateChunk(chunk.get());
    }

    for (const auto &chunk : m_toErase)
    {
        m_chunks.erase(chunk);
        m_loadedChunks.erase(chunk);
    }
    m_toErase.clear();

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
                        neighbor->second->dirty();
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

    if (m_input.keyDoublePressed(GLFW_KEY_SPACE, 0.25f))
    {
        std::cout << "double" << std::endl;
        m_flying = !m_flying;
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

void Game::updateChunk(Chunk *chunk)
{
    if (glm::distance(chunk->getCenter(), m_camera.getPos()) > m_eraseDistance)
    {
        m_toErase.push_back(chunk->getCoords());
        return;
    }

    if (chunk->isDirty())
    {
        auto update = [this, chunk]() -> void
        {
            auto compute = std::make_unique<ComputeJob>(*chunk, m_chunks);
            compute->execute();
            m_updates.push_back(compute);
        };
        m_pool.addJob(update);
    }
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
                    neighbor->second->dirty();
            }
        }
    }
}

void Game::loadChunks()
{
    glm::vec3 toChunk = m_camera.getPos() / 16.0f;
    glm::ivec3 current(
        static_cast<int>(std::floorf(toChunk.x)),
        static_cast<int>(std::floorf(toChunk.y)),
        static_cast<int>(std::floorf(toChunk.z)));

    for (int x = -m_loadDistance; x <= m_loadDistance; x++)
    {
        for (int y = -m_loadDistance; y <= m_loadDistance; y++)
        {
            for (int z = -m_loadDistance; z <= m_loadDistance; z++)
            {
                glm::ivec3 coords = current + glm::ivec3(x, y, z);
                if (m_loadedChunks.find(coords) != m_loadedChunks.end()) 
                    continue;

                Chunk *c = new Chunk(coords);
                m_loadedChunks.insert(coords);
                auto lambda = [c, this]() -> void
                {
                    m_chunkGenerator.generate(*c);
                    c->compute(m_chunks);
                    std::unique_ptr<Chunk> ptr(c);
                    m_processed.push_back(ptr);
                };
                m_pool.addJob(lambda);
            }
        }
    }
}

void Game::initChunks()
{
    glm::vec3 toChunk = m_camera.getPos() / 16.0f;
    int x = static_cast<int>(std::floorf(toChunk.x));
    int y = static_cast<int>(std::floorf(toChunk.y));
    int z = static_cast<int>(std::floorf(toChunk.z));

    glm::ivec3 p(x, y, z);
    std::unique_ptr<Chunk> c = std::make_unique<Chunk>(p);
    m_chunkGenerator.generate(*c);
    m_chunks.insert(std::make_pair(p, std::move(c)));
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