#include "game.h"

#include <algorithm>
#include <chrono>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "blocks.h"
#include "chunk.h"
#include "shader.h"
#include "texture.h"

Game::Game(GLFWwindow *window) : m_window(window), m_camera(glm::vec3(-88, 49, -28)),
    m_lastX(960), m_lastY(540), m_firstMouse(true), m_chunkGenerator(), m_processed(),
    m_renderer(m_chunks)
{
    m_eraseDistance = sqrtf(3 * (pow(16 * m_loadDistance, 2))) + 32.0f;
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
        processInput(dt);

        update();

        float daylight = (1.0f + sinf(glfwGetTime() * 0.5f)) * 0.5f * 0.7f;
        m_renderer.setSkyColor(skyColor * daylight);
        m_renderer.setDaylight(daylight);
        m_renderer.render(m_camera);

        glfwSwapBuffers(m_window);
        glfwPollEvents();

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
    if (glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(m_window, true);
    }

    float camSpeed = 2.5f * dt;
    if (glfwGetKey(m_window, GLFW_KEY_W) == GLFW_PRESS)
    {
        m_camera.processKeyboard(Camera::Movement::FOWARD, dt);
    }
    if (glfwGetKey(m_window, GLFW_KEY_S) == GLFW_PRESS)
    {
        m_camera.processKeyboard(Camera::Movement::BACKWARD, dt);
    }
    if (glfwGetKey(m_window, GLFW_KEY_A) == GLFW_PRESS)
    {
        m_camera.processKeyboard(Camera::Movement::LEFT, dt);
    }
    if (glfwGetKey(m_window, GLFW_KEY_D) == GLFW_PRESS)
    {
        m_camera.processKeyboard(Camera::Movement::RIGHT, dt);
    }
    if (glfwGetKey(m_window, GLFW_KEY_SPACE) == GLFW_PRESS)
    {
        m_camera.processKeyboard(Camera::Movement::UP, dt);
    }
    if (glfwGetKey(m_window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
    {
        m_camera.processKeyboard(Camera::Movement::DOWN, dt);
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
            std::cout << "place: " << local.x << ", " << local.y << ", " << local.z << std::endl;
            bool left = glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
            c->setBlock(local.x, 
                        local.y,
                        local.z, left ? Blocks::Air : Blocks::Glowstone);
            dirtyChunks(c->getCoords());
        }
    }
}

void Game::update()
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
    glm::vec3 toChunk = pos / 16.0f;
    int x = static_cast<int>(std::floorf(toChunk.x));
    int y = static_cast<int>(std::floorf(toChunk.y));
    int z = static_cast<int>(std::floorf(toChunk.z));

    auto chunk = m_chunks.find(glm::ivec3(x, y, z));
    if (chunk != m_chunks.end())
    {
        return chunk->second.get();
    }
    else
    {
        return nullptr;
    }
}