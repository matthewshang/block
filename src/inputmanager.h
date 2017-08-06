#pragma once

#include <array>

#include <GLFW/glfw3.h>

class InputManager
{
public:
    InputManager();

    void handleKey(int key, int action);
    bool keyPressed(int key);

private:
    std::array<bool, GLFW_KEY_LAST> m_keys;
};