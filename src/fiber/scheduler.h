#include <atomic>
#include <iostream>
#include <list>
#include <string>
#include <vector>

#include "../thread.h"
#include "../thread/lockfreelist.h"
#include "fiber.h"
#include "processer.h"

namespace ppcode {

class Scheduler {
public:
    friend class Processer;
    using ptr = std::shared_ptr<Scheduler>;
    using MutexType = Mutex;
    Scheduler();
~Scheduler();
    void createFiber(const std::function<void()>& fn);
    void start(size_t threads = 1, const std::string& name = "");
    void stop();


    bool hasProcesser() { return m_lastActiveNum == 0;}
    bool isNoFiber() {return m_fiberNum == 0;}
    bool isStop() { return m_stop;}
    bool& isExiting();


public:
    // static Scheduler& getInstance(){
    //     static Scheduler s_sche;
    //     return s_sche;
    // }
    
    static Scheduler* GetScheduler();
    

private:
    
    

    virtual void addFiber(Fiber::ptr fb);
    void dispatcherThread();
    void newProcessThread(size_t num);

    Scheduler(Scheduler const&) = delete;
    Scheduler(Scheduler &&) = delete;
    Scheduler& operator=(Scheduler const&) = delete;
    Scheduler& operator=(Scheduler &&) = delete;
    
private:
    std::string                 m_name;
    MutexType                   m_mutex;
    std::vector<Processer::ptr> m_processers;

    volatile uint32_t           m_lastActiveNum = 0;
    std::atomic_uint32_t        m_fiberNum;
    uint32_t                    m_processerNum;
    bool                        m_stop = false;
    Thread::ptr                 m_dispatcherThread;

};

}  // namespace ppcode
