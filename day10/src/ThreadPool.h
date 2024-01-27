#pragma once
#include <functional>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <stdexcept>
#include <condition_variable>

class ThreadPool {
private:
    std::vector<std::thread> threads;    // 线程池
    std::queue<std::function<void()>> tasks;  // 任务队列
    std::mutex task_mtx;
    std::condition_variable cv;
    bool stop;

public:
    ThreadPool(uint32_t threadNum = std::thread::hardware_concurrency());
    ~ThreadPool();

    void add(std::function<void()>);
};