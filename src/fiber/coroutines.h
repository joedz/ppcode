#pragma once 

#include "scheduler.h"

namespace ppcode {


using  SchedulerSingleton = Singleton<Scheduler>;

struct Co {
public:
    Co() {
        m_sche = SchedulerSingleton::getInstance();
        m_poller = m_sche->getPoller();
    }

    static Scheduler* GetScheduler() {
        return m_sche;
    }

    static Poller* GetPoller() {
        return m_poller;
    }

    template<class Function>
    void operator-(Function fn){
        m_sche->createFiber(fn);
    }

    static Scheduler* m_sche;
    static Poller* m_poller;
};





}
