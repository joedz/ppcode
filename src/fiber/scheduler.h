#pragma once 

#include <memory>
#include <functional>
#include <string>
#include <vector>

#include "processer.h"
#include "../util/singleton.h"
#include "../thread.h"

namespace ppcode {

// 协程调度器
class Scheduler {
public:
    using ptr = std::shared_ptr<Scheduler>;
    using MutexType = Mutex;
    // 协程构造函数 单例
    Scheduler();
    // 析构函数 停止所有子线程
    ~Scheduler();

    // 创建协程的接口    限制只能从这里创建协程
    void createFiber(const std::function<void()>& cb);
    // 调度器执行 创建协程执行器  
    void start(size_t threads = 0, const std::string& name = "scheduler");
    // 调度器停止 运行, 并关闭所有执行器
    void stop();
    // 调度器是否已经停止
    bool isStopping() const { return m_stopping;}

private:

    // 添加协程到执行器
    void addFiber(Fiber::ptr fb);
    // 调度协程 
    void dispatcherThread();
    // 创建协程执行器
    void createProcesser(size_t id);

private:
    // 调度器名称
    std::string m_name;
    // 互斥锁
    MutexType m_mutex;
    // 执行器句柄
    std::vector<Processer::ptr> m_processers;
    // 执行器中大致的协程数量
    std::vector<size_t> m_guessesFibers;
    // 调度器停止标识
    std::atomic_bool m_stopping;
    bool m_isStart;
    // 执行器数量
    uint32_t m_processerNumber;
    // 统计协程数量
    uint64_t m_fiberNumber;
    // 上一次活跃的执行器数量
    volatile uint32_t m_lastActiveProcessers;
    // 调度线程句柄
    Thread::ptr m_dispatcherThread;
    // 定时器线程
    Thread::ptr m_timer;
};


}