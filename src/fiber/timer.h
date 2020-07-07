#pragma once 

#include <memory>
#include <vector>
#include <set>
#include <list>
#include "../thread.h"
#include "../util/util.h"


namespace ppcode {
class Timer;
class Fiber;

class TimerTask : public std::enable_shared_from_this<TimerTask> {
public:
    friend class Timer;
    using ptr = std::shared_ptr<TimerTask>;
    TimerTask(std::function<void()> cb, uint64_t ms, Timer* timer, bool recurring = false);
    //TimerTask(std::shared_ptr<Fiber> fb, uint64_t ms, Timer* timer);
    // 取消定时器
    bool cancel();
    // 刷新定时器执行间隔时间
    bool refresh();

    bool reset(uint64_t ms, bool from_now);
    ~TimerTask() = default;

private:
    std::function<void()> m_cb;
    //std::weak_ptr<Fiber> m_fb;
    uint64_t m_ms = 0;
    uint64_t m_next =0;
    bool m_recurring = false;
    Timer* m_timer;
private:
    struct Comparator{
        bool operator()(const TimerTask::ptr& lhs, const TimerTask::ptr& rhs) const;
    };
};

class Timer {
public:
    friend class TimerTask;

    using RWMutexType = RWMutex;
    Timer() {
         m_previouseTime = GetCurrentMS();
    }
    virtual ~Timer() = default;

    TimerTask::ptr createTimer(const std::function<void()>& cb, uint64_t ms, bool recurring = false);
    TimerTask::ptr createCondTimer(const std::function<void()>& cb, std::weak_ptr<void> weak_cond, uint64_t ms, bool recurring = false);

    uint64_t getNextExecuteTime();
    void getExecuteTask(std::vector<std::function<void()> >&  cbs);

    bool hasTimer();
protected:

    virtual void onTimerInsertedAtFront() = 0;
    void addTimerTask(TimerTask::ptr task, RWMutexType::WriteLock& lock);
    bool detectClockRollover(uint64_t now_ms);

protected:
    RWMutexType m_mutex;
    std::set<TimerTask::ptr, TimerTask::Comparator> m_tasks;
    bool m_tickled = false;
    uint64_t m_previouseTime = 0;
};


}