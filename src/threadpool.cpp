#include "threadpool.h"

#include <iostream>
#include <mutex>

ThreadPool::ThreadPool() : m_stop(false), m_jobsPending(0)
{
    int nThreads = std::thread::hardware_concurrency();
    for (int i = 0; i < nThreads; i++)
    {
        m_workers.push_back(std::thread(&ThreadPool::getJob, this));
    }
}

ThreadPool::~ThreadPool()
{
    m_stop = true;
    m_cond.notify_all();

    for (int i = 0; i < m_workers.size(); i++)
    {
        m_workers[i].join();
    }  
}

void ThreadPool::addJob(std::function<void()> job)
{
    m_jobsPending++;
    std::unique_lock<std::mutex> lock(m_mutex);
    m_queue.push(job);
    lock.unlock();
    m_cond.notify_one();
}

void ThreadPool::waitUntilCompleted()
{
    if (m_jobsPending <= 0) return;
    std::unique_lock<std::mutex> lock(m_finishMutex);
    m_finishCond.wait(lock);
}

void ThreadPool::getJob()
{
    while (true)
    {
        std::unique_lock<std::mutex> lock(m_mutex);

        m_cond.wait(lock, [this] {return !m_queue.empty() || m_stop; });

        if (m_stop)
            return;

        std::function<void()> job = m_queue.front();
        m_queue.pop();
        lock.unlock();

        job();
        if (--m_jobsPending == 0)
        {
            m_finishCond.notify_one();
        }
    }
}