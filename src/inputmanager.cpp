#include "inputmanager.h"

InputManager::InputManager() : m_keys{ false }, m_lastPressed{ -1.0f }
{
    
}

void InputManager::handleKey(int key, int action)
{
    m_keys[key] = action != GLFW_RELEASE;
    if (!m_keys[key])
    {
        m_lastPressed[key] = 0.0f;
    }
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
}

bool InputManager::keyPressed(int key)
{
    return m_keys[key];
}

bool InputManager::keyDoublePressed(int key, float threshold)
{
    return m_keys[key] && m_lastPressed[key] > 0.0f && m_lastPressed[key] < threshold;
}