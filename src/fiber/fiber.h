#pragma once
#include <functional>
#include <memory>
#include "../util/macro.h"
#include "fcontext.h"

namespace ppcode {

// 工作状态
enum class TaskState { 
    init,        // 初始状态
    running,     // 运行状态
    block,       // 阻塞状态
    idle,        // 空闲状态
    stop,       // 停止状态
    done,        // 结束状态
    execpt      // 异常状态
};

std::string ToStringTaskState(TaskState state);



/*
协程状态 
    可运行      协程被创建或者正在执行 处于可运行状态   
    被阻塞      协程陷入io操作,主动陷入此状态   
    死亡        协程在停止后进入此状态
*/

/*
协程执行器状态 
    初始状态    // 没有执行execute方法
    运行状态    // 执行器正在运行 
    阻塞状态    // 其他执行器可窃取本执行器协程
    idle状态    // 空闲状态 可优先获取协程  此时等待条件变量 并且此时可以去窃取其他执行器的协程 
    stop        // 停止状态
*/

/*
调度器状态 IO协程调度状态 
    初始状态    // Scheduler创建时状态 此时程序处于初始状态
    运行状态    // 开启了执行器后处于的状态
    停止状态    // 调度器正在结束其他的执行器
    结束状态    // 此时调度器真正结束并且程序退出
*/

/*
IO协程调度 定时器特殊状态
    idle状态  空闲状态 此时处于epoll_wait状态
*/



using Fn_t = void (*)(intptr_t);
class AllocMemory{
public:
    static void *alloc(size_t size) {
        return ::malloc(size);
    }
    static void free(void *ptr, size_t size){
        return ::free(ptr);
    }
};

class Processer;
// 
class Fiber : public std::enable_shared_from_this<Fiber> {
public:
    using ptr = std::shared_ptr<Fiber>;

    // 创建协程  param:1 协程执行函数  param:2 栈大小
    Fiber(const std::function<void()>& fn, size_t stackSize = 16*1024);
    ~Fiber();

    // 切入协程执行
    INLINE void SwapIn() { 
        jump_fcontext(&GetTlsContext(), m_ctx, m_vp);
    }

    // 切出协程执行
    INLINE void SwapOut() { 
        jump_fcontext(&m_ctx, GetTlsContext(), m_vp);
    }

    INLINE fcontext_t& GetTlsContext(){
        static thread_local fcontext_t tls_context;
        return tls_context;
    }

    INLINE uint64_t getFiberId() { return m_id; }

    INLINE TaskState& getFiberState() { return m_state; }

    //INLINE Processer*& getProcesser() { return m_proc;}

    INLINE uint64_t& getYieldCount() { return m_yieldCount;}

    INLINE Processer*& getProcesser() { return m_proc;}

    INLINE std::exception_ptr getExcetionPtr() { return m_eptr;}

private:

    void run();
    static void StaticRun(intptr_t vp);

    Fiber(const Fiber&) = delete;
    Fiber(Fiber&&) = delete;
    Fiber& operator=(const Fiber&) = delete;
    Fiber& operator=(Fiber&&) = delete;

private:
    std::function<void()>   m_fn;  // 协程绑定函数
    intptr_t                m_vp;               // 当前上下文属于的协程 Task 对象指针
    uint32_t                m_stackSize;    // 栈空间大小
    fcontext_t              m_ctx;              // 协程上下文 采用boost
    char*                   m_stack = nullptr;  // 栈空间
    TaskState m_state;           // 协程状态
    uint64_t m_id;               // 协程id
    uint64_t m_yieldCount = 0;       // 协程切出次数
    Processer* m_proc = nullptr;           // 协程执行器
     std::exception_ptr m_eptr = nullptr;   // 异常处理
};

}  // namespace ppcode
