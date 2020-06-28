#pragma once

#include <functional>
#include <memory>

#include "../util/macro.h"
#include "context.h"



namespace ppcode {

enum class TaskState { reday, running, block, done };

class Processer;

class Fiber : public std::enable_shared_from_this<Fiber> {
public:
    using ptr = std::shared_ptr<Fiber>;

    // 创建协程  param:1 协程执行函数  param:2 栈大小
    Fiber(const std::function<void()>& fn, size_t stackSize = 16*1024);
    ~Fiber();

    INLINE void SwapIn() { m_ctx.SwapIn(); }

    INLINE void SwapOut() { m_ctx.SwapOut(); }

    INLINE Context& getContext() { return m_ctx; }

    INLINE uint64_t getFiberId() { return m_id; }

    INLINE TaskState& getFiberState() { return m_state; }

    INLINE Processer*& getProcesser() { return m_proc;}

    INLINE uint64_t& getYieldCount() { return m_yieldCount;}

private:

    void run();
    static void StaticRun(intptr_t vp);
    Fiber(const Fiber&) = delete;
    Fiber(Fiber&&) = delete;
    Fiber& operator=(const Fiber&) = delete;
    Fiber& operator=(Fiber&&) = delete;

private:
    uint64_t m_id;               // 协程id
    std::function<void()> m_fn;  // 协程绑定函数
    Context m_ctx;               // 协程上下文
    std::exception_ptr m_eptr;   // 异常处理
    uint64_t m_yieldCount;       // 协程切出次数
    TaskState m_state;           // 协程状态
    Processer* m_proc;
};

}  // namespace ppcode
