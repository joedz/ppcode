#include "scheduler.h"
#include "../log.h"
#include "../util/macro.h"
#include "../util/util.h"


namespace ppcode{

static thread_local Scheduler* t_scheduler;

static Logger::ptr g_logger = LOG_ROOT();

    void Scheduler::createFiber(const std::function<void()>& fn){

        Fiber::ptr fb(new Fiber(fn));

        // 协程设置 TODO

        fb->getFiberState() = TaskState::reday;
        
        // 协程私有数据 TODO

        ++m_fiberNum;
        addFiber(fb);
    }

    void Scheduler::start(size_t threads, const std::string& name){
        MutexType::Lock lock(m_mutex);

        if(threads < 1) {
            threads = Thread::hardware_concurrency();
        }

        m_name = name;
        m_processerNum = threads;

        Thread::SetThreadName(name);
        ASSERT_BT(t_scheduler == nullptr);
        //t_scheduler = this;

        auto mainProc = m_processers[0];

        // 在调度器开始时默认创建了一个执行器 这里减一
        for(size_t i = 0; i < threads - 1; ++i) {
            newProcessThread(i);
        }

        // 创建定时器 TODO
    
        mainProc->Process();
    }

    void Scheduler::stop(){
        
        if(m_stop){
            return;
        }
        m_stop = true;

        size_t n = m_processers.size();
        for(size_t i = 0; i < n; ++i) {
            auto p = m_processers[i];
            if(p) {
                p->NotifyCondition();
            }
        }
        // 若存在定时器等其他线程或资源 进行统一删除

        if(m_dispatcherThread->joinable()) 
            m_dispatcherThread->join();
        else {
            m_dispatcherThread->cancel();
        }
    }

     bool& Scheduler::isExiting(){
         static bool exit = false;
         return exit;
     }

     Scheduler* Scheduler::GetScheduler(){
        //  if(t_scheduler)
        //  return t_scheduler;
        
        
        return t_scheduler;
     }



    Scheduler::Scheduler(){
        // TODO 创建一个默认的P
        LOG_DEBUG(g_logger) << "Scheduler is create" << std::endl;
        t_scheduler = this;
        Processer::ptr procPtr = std::make_shared<Processer>(this, 0, "mainPro");
        m_processers.push_back(procPtr);
    }

    Scheduler::~Scheduler(){
        isExiting() = true;
        stop();
        ASSERT_BT(m_stop);
    }

    void Scheduler::addFiber(Fiber::ptr fb){

        auto proc = fb->getProcesser();
        if(proc && proc->isActive() && proc->GetScheduler() == this) {
            proc->AddFiber(fb);
            return;
        }

        proc = Processer::GetCurrentProcesser();
        if(proc && proc->isActive() && proc->GetScheduler() == this) {
            proc->AddFiber(fb);
            return;
        }

        size_t pcount = m_processers.size();
        size_t idx = 0;

        for (std::size_t i = 0; i < pcount; ++i, ++idx) {
            idx = idx % pcount;
            proc = m_processers[idx].get();
            if (proc && proc->isActive())
                break;
        }
         proc->AddFiber(fb);
    }

    void Scheduler::dispatcherThread(){
        
    }

    void Scheduler::newProcessThread(size_t num){
        
        std::string thrName = m_name + "_" + std::to_string(num);

        Processer::ptr p(new Processer(this,num, thrName));

        Thread t( thrName, [this, p]{
            p->Process();
        });

        t.detach();
        m_processers.push_back(p);
    }
    
} // namespace ppcode