#include "ThreadPool.h"

ThreadPool::ThreadPool(uint32_t threadNum) : stop(false) {
    for (int i = 0; i < threadNum; ++i) {
        threads.emplace_back(std::thread([this](){
            while (true) {
                std::function<void()> task;   // 线程需要执行的任务
                {
                    std::unique_lock<std::mutex> lock(task_mtx);
                    cv.wait(lock, [this](){
                        return stop || !tasks.empty();
                    });
                    if (stop && tasks.empty()) return;
                    task = tasks.front();
                    tasks.pop();
                }
                task();        // 执行任务
            }
        }));
    }
}

ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(task_mtx);
        stop = true;
    }
    cv.notify_all();  // 唤醒所有线程
    for (auto &th : threads) {
        if (th.joinable())
            th.join();
    }
}

void ThreadPool::add(std::function<void()> func) {
    {
        std::unique_lock<std::mutex> lock(task_mtx);
        if (stop) {
            throw std::runtime_error("ThreadPool already stop, can't add task any more");
        }
        tasks.emplace(func);  // 增加到任务队列
    }
    cv.notify_one();          // 唤醒一个线程处理
}