#include "inputmanager.h"

InputManager::InputManager() : m_keys{false}
{
    
}

void InputManager::handleKey(int key, int action)
{
    m_keys[key] = action != GLFW_RELEASE;
}

bool InputManager::keyPressed(int key)
{
    return m_keys[key];
}