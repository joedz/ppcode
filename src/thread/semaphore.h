#pragma once
#include "../util/noncopyable.h"

#include <semaphore.h>
#include <stdint.h>

namespace ppcode{

class Semaphore : Noncopyable {
public:

    Semaphore(uint32_t count = 0);


    ~Semaphore();

 
    void wait();


    void notify();
private:
    sem_t m_semaphore;
};
} // namespace ppcode
