#pragma once

#include <map>
#include <memory>
#include <mutex>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "camera.h"
#include "chunk.h"
#include "threadpool.h"

struct ChunkCompare
{
    bool operator() (const glm::ivec3 &a, const glm::ivec3 &b) const
    {
        if (a.x != b.x)
            return a.x < b.x;
        else if (a.y != b.y)
            return a.y < b.y;
        else if (a.z != b.z)
            return a.z < b.z;
        else
            return false;
    }
};

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

    const int m_renderDistance = 1;
    std::map<glm::ivec3, std::unique_ptr<Chunk>, ChunkCompare> m_chunks;
    std::vector<std::unique_ptr<Chunk>> m_toAdd;
    std::vector<glm::ivec3> m_toErase;
    ThreadPool m_pool;
    std::mutex m_mutex;

    GLFWwindow *m_window;
    Camera m_camera;

    double m_lastX;
    double m_lastY;
    bool m_firstMouse;
};