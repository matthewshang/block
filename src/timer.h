#pragma once

#include <chrono>
#include <string>

class Timer
{
public:
    Timer() {};

    void start();
    void log(std::string label);

private:
    std::chrono::steady_clock::time_point m_start;
};