#include "../src/thread.h"
#include "../src/log.h"

#include <pthread.h>
#include <unistd.h>
#include <functional>

using namespace ppcode;

Logger::ptr g_logger = LOG_ROOT();

void thread_fun(){

    uint32_t count = 0;
    while(count < 10000){
        ++count;
    }

    LOG_INFO(g_logger) << count << " id"
            << " " << Thread::GetThis()->getPid()
            << " " << Thread::GetThis()->getThreadId()
            << " " << std::bind(&Thread::getPid, Thread::GetThis())()
            << " " << Thread::GetThis()->getName()
            << " " << Thread::GetThreadName();
}

int main(){

    ppcode::Thread thr( "workThread", thread_fun);

    thr.join();
    
    LOG_INFO(g_logger) << getpid();

}