#include "sharedvector.h"

#include <mutex>

//template<typename T>
//void SharedVector<T>::push_back(const T &val)
//{
//    std::lock_guard<std::shared_mutex> lock(m_mutex);
//    m_data.push_back(val);
//}

//template<typename T>
//void SharedVector<T>::push_back(T &&val)
//{
//    std::lock_guard<std::shared_mutex> lock(m_mutex);
//    m_data.push_back(val);
//}
//
//template<typename T>
//void SharedVector<T>::clear()
//{
//    std::lock_guard<std::shared_mutex> lock(m_mutex);
//    m_data.clear();
//}
//
//template<typename T>
//void SharedVector<T>::for_each(std::function<void(T&)> func)
//{
//    std::lock_guard<std::shared_mutex> lock(m_mutex);
//    for (auto &it : m_data)
//    {
//        func(it);
//    }
//}