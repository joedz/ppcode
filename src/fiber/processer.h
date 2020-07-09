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

    // 协程切出
    static void toYield();
    // 协程切出
    void yield();

    static Processer*& getThis();
    static Fiber::ptr getCurrentFiber();

// 为调度器提供的方法
    // 添加协程
    void addFiber(Fiber::ptr fiber);
    // 添加多个协程  加锁
    void addFibers(std::list<Fiber::ptr>& list);
    // 执行器是否在执行协程
    bool isRunning() { return m_state == TaskState::running;}
    // 执行器是否在等待
    bool isWaitting() { return m_state == TaskState::idle;}
    // 执行器是否已经停止
    bool isStop() { return m_state == TaskState::done;}
    // 执行器处于阻塞执行状态 此时协程正在执行 但是由于协程处于阻塞状态,但是此时可以被其他执行器窃取协程
    bool isBlock() { return m_state == TaskState::block;}

    TaskState getState() const { return m_state.load();}
    void setState(TaskState state)  { m_state.store(state);}

    // 执行器中的协程个数 0
    uint64_t filberSize() { return m_newfiberCount;}

    //  调度器通知等待的执行器
     void notify();

//调度器本身方法
    // 执行器执行
    void execute();
    // gc回收
    void gc();
    //void getNewFibers();
    // 获取当前调度器
    Scheduler*  getScheduler();
    // 执行器等待时的方法
     void idle();
    // 执行器添加新的协程 从 newQueue 到 runableQueue
    void getNewFibers();

protected:
    // 执行器id ,对应在调度器的位置
    uint32_t m_id;

    // 新加入的协程数量
    std::atomic<unsigned long> m_newfiberCount{0};

    // 现有的可运行的协程数量   TODO 可能被其他线程访问 原子 ?? 
    unsigned long m_nowFiberSize = 0;
    // 协程阻塞的次数
    uint64_t m_yieldCount = 0;
    // 协程切换的次数
    uint64_t m_swithCount = 0;
    // 执行的协程数量,
    uint64_t m_FiberCount = 0;
    // 执行队列的选取问题 UNDO

    // 协程可运行队列 
    std::list<Fiber::ptr> runableQueue;
    // 协程阻塞队列
    fiberQueue waitQueue;
    // 协程新加入队列
    fiberQueue newQueue;
    // gc队列
    std::list<Fiber::ptr> gcQueue;

    // 条件变量
    Condition_variable m_cv;
    // 互斥锁
    MutexType m_mutex;
    Scheduler* m_sche;
    std::atomic<TaskState> m_state;
    // 执行器状态 死亡 执行 阻塞

    // 是否被唤醒
    bool m_notidied = false;
    // 调度器器
    // scheduler*  m_sche;
};

}
