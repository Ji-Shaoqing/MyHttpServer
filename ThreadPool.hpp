// ThreadPool.hpp
#ifndef THREADPOOL_HPP
#define THREADPOOL_HPP

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>
#include <iostream>

class ThreadPool {
public:
    // 构造函数，创建指定数量的工作线程
    explicit ThreadPool(size_t numThreads) : stop(false) {
        for(size_t i = 0; i < numThreads; ++i) {
            // 每个工作线程是一个无限循环的lambda函数
            workers.emplace_back([this] {
                while(true) {
                    std::function<void()> task; // 用于存储从队列取出的任务
                    {
                        // 1. 获取队列锁
                        std::unique_lock<std::mutex> lock(this->queue_mutex);
                        
                        // 2. 等待条件成立：停止信号或任务队列非空
                        //    条件变量会暂时释放锁并休眠，被唤醒后重新获取锁
                        this->condition.wait(lock, [this] {
                            return this->stop || !this->tasks.empty();
                        });
                        
                        // 3. 如果收到停止信号且任务已清空，则线程结束
                        if(this->stop && this->tasks.empty()) {
                            return;
                        }
                        
                        // 4. 从队列中取出一个任务
                        task = std::move(this->tasks.front());
                        this->tasks.pop();
                        // 注意：锁在作用域结束时（此处）自动释放
                    }
                    
                    // 5. 执行任务（此时已释放锁，其他线程可以并行操作队列）
                    task();
                }
            });
        }
    }

    // 向线程池添加任务（模板函数，支持任何可调用对象）
    template<class F>
    void enqueue(F&& task) {
        {
            // 获取队列锁，保护共享的任务队列
            std::lock_guard<std::mutex> lock(queue_mutex);
            if(stop) {
                throw std::runtime_error("Cannot enqueue on a stopped ThreadPool");
            }
            // 将任务添加到队列
            tasks.emplace(std::forward<F>(task));
        }
        // 通知一个正在等待的工作线程（如果有）
        condition.notify_one();
    }

    // 析构函数：安全关闭线程池
    ~ThreadPool() {
        {
            std::lock_guard<std::mutex> lock(queue_mutex);
            stop = true; // 设置停止标志
        }
        condition.notify_all(); // 唤醒所有工作线程
        // 等待所有线程执行完毕
        for(std::thread &worker : workers) {
            if(worker.joinable()) {
                worker.join();
            }
        }
        std::cout << "[ThreadPool] All threads stopped." << std::endl;
    }

private:
    std::vector<std::thread> workers; // 工作线程容器
    std::queue<std::function<void()>> tasks; // 任务队列（函数包装器）
    std::mutex queue_mutex; // 保护任务队列的互斥锁
    std::condition_variable condition; // 用于线程同步的条件变量
    std::atomic<bool> stop; // 原子布尔值，标志线程池是否停止
};

#endif // THREADPOOL_HPP