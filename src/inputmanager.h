#pragma once

#include <array>

#include <GLFW/glfw3.h>

class InputManager
{
public:
    InputManager();

    void handleKey(int key, int action);

    void update(float dt);
    
    bool keyPressed(int key);
    bool keyDoublePressed(int key, float threshold);

private:
    std::array<bool, GLFW_KEY_LAST> m_keys;
    std::array<float, GLFW_KEY_LAST> m_lastPressed;

};