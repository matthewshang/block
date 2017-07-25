#pragma once

#include <atomic>
#include <condition_variable>
#include <queue>
#include <thread>
#include <vector>

class ThreadPool
{
public:
    ThreadPool();
    ~ThreadPool();

    void addJob(std::function<void()> job);
    void waitUntilCompleted();
    int getJobsAmount();

private:
    void getJob();

    std::vector<std::thread> m_workers;
    std::queue<std::function<void()>> m_queue;
    std::mutex m_mutex;
    std::condition_variable m_cond;

    std::atomic<int> m_jobsPending;
    std::mutex m_finishMutex;
    std::condition_variable m_finishCond;
    bool m_stop;
};