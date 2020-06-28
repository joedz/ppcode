#pragma once
#include <boost/context/all.hpp>
#include <functional>
#include <memory>
#include "../util/macro.h"


namespace ppcode {

using Fn_t = void (*)(intptr_t);
using fcontext_t = boost::context::fcontext_t;
using intptr_t = long;


class AllocMemory{
public:
    static void *alloc(size_t size) {
        return ::malloc(size);
    }
    static void free(void *ptr, size_t size){
        return ::free(ptr);
    }
};

// 对协程原接口进行封装
class Context {
public:
INLINE Context(Fn_t fn, intptr_t vp, size_t stacksize)
    :m_fn(fn),
    m_vp(vp),
    m_stackSize(stacksize){

    m_stack = (char *)AllocMemory::alloc(m_stackSize);
    m_ctx = boost::context::make_fcontext(m_stack + m_stackSize, m_stackSize, m_fn);

    // totdo protectpage
}

INLINE ~Context(){
    if(m_stack) {
        AllocMemory::free(m_stack, 0);
        m_stack = nullptr;
    }
}

INLINE void SwapIn(){
     boost::context::jump_fcontext(&GetTlsContext(), m_ctx, m_vp, false);
}

INLINE void SwapIn(Context& other){
     boost::context::jump_fcontext(other.m_ctx, m_ctx, m_vp, false);
}

INLINE void SwapOut(){
     boost::context::jump_fcontext(m_ctx, &GetTlsContext(), m_vp, false);
}

INLINE void SwapOut(Context& other){
     boost::context::jump_fcontext(m_ctx, other.m_ctx, other.m_vp, false);
}

INLINE static fcontext_t& GetTlsContext(){
    static thread_local fcontext_t tls_context;
    return tls_context;
}

private:
    fcontext_t*             m_ctx;              // 协程上下文 采用boost
    Fn_t                    m_fn;               // 运行函数
    intptr_t                m_vp;               // 当前上下文属于的协程 Task 对象指针
    char*                   m_stack = nullptr;  // 栈空间
    uint32_t                m_stackSize = 0;    // 栈空间大小

};


}  // namespace ppcode