#pragma once

#include <mutex>
#include <shared_mutex>
#include <vector>

template<typename T>
class SharedVector
{
public:
    SharedVector() : m_data(), m_mutex() {};

    void push_back(T &val);
    void clear();
    void for_each(std::function<void(T&)> func);

private:
    std::vector<T> m_data;
    std::shared_mutex m_mutex;
};

template<typename T>
void SharedVector<T>::push_back(T &val)
{
    std::lock_guard<std::shared_mutex> lock(m_mutex);
    m_data.push_back(std::move(val));
}

template<typename T>
void SharedVector<T>::clear()
{
    std::lock_guard<std::shared_mutex> lock(m_mutex);
    m_data.clear();
}

template<typename T>
void SharedVector<T>::for_each(std::function<void(T&)> func)
{
    std::lock_guard<std::shared_mutex> lock(m_mutex);
    for (auto &it : m_data)
    {
        func(it);
    }
}