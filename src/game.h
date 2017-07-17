#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "camera.h"

class Game
{
public:
    Game(GLFWwindow *window);

    void run();

private:
    void processInput(float dt);

    GLFWwindow *m_window;
    Camera m_camera;

    double m_lastX;
    double m_lastY;
    bool m_firstMouse;
};