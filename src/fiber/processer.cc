#include "hook.h"
#include "scheduler.h"
#include "processer.h"
#include "../util/macro.h"
#include "../log.h"


namespace ppcode {

static Logger::ptr g_logger = LOG_ROOT();
//static thread_local Fiber* t_fiber = nullptr;
static thread_local Fiber::ptr t_fiber;

// 构造方法
Processer::Processer(Scheduler* sche, uint32_t id)
: m_id(id),
m_sche(sche)
{
   // LOG_DEBUG(g_logger) << "Processer is start, and num=" << m_id;
    m_state = TaskState::init;
}

Processer::~Processer(){
    LOG_DEBUG(g_logger) << "Processer is end, and num=" << m_id
       << " .a total of " <<  m_FiberCount << " fibers have been exectued. "
       << m_swithCount << " times have been switched, " 
       << m_yieldCount  << " times have been blocked, and "
       << m_newfiberCount << " fibers have not been executed." << m_nowFiberSize;
    m_state = TaskState::stop;
}

// 协程切出
void Processer::toYield(){
    auto proc = getThis();
    proc->yield();
}

// 协程切出
void Processer::yield(){
    ++m_yieldCount;
    Fiber::ptr fb = getCurrentFiber();
    fb->getFiberState() = TaskState::block;
    fb->SwapOut();
}

// 获取协程执行器自己
Processer* & Processer::getThis(){
    static thread_local Processer* proc = nullptr;
    return proc; 
}

// 获取当前正在执行的协程
Fiber::ptr Processer::getCurrentFiber(){
    return t_fiber;
}

// 添加协程
void Processer::addFiber(Fiber::ptr fiber){

    //MutexType::lock lock(m_mutex);
    //newQueue.push(fiber);
    //Fiber* fb = fiber.get();

    if(m_state == TaskState::stop ||
        m_state == TaskState::done) {
        return;        
    }

    // 以无锁的方式加入协程到协程执行队列
    while(! newQueue.push(fiber)) {}

    m_newfiberCount.fetch_add(1);

    // 通知执行器
    if(!isRunning()) {
        m_notidied = true;
        this->notify();
    } else {
        m_notidied = true;
    }
}

// 添加多个协程 
void Processer::addFibers(std::list<Fiber::ptr>& list){
    Fiber::ptr fb;
    
    // 以无锁的方式添加多个协程到执行队列中
    while(list.empty()) {
        fb = list.front();
        list.pop_front();
        while(!newQueue.push(fb));
    }

    m_newfiberCount.fetch_add(list.size());

    // 通知执行器
    if(!isRunning()) {
        
        this->notify();
    } else {
        m_notidied = true;
    }
}

// 调度器通知等待的执行器
void Processer::notify(){
    MutexType::Lock lock(m_mutex);
    m_notidied = true;
    m_cv.notify_all();
}

// 执行器执行
void Processer::execute(){
    getThis() = this;

    //LOG_ERROR(g_logger) << "product num=" << m_id << " is execute";
    set_hook_enable(true);
    m_state = TaskState::running;

    while(!m_sche->isStopping()) {
        Fiber::ptr fb;

        if(runableQueue.size() > 0) {
            fb = runableQueue.front();
            runableQueue.pop_front();
        }
        
        if(!fb) {
            getNewFibers();
            
            if( runableQueue.size() <= 0 && !fb){
                idle(); // 协程进入 空闲状态
            }
            continue;
        }

        while(fb && !m_sche->isStopping()) {
            t_fiber = fb;
            //LOG_ERROR(g_logger) << t_fiber->getFiberId() << std::endl;

            fb->getFiberState() = TaskState::running;
            fb->getProcesser() = this;

            ++m_swithCount;
            fb->SwapIn();

            switch (fb->getFiberState()) {
            case TaskState::running:
                
                //while(!runableQueue.push(fb));
                runableQueue.push_back(fb);
                if(m_newfiberCount > 32) {
                    getNewFibers();
                }
                break;
            case TaskState::block:
                break;
            case TaskState::done:
            default:
                if(gcQueue.size() > 32) {
                    gc();
                }
                gcQueue.push_back(fb);
                if(fb->getExcetionPtr()) {
                    std::exception_ptr ep = fb->getExcetionPtr();
                    std::rethrow_exception(ep);
                }
                ++m_FiberCount;
                break;
            }
            fb = nullptr;
        }
    }
}

// gc回收
void Processer::gc(){
    // 统一将协程在这里销毁
    while(!gcQueue.empty()){
        gcQueue.pop_front();
    }
}

// 从新加入协程的队列中添加新的协程到可执行队列
void Processer::getNewFibers(){
    unsigned long size = 0;

    // 获取新加入的协程个数  因为可能会有 race condition 所以需要原子的获取newQueue队列中的协程数量
    do {
        size = m_newfiberCount;
        // 当前的值等于协程的个数 ,之后将这个数值设置成0
        if(m_newfiberCount.compare_exchange_strong(size, 0, std::memory_order_acquire)){
            break;
        }
    }while(1);

    // 现在的协程数量 + size
    m_nowFiberSize += size;

    // 扩展TODO  从newQueue队列中取出size个协程数, 因为在其他执行器看这个Processer当前协程个数为0, 所以其他线程只能添加协程, 不能steal协程

    Fiber::ptr fb;
    for(unsigned long i = 0; i < size; ++i) {
        while(newQueue.pop(fb)){    // 从 newQueue队列中获取 协程 成功
            //while(!runableQueue.push(fb)); // 加入到run队列
            runableQueue.push_back(fb);
            break;
        }
    }
}

// 获取当前调度器
Scheduler* Processer::getScheduler(){
    return m_sche;
}

// 执行器等待时的方法
void Processer::idle(){
    this->gc();

    // steal TODO  从其他协程执行器偷取线程 

    MutexType::Lock lock(m_mutex);
    LOG_ERROR(g_logger) << "product num=" << m_id << " is waitting";

    if(m_notidied) {
        // 多去检测以下有没有新的连接, 不要老是等待
        m_notidied = false;
        return;
    }

    m_state = TaskState::idle;
    
    while(!m_notidied) {
        m_cv.wait(m_mutex);
    }
    //m_notidied = false;
    m_state = TaskState::running;

}

}