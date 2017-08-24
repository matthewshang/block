#pragma once

#include <array>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

class InputManager
{
public:
    static void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
    static void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods);

    InputManager(GLFWwindow *window);

    void handleKey(int key, int action);
    void handleCursorPos(double x, double y);
    void handleMouseButton(int button, int action);

    void update(float dt);
    
    bool keyPressed(int key);
    bool keyDoublePressed(int key, float threshold);

    const glm::vec2 &getCursorDelta() { return m_dMouse; };
    bool mousePressed(int button);

private:
    GLFWwindow *m_window;

    std::array<bool, GLFW_KEY_LAST> m_keys;
    std::array<float, GLFW_KEY_LAST> m_lastPressed;

    bool m_firstMouse;
    glm::vec2 m_lastMouse;
    glm::vec2 m_dMouse;
    std::array<bool, GLFW_MOUSE_BUTTON_LAST> m_mouseButtons;
};