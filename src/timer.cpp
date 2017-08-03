#include "timer.h"

#include <iostream>

void Timer::start()
{
    m_start = std::chrono::high_resolution_clock::now();
}

void Timer::log(std::string label)
{
    auto end = std::chrono::high_resolution_clock::now();
    auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(end - m_start);

    std::cout << label << diff.count() << "ms" << std::endl;
}