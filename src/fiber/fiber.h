#pragma once 

#include <memory>
#include <functional>
#include "context.h"
#include "../util/macro.h"

namespace ppcode{


enum class TaskState{
    reday,
    running,
    block,
    done
};

class Fiber : public std::enable_shared_from_this<Fiber>{
public:
    using ptr = std::shared_ptr<Fiber>;

    // 创建协程  param:1 协程执行函数  param:2 栈大小
    Fiber(const std::function<void()>& fn, size_t stackSize);
    ~Fiber();


    INLINE void SwapIn(){
        m_ctx.SwapIn();
    }
    INLINE void SwapOut(){
        m_ctx.SwapOut();
    }

private:

    void run();
    static void StaticRun(intptr_t vp);
    
private:
    uint64_t                m_id;           // 协程id
    std::function<void()>   m_fn;           // 协程绑定函数
    Context                 m_ctx;          // 协程上下文
    std::exception_ptr      m_eptr;         // 异常处理
    uint64_t                m_yieldCount;   // 协程切出次数
    TaskState               m_state;        // 协程状态
};


} // namespace ppcode
