#pragma once
#include <functional>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <stdexcept>
#include <condition_variable>
#include <future>

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

    // void add(std::function<void()>);
    template <class F, class... Args>
    auto add(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type>;
};

// 不能放在 cpp 文件，原因是 C++ 编译器不支持模板的分离编译
template <class F, class... Args>
auto ThreadPool::add(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type> {
    using return_type = typename std::result_of<F(Args...)>::type;

    auto task = std::make_shared<std::packaged_task<return_type()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));

    std::future<return_type> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(task_mtx);
        if (stop) throw std::runtime_error("enqueue on stopped ThreadPool.");
        tasks.emplace([task](){ (*task)(); });
    }
    cv.notify_one();
    return res;
}