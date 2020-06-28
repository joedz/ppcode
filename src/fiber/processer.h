#pragma once

#include <boost/lockfree/queue.hpp>
#include <boost/lockfree/spsc_queue.hpp>
#include <iostream>
#include <list>
#include <memory>
#include <string>

#include "../thread.h"
#include "../thread/lockfreelist.h"
#include "fiber.h"

/*
    使用boost库无锁队列
*/



namespace ppcode {

class Scheduler;

class Processer {
    // using FiberQueue = LockFreeList;
    using FiberQueue = boost::lockfree::spsc_queue<Fiber*, boost::lockfree::capacity<1024> >;
    
    using MutexType = Mutex; 

public:
    friend class Scheduler;
    using ptr = std::shared_ptr<Processer>;
    ~Processer() = default;
    uint64_t getId() { return m_id; }
    std::string getName() { return m_name; }
 Processer(Scheduler* sche, size_t num, const std::string& name);
    bool isActive(){return true;}

public:
    static Fiber* GetCuttentFiber();
    static Processer* GetCurrentProcesser();
    static Scheduler* GetScheduler();
    static void ToYield();
    static bool Wakeup();
    
private:
    // 友元函数提供的方法

    // 执行器的构造方法
   
    // 执行
    void Process();
    // 通知执行器 条件到达
    void NotifyCondition();
    // 添加协程
    void AddFiber(Fiber::ptr fb);
    // 偷取协程
   // std::list<Fiber> Steal(size_t n);

private:
    // 等待条件到达
    void waitCondition();

    // 垃圾回收
    void gc();
    void addNewFiber();

private:
    Scheduler* m_sche;                   // Scheduler指针
    std::string m_name;                  // 执行器名称
    uint64_t m_id;                       // 执行器id
    uint64_t m_switchCount;              // 执行器调度次数
    std::atomic_uint32_t m_addNewFiber;  // 执行器新加入的Fiber数量

    Condition_variable m_cv;            // 执行器条件变量
    std::atomic_bool m_waiting{false};  // 执行器是否处于等待条件
    std::atomic_bool m_notified{false}; // 是狗通知了执行器
    MutexType m_mutex;                  // 互斥锁

    FiberQueue runnableQueue;  // 可以运行的队列
    FiberQueue waitQueue;      // 等待队列
    std::list<Fiber*> gcQueue;        // 等带回收队列
    FiberQueue newQueue;       // 新加入的协程队列
};
}  // namespace ppcode
