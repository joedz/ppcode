#pragma once 

#include "fiber.h"
#include "timer.h"

#include "../wraplib/epoll.h"
namespace ppcode{

class Scheduler;


struct FdContext {
    typedef Mutex MutexType;
    enum Event {
        NONE = 0x0,
        READ = 0x1,
        WRITE = 0x4 
    };

    struct EventContext {
        /// 事件执行的调度器
        Scheduler* scheduler_ = nullptr;
        /// 事件的回调函数
        std::function<void()> cb_;
    };

    EventContext& getContext(Event event);

    void resetContext(EventContext& ctx);

    void triggerEvent(Event event);

    /// 读事件上下文
    EventContext read_;
    /// 写事件上下文
    EventContext write_;
    /// 事件关联的句柄
    int fd_ = 0;
    /// 当前的事件
    Event events_ = Event::NONE;
    /// 事件的Mutex
    MutexType mutex_;
    bool isListening = false;
};

class Poller : public Timer{
public:
    friend class FdContext;
    using RWMutexType = RWMutex;
    using ptr = std::shared_ptr<Poller>;

    Poller(Scheduler* sche);
    virtual ~Poller();
public:
    int addEvent(int fd, FdContext::Event event, std::function<void()> cb = nullptr);
    bool delEvent(int fd, FdContext::Event event);
    bool cancelEvent(int fd, FdContext::Event event);
    bool cancelAll(int fd);
    void networkPoller();

    TaskState getState() const { return m_state.load();}
    void setState(TaskState state)  { m_state.store(state);}

    void notify();
    bool stopping();
private:
    void resize(size_t size); 
protected:
    // 已经加锁 无需再加锁
    void onTimerInsertedAtFront() override;
private:
    Epoll m_epoll;
    int m_pipe[2];
    std::atomic<TaskState> m_state;
    std::atomic_size_t m_pendingEventCount = {0};
    RWMutexType m_mutex;
    std::vector<FdContext*> m_fdContexts;
    Scheduler* m_sche;
};





} // namespace ppcode