#include "processer.h"

#include "../log.h"

#include "scheduler.h"


namespace ppcode {

Logger::ptr g_logger = LOG_ROOT();


static thread_local Fiber* t_fiber = nullptr;
static thread_local Processer* t_proc = nullptr;

  Fiber* Processer::GetCuttentFiber(){
      return t_fiber;
  }
  Processer* Processer::GetCurrentProcesser(){
      return t_proc;
  }

  Scheduler* Processer::GetScheduler(){
      return t_proc->m_sche;
  }

  void Processer::ToYield(){
    auto fiber = GetCuttentFiber();
    ++(fiber->getYieldCount());
    fiber->SwapOut();
  }

  bool Processer::Wakeup(){
    return false;
  }


Processer::Processer(Scheduler* sche, size_t num, const std::string& name)
:m_sche(sche),
m_name(name),
m_id(num),
m_switchCount(0)
{
    LOG_DEBUG(g_logger) << "processer is start " << num << " " << name;

}


void Processer::Process(){
    t_proc = this;
    static int gcTimes = 0;
    LOG_DEBUG(g_logger) << "processer is processer " << m_id << " " << m_name;
    while(!(m_sche->isStop())){

        runnableQueue.pop(t_fiber);

        if(!t_fiber) {
            waitCondition();
            continue;
        }

        if(t_fiber->getFiberState() != TaskState::reday) {
            if(t_fiber->getFiberState() == TaskState::done) {
                LOG_ERROR(g_logger) << "err, A coroutine ends but reappears in the run queue"
                    << t_fiber->getFiberId() ;
            } else {

            }
        } 

        while(t_fiber && !m_sche->isStop()) {
            
            
            t_fiber->getFiberState() = TaskState::running;

            t_fiber->getProcesser() = this;

            ++m_switchCount;

            t_fiber->SwapIn();

            switch (t_fiber->getFiberState())
            {
            case TaskState::running:
                /* code */
                runnableQueue.push(t_fiber);
                t_fiber = nullptr;

                break;
            case TaskState::block:
                /* code */
                
                break;
            case TaskState::done:
              
            default:
                ++gcTimes;
                //gcQueue.push(t_fiber);
                gcQueue.push_back(t_fiber);
                t_fiber = nullptr;

                if(gcQueue.size() > 16) {
                    gc();
                }

                break;
            }
        }
    }
}

void Processer::NotifyCondition(){
    MutexType::Lock lock(m_mutex);

    if(m_waiting) {
        m_cv.notify_all();
    } else {
        m_notified = true;
    }
}

void Processer::waitCondition(){
    this->gc();
    MutexType::Lock lock(m_mutex);
    if(m_notified) {
        m_notified = false;
        return;
    }
    m_waiting = true;
    m_cv.wait(m_mutex);
    m_waiting = false;
}

void Processer::AddFiber(Fiber::ptr fb){
     //MutexType::Lock lock(m_mutex);

    uint32_t times = 0;
    Fiber* fbPtr = fb.get();

    while(!runnableQueue.push(fbPtr)){
        ++times;
        if(times >  100) {
            times = 0;
            LOG_DEBUG(g_logger) << "Processer push newQueue fail 100 times";
            // 暂时让出cpu
        }
    }

    if(m_waiting) {
        m_cv.notify_all();
    } else {
        m_notified = true;
    }
}


// std::list<Fiber> Processer::Steal(size_t n){
//     s
// }


void Processer::gc(){

    Fiber* fb;

    while(!gcQueue.empty()) {
        fb = gcQueue.front();
        fb->~Fiber();
    }
}


void Processer::addNewFiber(){

}

}  // namespace ppcode