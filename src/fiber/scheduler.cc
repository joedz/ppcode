#include "scheduler.h"
#include "processer.h"
#include "fiber.h"
#include "../log.h"


namespace ppcode {
static Logger::ptr g_logger = LOG_ROOT();


// 创建协程的接口    限制只能从这里创建协程
void Scheduler::createFiber(const std::function<void()>& cb){
    Fiber::ptr fb(new Fiber(cb));

    fb->getFiberState() = TaskState::reday;

    ++m_fiberNumber;
    addFiber(fb);
}

// 调度器执行 创建协程执行器  
void Scheduler::start(size_t threads, const std::string& name){

    ASSERT_BT2(!m_isStart, "The scheduler cannot be started repeatedly");
    m_isStart = true;
    LOG_DEBUG(g_logger) << "The scheduler is starting. Threads=" << threads ;
    // 当指定的协程数小于1时, 创建数量等于cpu数量
    if(threads < 1) {
        threads = Thread::hardware_concurrency();
    }

    m_name = name;
    m_processerNumber = threads;

    Thread::SetThreadName(m_name);

    for(size_t i = 0; i < m_processerNumber; ++i) {
        createProcesser(i);
    }
}

// 调度器停止 运行, 并关闭所有执行器
void Scheduler::stop(){
    //ASSERT_BT2(!m_stopping, "The scheduler cannot be turned off repeatedly");
    if(m_stopping) {
        return;
    }

    // 这里的线程数不一定等于 m_processerNumber, 也许以后会有执行器被删除
    size_t n = m_processers.size();
    
    for(size_t i = 0; i < n; ++i) {
        auto p = m_processers[i];
        if(p) {
            // 唤醒执行器 执行stop方法
            p->notify();
        }
    } 

    // 关闭其他线程  调度器  定时器  TODO
}

// 协程构造函数 单例
Scheduler::Scheduler(){
    LOG_DEBUG(g_logger) << "The Scheduler is being created";
}

// 析构函数 停止所有子线程
Scheduler::~Scheduler(){
    m_stopping = true;
    stop();
}
// 添加协程到执行器
void Scheduler::addFiber(Fiber::ptr fb){

    auto proc = fb->getProcesser();

    if(proc && proc->isRunning()) {
        proc->addFiber(fb);
    }

    auto it = std::min(m_guessesFibers.begin(), m_guessesFibers.end());

    size_t index = it - m_guessesFibers.begin();

    auto procSptr = m_processers[index];
    procSptr->addFiber(fb);

    ++(*it);
}

// 调度协程 
void Scheduler::dispatcherThread(){

}
// 创建协程执行器
void Scheduler::createProcesser(size_t id){
    
    Processer::ptr proc = std::make_shared<Processer>(this, id);

    const std::string thrName = m_name + "_" + std::to_string(id);

    Thread t(thrName, std::bind(&Processer::execute, proc.get()));
    t.detach();
    m_processers.push_back(proc);
}


}