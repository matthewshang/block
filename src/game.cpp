#include "game.h"

#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "chunk.h"
#include "shader.h"
#include "texture.h"

Game::Game(GLFWwindow *window) : m_window(window), m_camera(glm::vec3(0.0f, 2.5f, 2.0f)), 
m_lastX(960), m_lastY(540), m_firstMouse(true)
{
    initChunks();
}

void Game::run()
{
    Shader shader("../res/shaders/simple_vertex.glsl", "../res/shaders/simple_fragment.glsl");

    Texture texture1("../res/textures/dirt.jpg", GL_RGB);

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

        std::vector<glm::ivec3> eraseList;
        for (const auto& it : m_chunks)
        {
            auto& chunk = it.second;
            updateChunk(*chunk, eraseList);
        }

        for (const auto &chunk : eraseList)
        {
            m_chunks.erase(chunk);
        }

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
            snprintf(title, 255, "block - [FPS: %d] [%d chunks]", nFrames, m_chunks.size());
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

void Game::updateChunk(Chunk &chunk, std::vector<glm::ivec3> &eraseList)
{
    if (glm::distance(chunk.getCenter(), m_camera.getPos()) > 64)
    {
        chunk.unhookNeighbors();
        eraseList.push_back(chunk.getCoords());
        return;
    }
    chunk.buildMesh();
    if (chunk.getNumNeighbors() < 6)
    {
        static const glm::ivec3 positions[6] = {
            glm::ivec3(0, 0, 1), glm::ivec3(0, 0, -1), glm::ivec3(-1, 0, 0),
            glm::ivec3(1, 0, 0), glm::ivec3(0, 1, 0), glm::ivec3(0, -1, 0)
        };

        for (int i = 0; i < 6; i++)
        {
            if (chunk.getNeighbor(i) == nullptr)
            {
                glm::vec3 newPos = chunk.getCenter() + glm::vec3(positions[i] * 16) - m_camera.getPos();
                if (glm::length(newPos) > 64)
                    continue;

                glm::ivec3 newCoords = positions[i] + chunk.getCoords();
                auto neighbor = m_chunks.find(newCoords);
                if (neighbor == m_chunks.end())
                {
                    std::unique_ptr<Chunk> c = std::make_unique<Chunk>(newCoords);
                    makeTerrain(*c);
                    chunk.hookNeighbor(i, c.get());
                    c->hookNeighbor(Chunk::opposites[i], &chunk);
                    m_chunks.insert(std::make_pair(newCoords, std::move(c)));
                }
                else
                {
                    chunk.hookNeighbor(i, neighbor->second.get());
                    neighbor->second->hookNeighbor(Chunk::opposites[i], &chunk);
                }
            }
        }
    }
}

void Game::makeTerrain(Chunk &c)
{
    //if (c.getCoords().y >= 0) return;
    //for (int x = 0; x < CHUNK_SIZE; x++)
    //{
    //    for (int y = 0; y < CHUNK_SIZE; y++)
    //    {
    //        for (int z = 0; z < CHUNK_SIZE; z++)
    //        {
    //            c.setBlock(x, y, z, 1);
    //        }
    //    }
    //}

    for (int x = 6; x < 10; x++)
    {
        for (int y = 6; y < 10; y++)
        {
            for (int z = 6; z < 10; z++)
            {
                c.setBlock(x, y, z, 1);
            }
        }
    }
}

void Game::initChunks()
{
    glm::ivec3 p(0, -1, 0);
    std::unique_ptr<Chunk> c = std::make_unique<Chunk>(p);
    makeTerrain(*c);
    m_chunks.insert(std::make_pair(p, std::move(c)));
    //for (int x = -1; x < 1; x++)
    //{
    //    for (int y = -1; y < 1; y++)
    //    {
    //        for (int z = -1; z < 1; z++)
    //        {
    //            glm::ivec3 p(x, y, z);
    //            std::unique_ptr<Chunk> c = std::make_unique<Chunk>(p);
    //            makeTerrain(*c);
    //            m_chunks.insert(std::make_pair(p, std::move(c)));
    //        }
    //    }
    //}
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