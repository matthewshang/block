#pragma once

#include <map>
#include <memory>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "camera.h"
#include "chunk.h"

class Game
{
public:
    Game(GLFWwindow *window);

    void run();

private:
    void processInput(float dt);
    void initChunks();

    std::map<ChunkCoord, std::unique_ptr<Chunk>> m_chunks;

    GLFWwindow *m_window;
    Camera m_camera;

    double m_lastX;
    double m_lastY;
    bool m_firstMouse;
};