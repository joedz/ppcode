#include "timer.h"

#include <functional>

#include "../log.h"

namespace ppcode {

static Logger::ptr g_logger = LOG_ROOT();

TimerTask::TimerTask(std::function<void()> cb, uint64_t ms, Timer* timer,
                     bool recurring)
    : m_cb(cb), m_ms(ms), m_recurring(recurring), m_timer(timer) {
    m_next = GetCurrentMS() + m_ms;
}

//  取消定时器
bool TimerTask::cancel() {
    Timer::RWMutexType::WriteLock lock(m_timer->m_mutex);

    if (m_cb) {
        m_cb = nullptr;
        auto it = m_timer->m_tasks.find(shared_from_this());
        m_timer->m_tasks.erase(it);
        return true;
    }
    return false;
}

// 刷新定时器
bool TimerTask::refresh() {
    Timer::RWMutexType::WriteLock lock(m_timer->m_mutex);
    if (!m_cb) {
        return false;
    }

    // 定时器中找到自己
    auto it = m_timer->m_tasks.find(shared_from_this());
    if (it == m_timer->m_tasks.end()) {
        return false;
    }
    // 删除自己
    m_timer->m_tasks.erase(it);
    m_next = GetCurrentMS() + m_ms;  // 更改下次触发事件, 并加入定时器
    m_timer->m_tasks.insert(shared_from_this());
    return true;
}

bool TimerTask::reset(uint64_t ms, bool from_now) {
    if (ms == m_ms && !from_now) {
        return true;
    }

    Timer::RWMutexType::WriteLock lock(m_timer->m_mutex);
    if (!m_cb) {
        return false;
    }

    auto it = m_timer->m_tasks.find(shared_from_this());
    if (it == m_timer->m_tasks.end()) {
        return false;
    }
    m_timer->m_tasks.erase(it);
    uint64_t start = 0;
    if (from_now) {
        start = GetCurrentMS();
    } else {
        start = m_next - ms;
    }
    m_ms = ms;
    m_next = start + m_ms;
    m_timer->addTimerTask(shared_from_this(), lock);
    return true;
}

bool TimerTask::Comparator::operator()(const TimerTask::ptr& lhs,
                                       const TimerTask::ptr& rhs) const {
    if (lhs->m_next < rhs->m_next) {
        return true;
    } else if (lhs->m_next > rhs->m_next) {
        return false;
    } else {
        return lhs.get() < rhs.get();
    }
}

TimerTask::ptr Timer::createTimer(const std::function<void()>& cb, uint64_t ms,
                                  bool recurring) {
    TimerTask::ptr task = std::make_shared<TimerTask>(cb, ms, this, recurring);
    RWMutexType::WriteLock lock(m_mutex);
    addTimerTask(task, lock);
    return task;
}

static void TaskCond(std::weak_ptr<void> weak_cond, std::function<void()> cb) {
    std::shared_ptr<void> tmp = weak_cond.lock();
    if (tmp) {
        cb();
    }
}

TimerTask::ptr Timer::createCondTimer(const std::function<void()>& cb,
                                      std::weak_ptr<void> weak_cond,
                                      uint64_t ms, bool recurring) {
    return createTimer(std::bind(&TaskCond, weak_cond, cb), ms, recurring);
}

uint64_t Timer::getNextExecuteTime() {
    RWMutexType::ReadLock lock(m_mutex);
    m_tickled = false;
    if (m_tasks.empty()) {
        return ~0ull;
    }

    const TimerTask::ptr& next = *m_tasks.begin();
    uint64_t now_ms = GetCurrentMS();
    if (now_ms >= next->m_next) {
        return 0;
    } else {
        return next->m_next - now_ms;
    }
}

bool Timer::hasTimer() {
    RWMutexType::ReadLock lock(m_mutex);
    return !m_tasks.empty();
}

void Timer::addTimerTask(TimerTask::ptr task, RWMutexType::WriteLock& lock) {
    bool isFront;

    auto it = m_tasks.insert(task).first;
    isFront = (it == m_tasks.begin()) && !m_tickled;
    if (isFront) {
        m_tickled = true;
    }

    lock.unlock();

    if (isFront) {
        onTimerInsertedAtFront();
    }
}

bool Timer::detectClockRollover(uint64_t now_ms) {
    bool rollover = false;
    if (now_ms < m_previouseTime &&
        now_ms < (m_previouseTime - 60 * 60 * 1000)) {
        rollover = true;
    }
    m_previouseTime = now_ms;
    return rollover;
}

void Timer::getExecuteTask(std::vector<std::function<void()>>& cbs) {
    uint64_t now_ms = GetCurrentMS();
    std::vector<TimerTask::ptr> expired;
    {
        RWMutexType::ReadLock lock(m_mutex);
        if (m_tasks.empty()) {
            return;
        }
    }
    RWMutexType::WriteLock lock(m_mutex);
    if (m_tasks.empty()) {
        return;
    }
    // 判断时间是否被更改
    bool rollover = detectClockRollover(now_ms);
    if (!rollover && (*m_tasks.begin())->m_next > now_ms) {
        // 如果时间没有被更改, 下一个定时器任务的时间比当前时间大,没有超时任务
        return;
    }

    TimerTask::ptr now_task(new TimerTask(nullptr, 0, nullptr));

    // 获取所有超时的定时器任务
    auto it = rollover ? m_tasks.end() : m_tasks.lower_bound(now_task);

    // LOG_INFO(g_logger) << (*it)->m_ms << (*it)->m_next;
    // 如果存在有定时器任务等于现在的时间,那么也将任务加入定时器中
    while (it != m_tasks.end() && (*it)->m_next == now_ms) {
        ++it;
    }
    expired.insert(expired.begin(), m_tasks.begin(), it);
    m_tasks.erase(m_tasks.begin(), it);

    cbs.reserve(expired.size());

    for (auto& task : expired) {
        cbs.push_back(task->m_cb);
        if (task->m_recurring) {
            // 将循环任务加入定时器中
            task->m_next = now_ms + task->m_ms;
            m_tasks.insert(task);
        } else {
            task->m_cb = nullptr;
        }
    }
}

}  // namespace ppcode