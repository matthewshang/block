#include "inputmanager.h"

InputManager::InputManager(GLFWwindow *window) : m_keys{ false }, m_mouseButtons{ false }, 
    m_lastPressed { -1.0f }, m_window(window), m_firstMouse(true)
{
    
}

void InputManager::keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    InputManager *input = reinterpret_cast<InputManager *>(glfwGetWindowUserPointer(window));
    input->handleKey(key, action);
}

void InputManager::mouseButtonCallback(GLFWwindow *window, int button, int action, int mods)
{
    InputManager *input = reinterpret_cast<InputManager *>(glfwGetWindowUserPointer(window));
    input->handleMouseButton(button, action);
}

void InputManager::handleKey(int key, int action)
{
    m_keys[key] = action != GLFW_RELEASE;
    if (!m_keys[key])
    {
        m_lastPressed[key] = 0.0f;
    }
}

void InputManager::handleCursorPos(double x, double y)
{
    m_dMouse = glm::vec2(x - m_lastMouse.x, m_lastMouse.y - y);
    m_lastMouse = glm::vec2(x, y);
}

void InputManager::handleMouseButton(int button, int action)
{
    m_mouseButtons[button] = action == GLFW_PRESS;
}

void InputManager::update(float dt)
{
    for (auto &last : m_lastPressed)
    {
        if (last != -1.0f)
        {
            last += dt;
        }
    }

    double xpos, ypos;
    glfwGetCursorPos(m_window, &xpos, &ypos);
    if (m_firstMouse)
    {
        m_lastMouse = glm::vec2(xpos, ypos);
        m_firstMouse = false;
    }
    m_dMouse = glm::vec2(xpos - m_lastMouse.x, m_lastMouse.y - ypos);
    m_lastMouse = glm::vec2(xpos, ypos);
}

bool InputManager::keyPressed(int key)
{
    return m_keys[key];
}

bool InputManager::keyDoublePressed(int key, float threshold)
{
    return m_keys[key] && m_lastPressed[key] > 0.0f && m_lastPressed[key] < threshold;
}

bool InputManager::mousePressed(int button)
{
    return m_mouseButtons[button];
}