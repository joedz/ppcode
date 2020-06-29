#include "processer.h"
#include "../util/macro.h"
#include "../log.h"
#include "scheduler.h"
namespace ppcode {

static Logger::ptr g_logger = LOG_ROOT();
static thread_local Fiber* t_fiber = nullptr;

// 构造方法
Processer::Processer(Scheduler* sche, uint32_t id)
:m_id(id),
 m_sche(sche){
    LOG_DEBUG(g_logger) << "Processer is start, and num=" << m_id;
}

Processer::~Processer(){
    m_stopping = true;
    LOG_DEBUG(g_logger) << "Processer is end, and num=" << m_id
        << " but have fiber=" << m_fiberSize;

    ASSERT_BT(m_stopping);
}


// 协程切出
void Processer::toYield(){
    auto proc = getThis();
    proc->yield();
}

// 协程切出
void Processer::yield(){
    ++m_yieldCount;

    Fiber* fb = getCurrentFiber();

    fb->SwapOut();
}

// // 阻塞
// static void Suspend();
// // 阻塞
// void suspend();
// 获取执行器

Processer* & Processer::getThis(){
    static thread_local Processer* proc = nullptr;
    return proc; 
}

Fiber* Processer::getCurrentFiber(){
    return t_fiber;
}

// 添加协程
void Processer::addFiber(Fiber::ptr fiber){

    //MutexType::lock lock(m_mutex);
    //newQueue.push(fiber);

    //Fiber* fb = fiber.get();

    while(! newQueue.push(fiber)) {}
    ++m_fiberSize;

    if(!m_running) {
        this->notify();
    } else {
        m_notidied = true;
    }

}

// 添加多个协程  加锁
void Processer::addFibers(std::list<Fiber::ptr>& list){
    //MutexType::lock lock(m_mutex);

   //Fiber* fb;
   Fiber::ptr fb;

    m_fiberSize += list.size();

    while(list.empty()) {
        fb = list.front();
        list.pop_front();
        while(!newQueue.push(fb));
    }

    if(!m_running) {
        this->notify();
    } else {
        m_notidied = true;
    }
}

// 调度器执行使执行器执行
void Processer::start(){
    // 线程执行这个函数
    execute();
}

//  调度器通知等待的执行器
void Processer::notify(){
//  MutexType::Lock lock(m_mutex);
//  m_cv.notify_all();
}

// 执行器执行
void Processer::execute(){
    getThis() = this;
    LOG_ERROR(g_logger) << "product num=" << m_id << " is execute";

    while(!m_sche->isStopping()) {
        Fiber::ptr fb;
        runableQueue.pop(fb);
        
        //LOG_ERROR(g_logger) << t_fiber->getFiberId() << std::endl;
        if(!fb) {
            getNewFibers();
            continue;
            //runableQueue.pop(t_fiber);
            if(!fb){
                waitting();
            }
        }

        while(fb && !m_sche->isStopping()) {
            fb->getFiberState() = TaskState::running;
            fb->getProcesser() = this;

            ++m_swithCount;
            fb->SwapIn();

            switch (fb->getFiberState())
            {
            case TaskState::running:
                
                while(!runableQueue.push(fb));

                if(m_fiberSize > 32) {
                    getNewFibers();
                }
                break;
            case TaskState::block:
                // TODO 
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
                break;
            }
            fb = nullptr;
        }
    }
}

// gc回收
void Processer::gc(){
    //Fiber::ptr fb;
    // 清楚
    while(!gcQueue.empty()){
       // fb = gcQueue.front();
        gcQueue.pop_front();
        //delete fb;
    }
}

// 执行器添加新的协程
void Processer::getNewFibers(){
    
    //uint64_t size = m_fiberSize;
    unsigned long size = 0;
    // 获取新加入的协程个数
    do {
        size = m_fiberSize;
        // 当前的 值等于 协程的个数 ,之后将这个数值设置成0
        if(m_fiberSize.compare_exchange_strong(size, 0, std::memory_order_acquire)){
            break;
        }
    }while(1);
    m_nowFiberSize += size;
    // 从newQueue队列中取出size个协程数, 因为在其他线程现实当前协程个数为0, 所以其他线程只能添加协程, 不能steal协程
    //Fiber *fb;
    Fiber::ptr fb;
    for(unsigned long i = 0; i < size; ++i) {
        while(newQueue.pop(fb)){    // 从 newQueue队列中获取 协程 成功
            while(!runableQueue.push(fb)); // 加入到run队列
            break;
        }
    }
}

// 获取当前调度器
void Processer::getScheduler(){
    // UNDO
}

// 执行器等待时的方法
void Processer::waitting(){
    this->gc();

    MutexType::Lock lock(m_mutex);
    LOG_ERROR(g_logger) << "product num=" << m_id << " is waitting";

    if(m_notidied) {
        m_notidied = false;
        return;
    }

    m_running = false;
    m_cv.wait(m_mutex);
    m_running = true;
}

}