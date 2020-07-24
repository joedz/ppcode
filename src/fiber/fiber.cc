#include "fiber.h"

#include "../log.h"

namespace ppcode {

static Logger::ptr g_logger = LOG_ROOT();

static thread_local std::atomic_uint64_t s_fiber_id{0};
static thread_local std::atomic_uint64_t s_fiber_count{0};

std::string ToStringTaskState(TaskState state) {
    switch (state) {
#define XX(state)          \
    case TaskState::state: \
        return #state;     \
        break;

        XX(init)
        XX(running)
        XX(block)
        XX(idle)
        XX(stop)
        XX(done)
        XX(execpt)
        default:
            return "unkown";
            break;
    }
    return "unkown";
#undef XX
}

// static thread_local Fiber* t_fiber = nullptr;
// static thread_local Fiber::ptr t_main_fiber = nullptr;

Fiber::Fiber(const std::function<void()>& fn, size_t stackSize)
    : m_fn(fn),
      m_vp((intptr_t)this),
      m_stackSize(stackSize),
      m_state(TaskState::running) {
    m_stack = (char*)AllocMemory::alloc(m_stackSize);
    m_ctx = make_fcontext(m_stack + m_stackSize, m_stackSize, StaticRun);
    ++s_fiber_count;
    m_id = ++s_fiber_id;
    // LOG_DEBUG(g_logger) << "create a fiber";
}

Fiber::~Fiber() {
    // LOG_DEBUG(g_logger) << "the fiber is done";
    if (m_stack) {
        AllocMemory::free(m_stack, 0);
        m_stack = nullptr;
    }
    // ASSERT_BT(this->m_state == TaskState::done);
    --s_fiber_count;
}

void Fiber::run() {
    ASSERT_BT(this->m_state == TaskState::running);
    //    LOG_DEBUG(g_logger) << "the fiber is running";

    try {
        m_fn();
    } catch (...) {
        LOG_ERROR(g_logger) << "An exception was caught";
      //  m_eptr = std::current_exception();
    }

    m_state = TaskState::done;
    // 将协程切出或者删除
    SwapOut();
}

void Fiber::StaticRun(intptr_t vp) {
    Fiber* tfiber = (Fiber*)vp;
    tfiber->run();
}

}  // namespace ppcode