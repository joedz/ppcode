#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "../thread.h"
#include "../util/singleton.h"
#include "poller.h"
#include "processer.h"

namespace ppcode {

// 协程调度器
class Scheduler {
public:
    using ptr = std::shared_ptr<Scheduler>;
    using MutexType = Mutex;

    // 协程构造函数 单例
    Scheduler(size_t threads = 0, const std::string& name = "scheduler");

    // 析构函数 停止所有子线程
    ~Scheduler();

    // 创建协程的接口    限制只能从这里创建协程
    void createFiber(const std::function<void()>& cb);

    // 获取网路轮询器的接口 通过这里执行网路事件监听的接口
    Poller* getPoller() { return m_poller; }

    // 调度器执行 创建协程执行器
    void start();

    // 调度器是否正在停止
    bool isStopping() const { return m_state == TaskState::stop; }

    // 获取协程执行器状态
    TaskState getState() const { return m_state.load(); }

    // 设置协程执行器状态
    void setState(TaskState state) { m_state.store(state); }

    // 添加协程到执行器
    void addFiber(Fiber::ptr fb);

    // 获取Scheduler实例
    static Scheduler* getScheduler();

protected:
    // 调度协程
    void dispatcherThread();

    // 调度器停止 运行, 并关闭所有执行器
    void stop();

protected:
    // 调度器名称
    std::string m_name;
    // 互斥锁
    MutexType m_mutex;
    // 执行器句柄
    std::vector<Processer*> m_processers;
    std::vector<Thread::ptr> m_procThreads;
    // 执行器中大致的协程数量
    std::vector<size_t> m_guessesFibers;
    // 调度器停止标识
    std::atomic<TaskState> m_state;
    // 执行器数量
    uint32_t m_processerNumber = 0;
    // 统计协程数量
    uint64_t m_fiberNumber = 0;
    // 上一次活跃的执行器数量
    volatile uint32_t m_lastActiveProcessers;
    // 调度线程句柄
    Thread::ptr m_dispatcherThread;
    Poller* m_poller;
    Thread::ptr m_threadPoller;
};

}  // namespace ppcode