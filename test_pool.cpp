// test_pool.cpp （测试完可删除）
#include "ThreadPool.hpp"
#include <iostream>
#include <chrono>

int main() {
    ThreadPool pool(4); // 4个线程
    // 添加10个任务
    for(int i = 0; i < 10; ++i) {
        pool.enqueue([i] {
            std::this_thread::sleep_for(std::chrono::milliseconds(100)); // 模拟工作
            std::cout << "Task " << i << " executed by thread " 
                      << std::this_thread::get_id() << std::endl;
        });
    }
    // 析构函数会等待所有任务完成
    return 0;
}