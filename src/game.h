#pragma once

#include <map>
#include <memory>
#include <mutex>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "camera.h"
#include "chunk.h"
#include "chunkcompare.h"
#include "chunklist.h"
#include "perlin.h"
#include "threadpool.h"

class Game
{
public:
    Game(GLFWwindow *window);

    void run();

private:
    void makeTerrain(Chunk &c);
    void processInput(float dt);
    void updateChunk(Chunk *chunk);
    void loadChunks();
    void initChunks();
    Chunk *chunkFromWorld(const glm::vec3 &pos);

    const int m_renderDistance = 2;
    std::map<glm::ivec3, std::unique_ptr<Chunk>, ChunkCompare> m_chunks;
    ChunkList m_processed;
    std::vector<glm::ivec3> m_toErase;
    ThreadPool m_pool;
    Perlin m_noise;

    GLFWwindow *m_window;
    Camera m_camera;

    double m_lastX;
    double m_lastY;
    bool m_firstMouse;
};