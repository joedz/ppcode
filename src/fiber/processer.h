#pragma once 

#include <memory>
#include <boost/lockfree/spsc_queue.hpp>
#include <list>
#include <atomic>
#include <memory>

#include "../thread.h"
#include "fiber.h"

namespace ppcode {
class Scheduler;
//
class Processer : public std::enable_shared_from_this<Processer> {
public:
    friend class Scheduler;
    // 单生成者单消费者
    using fiberQueue = boost::lockfree::spsc_queue<Fiber::ptr, 
            boost::lockfree::capacity<128UL> >;
    using ptr = std::shared_ptr<Processer>;
    using MutexType = ppcode::Mutex;

    Processer(Scheduler*, uint32_t id);
    ~Processer();

// 为协程提供的方法

    // 协程切出
    static void toYield();
    // 协程切出
    void yield();
    // 阻塞
    // static void Suspend();
    // // 阻塞
    // void suspend();
    // 获取执行器
    static Processer*& getThis();
    static Fiber* getCurrentFiber();

// 为调度器提供的方法
    // 添加协程
    void addFiber(Fiber::ptr fiber);
    // 添加多个协程  加锁
    void addFibers(std::list<Fiber::ptr>& list);
    // 执行器是否在等待
    bool isWaitting() { return !m_running;}
    // 执行器是否已经停止
    bool isStop() { return m_stopping;}
    // 执行器是否在执行协程
    bool isRunning() { return m_running;}
    // 执行器中的协程个数
    uint64_t filberSize() { return m_fiberSize;}
     // 调度器执行使执行器执行
    void start();
    //  调度器通知等待的执行器
    virtual void notify();

// both

//调度器本身方法
    // 执行器执行
    void execute();
    // gc回收
    void gc();
    //void getNewFibers();
    // 获取当前调度器
    void getScheduler();
    // 执行器等待时的方法
    virtual void waitting();
    // 执行器添加新的协程 从 newQueue 到 runableQueue
    void getNewFibers();

private:
    // 执行器id ,对应在调度器的位置
    uint32_t m_id;
    // 新加入的协程数量  TODO 更改名称
    std::atomic<unsigned long> m_fiberSize{0};
    // 现有的可运行的协程数量   TODO 可能被其他线程访问 原子 ?? 
    unsigned long m_nowFiberSize = 0;
    // 协程阻塞的次数
    uint64_t m_yieldCount = 0;
    // 协程切换的次数
    uint64_t m_swithCount = 0;
    // 协程可运行队列 
    fiberQueue runableQueue;
    // 协程阻塞队列
    fiberQueue waitQueue;
    // 协程新加入队列
    fiberQueue newQueue;
    // gc队列
    std::list<Fiber::ptr> gcQueue;
    // 执行器状态 等待 死亡 执行
    std::atomic_bool m_running {false};
    std::atomic_bool m_stopping {false};
    // 条件变量
    Condition_variable m_cv;
    // 互斥锁
    MutexType m_mutex;
    Scheduler* m_sche;

    // 是否被唤醒
    bool m_notidied = false;
    // 调度器器
    // scheduler*  m_sche;
};

}
