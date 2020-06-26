#include "fiber.h"

#include "../log.h"

namespace ppcode {


static Logger::ptr g_logger = LOG_ROOT();


static thread_local std::atomic_uint64_t s_fiber_id{0};
static thread_local std::atomic_uint64_t s_fiber_count{0};

static thread_local Fiber* t_fiber = nullptr;
static thread_local Fiber::ptr t_main_fiber = nullptr;


Fiber::Fiber(const std::function<void()>& fn, size_t stackSize)
    : m_ctx(&Fiber::StaticRun, (intptr_t)this, stackSize),
      m_fn(fn),
      m_id(++s_fiber_id),
      m_state(TaskState::reday) {
          ++s_fiber_count;
      }

Fiber::~Fiber() { ASSERT_BT(this->m_state == TaskState::done); }

void Fiber::run() {

    ASSERT_BT(this->m_state == TaskState::running);
    LOG_DEBUG(g_logger) << "the fiber is running";

    try{
        m_fn();
    } catch(...) {
        LOG_ERROR(g_logger) << "An exception was caught";
        m_eptr = std::current_exception();
    }

    m_state = TaskState::done;
    // 将协程切出或者删除
}

void Fiber::StaticRun(intptr_t vp) {
    Fiber* tfiber = (Fiber*)vp;
    tfiber->run();
}

}  // namespace ppcode