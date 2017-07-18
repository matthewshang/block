#pragma once

#include <map>
#include <memory>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "camera.h"
#include "chunk.h"

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
    void processInput(float dt);
    void updateChunk(Chunk &chunk);
    void initChunks();
    Chunk *chunkFromWorld(const glm::vec3 &pos);

    std::map<glm::ivec3, std::unique_ptr<Chunk>, ChunkCompare> m_chunks;

    GLFWwindow *m_window;
    Camera m_camera;

    double m_lastX;
    double m_lastY;
    bool m_firstMouse;
};