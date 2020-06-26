#include "fiber.h"
#include "../thread.h"
#include "../thread/lockfreelist.h"

#include <memory>
#include <iostream>
#include <list>

namespace ppcode {
class Processer{

using FiberQueue = LockFreeList;

public:
     uint64_t getId();
     std::string getName();
     

    static Fiber* GetCuttentFiber();
    static Processer* GetThisPro();


private:
    std::string m_name;
    uint64_t m_id;


    FiberQueue* redayQueue;
    FiberQueue* waitQueue;
    FiberQueue* gcQueue;
    FiberQueue* newQueue;

    Condition_variable  m_cv;

    
    uint64_t m_switchCount;
    bool notified = false;

    
    std::atomic_bool m_waiting;
    std::atomic_uint32_t m_addNewFiber;


};
}

