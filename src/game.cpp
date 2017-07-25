#include "game.h"

#include <algorithm>
#include <chrono>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "chunk.h"
#include "shader.h"
#include "texture.h"

Game::Game(GLFWwindow *window) : m_window(window), m_camera(glm::vec3(0.0f, 40.0f, 2.0f)), 
m_lastX(960), m_lastY(540), m_firstMouse(true), m_noise(), m_processed()
{
    initChunks();
}

void Game::run()
{
    Shader shader("../res/shaders/simple_vertex.glsl", "../res/shaders/simple_fragment.glsl");

    Texture texture1("../res/textures/grass.jpg", GL_RGB);

    //Chunk chunk(ChunkCoord(0, 0, 0));

    glEnable(GL_DEPTH_TEST);

    shader.bind();
    shader.setInt("texture1", 0);

    glm::mat4 projection;
    projection = glm::perspective(glm::radians(45.0f), static_cast<float>(1920) / static_cast<float>(1080), 0.1f, 100.0f);

    float dt = 0.0f;
    double lastFrame = 0.0f;
    long nFrames = 0;
    double lastTime = 0.0f;

    while (!glfwWindowShouldClose(m_window))
    {
        processInput(dt);

        loadChunks();

        auto start = std::chrono::high_resolution_clock::now();

        for (const auto& it : m_chunks)
        {
            auto& chunk = it.second;
            updateChunk(chunk.get());
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

        if (diff.count() > 0) std::cout << "loadChunks: " << diff.count() << "ms" << std::endl;

        for (const auto &chunk : m_toErase)
        {
            m_chunks.erase(chunk);
        }
        m_toErase.clear();

        m_processed.moveChunks(m_chunks);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        //glEnable(GL_CULL_FACE);

        shader.bind();

        glm::mat4 model;

        glm::mat4 view = m_camera.getView();

        shader.setMat4("model", model);
        shader.setMat4("view", m_camera.getView());
        shader.setMat4("projection", projection);

        texture1.bind(GL_TEXTURE0);

        for (const auto& it : m_chunks)
        {
            auto& chunk = it.second;
            if (!chunk->isEmpty())
            {
                chunk->bind();
                glDrawArrays(GL_TRIANGLES, 0, chunk->getVertexCount());
            }
        }

        glfwSwapBuffers(m_window);
        glfwPollEvents();

        double current = glfwGetTime();
        dt = static_cast<float>(current - lastFrame);
        lastFrame = current;

        nFrames++;
        if (current - lastTime > 1.0f)
        {
            char title[256];
            title[255] = '\0';
            snprintf(title, 255, "block - [FPS: %d] [%d chunks] [pos: %f, %f, %f]", nFrames, m_chunks.size(),
                m_camera.getPos().x, m_camera.getPos().y, m_camera.getPos().z);
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

    //if (glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS ||
    //    glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
    //{
    //    const glm::vec3 &pos = m_camera.getPos();
    //    Chunk *c = chunkFromWorld(pos);
    //    if (c != nullptr)
    //    {
    //        const ChunkCoord &coords = c->getCoords();
    //        std::cout << coords.x << ", " << coords.y << ", " << coords.z << std::endl;

    //        glm::vec3 local = pos - glm::vec3(coords.getXWorld(), coords.getYWorld(), coords.getZWorld());
    //        local = glm::floor(local);
    //        bool left = glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    //        c->setBlock(static_cast<int>(local.x), 
    //                    static_cast<int>(local.y),
    //                    static_cast<int>(local.z), left ? 0 : 1);

    //    }
    //}
}

void Game::updateChunk(Chunk *chunk)
{
    if (glm::distance(chunk->getCenter(), m_camera.getPos()) > 64)
    {
        m_toErase.push_back(chunk->getCoords());
        return;
    }
    chunk->buildMesh();
}

void Game::loadChunks()
{
    glm::vec3 toChunk = m_camera.getPos() / 16.0f;
    glm::ivec3 current(
        static_cast<int>(std::floorf(toChunk.x)),
        static_cast<int>(std::floorf(toChunk.y)),
        static_cast<int>(std::floorf(toChunk.z)));

    for (int x = -m_renderDistance; x <= m_renderDistance; x++)
    {
        for (int y = -m_renderDistance; y <= m_renderDistance; y++)
        {
            for (int z = -m_renderDistance; z <= m_renderDistance; z++)
            {
                glm::ivec3 coords = current + glm::ivec3(x, y, z);
                if (m_processed.hasChunk(coords)) 
                    continue;

                auto chunk = m_chunks.find(coords);
                if (chunk == m_chunks.end())
                {
                    Chunk *c = new Chunk(coords);
                    auto lambda = [c, this]() -> void
                    {
                        makeTerrain(*c);
                        std::unique_ptr<Chunk> ptr(c);
                        m_processed.push(ptr);
                    };
                    m_pool.addJob(lambda);
                    //std::unique_ptr<Chunk> c = std::make_unique<Chunk>(coords);
                    //makeTerrain(*c);
                    //m_processed.push(c);
                }
            }
        }
    }
}

void Game::makeTerrain(Chunk &c)
{
    const glm::ivec3 &coords = c.getCoords();
    auto start = std::chrono::high_resolution_clock::now();

    for (int x = 0; x < CHUNK_SIZE; x++)
    {
        for (int y = 0; y < CHUNK_SIZE; y++)
        {
            for (int z = 0; z < CHUNK_SIZE; z++)
            {
                int rx = coords.x * 16 + x;
                int ry = coords.y * 16 + y;
                int rz = coords.z * 16 + z;

                double p = m_noise.noise3(
                    static_cast<double>(rx) * 0.07,
                    static_cast<double>(ry) * 0.07,
                    static_cast<double>(rz) * 0.07);

                double depth = 1.0 - std::max(0.0, std::min((static_cast<double>(ry) + 64.0) / 128.0, 1.0));

                p = p * 0.25 + depth * 0.75;
                 
                if (p > 0.3)
                {
                    c.setBlock(x, y, z, 1);
                }
            }
        }
    }

    auto end = std::chrono::high_resolution_clock::now();

    auto diff = end - start;
    auto nano = std::chrono::duration_cast<std::chrono::nanoseconds>(diff);
    //std::cout << "average: " << nano.count() / 4096 << "ns" << std::endl;
}

void Game::initChunks()
{
    glm::vec3 toChunk = m_camera.getPos() / 16.0f;
    int x = static_cast<int>(std::floorf(toChunk.x));
    int y = static_cast<int>(std::floorf(toChunk.y));
    int z = static_cast<int>(std::floorf(toChunk.z));

    glm::ivec3 p(x, y, z);
    std::unique_ptr<Chunk> c = std::make_unique<Chunk>(p);
    makeTerrain(*c);
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