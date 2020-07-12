#include "scheduler.h"

#include "../log.h"
#include "fiber.h"

namespace ppcode {
static Logger::ptr g_logger = LOG_ROOT();

static Scheduler::ptr t_scheduler = nullptr;

Scheduler* Scheduler::getScheduler() {
    if (t_scheduler) {
        return t_scheduler.get();
    }

    t_scheduler = std::make_shared<Scheduler>();
    return t_scheduler.get();
}

// 协程构造函数 单例
Scheduler::Scheduler(size_t threads, const std::string& name) : m_name(name) {
    m_state = TaskState::init;
    LOG_DEBUG(g_logger) << "The Scheduler is being created";

    if (threads < 1) {
        threads = Thread::hardware_concurrency();
    }
    m_processerNumber = threads;
}

// 调度器执行 创建协程执行器
void Scheduler::start() {
    ASSERT_BT(m_state == TaskState::init);
    // set_hook_enable(true);
    Thread::SetThreadName(m_name);

    for (size_t i = 0; i < m_processerNumber; ++i) {
        Processer* proc = new Processer(this, i);
        const std::string thrName = m_name + "_" + std::to_string(i);
        m_procThreads.push_back(std::make_shared<Thread>(
            thrName, std::bind(&Processer::execute, proc)));
        m_processers.push_back(proc);
    }

    m_poller = new Poller(this);
    m_threadPoller = std::make_shared<Thread>(
        "Network poller", std::bind(&Poller::networkPoller, m_poller));
    m_state = TaskState::running;
}

// 创建协程的接口    限制只能从这里创建协程
void Scheduler::createFiber(const std::function<void()>& cb) {
    Fiber::ptr fb(new Fiber(cb));

    ++m_fiberNumber;
    addFiber(fb);
}

// 添加协程到执行器
void Scheduler::addFiber(Fiber::ptr fb) {
    // 查看协程之前是否有P 如果有那么将协程加入到P的执行队列  当前P处于活跃状态
    // 那么将很快执行这个协程
    auto proc = fb->getProcesser();

    if (proc && proc->isRunning()) {
        proc->addFiber(fb);
        return;
    }

    // 找到一个空闲的执行器  找到一个空闲协程,并将其唤醒
    for (auto& it : m_processers) {
        if (it->isWaitting()) {
            it->addFiber(fb);
            return;
        }
    }

    // 轮询将协程加入执行器  并期待找到的协程执行器处于活跃状态
    static size_t id = 0;
    size_t n = m_processers.size();

    for (size_t i = 0; i < n; ++i) {
        auto& procSptr = m_processers[id];

        // 如果当前协程处于阻塞状态 那么暂时放弃将协程加入
        if (!procSptr->isBlock()) {
            procSptr->addFiber(fb);
            id = (id + 1) % n;
            return;
        }
        id = (id + 1) % n;
    }

    // TODO 如果说当前执行器都出与阻塞状态,
    // 那么向阻塞的执行器中加入协程,协程将不能直接运行,是否可以先将协程加入到一个全局队列里面
    auto& procSptr = m_processers[id];
    procSptr->addFiber(fb);
    id = (id + 1) % n;
}

// 调度协程
void Scheduler::dispatcherThread() {}

// 析构函数 停止所有子线程
Scheduler::~Scheduler() {
    if (m_state != TaskState::done) {
        m_state = TaskState::stop;
        stop();
    }
    LOG_DEBUG(g_logger) << "The scheduler is finished. There are "
                        << m_fiberNumber << " coroutines created";
}

// 调度器停止 运行, 并关闭所有执行器
void Scheduler::stop() {
    // ASSERT_BT2(!m_stopping, "The scheduler cannot be turned off repeatedly");

    // 这里的线程数不一定等于 m_processerNumber, 也许以后会有执行器被删除
    size_t n = m_processers.size();
    for (size_t i = 0; i < n; ++i) {
        auto p = m_processers[i];
        if (p) {
            // 唤醒执行器 执行stop方法
            p->setState(TaskState::stop);
            p->notify();
        }
    }

    // 等待协程执行器线程结束
    for (auto& thr : m_procThreads) {
        if (thr->joinable()) {
            thr->join();
        }
    }

    if (!m_poller->stopping()) {
        m_poller->setState(TaskState::stop);
    }
    m_poller->notify();

    // 等待网路轮询器线程结束
    if (m_threadPoller->joinable()) m_threadPoller->join();

    for (size_t i = 0; i < m_processers.size(); ++i) {
        Processer* p = m_processers[i];
        if (p) {
            delete p;
        }
    }

    delete m_poller;
    m_state = TaskState::done;
}

}  // namespace ppcode