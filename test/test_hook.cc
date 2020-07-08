#include "../src/fiber/hook.cc"
#include "../src/log.h"

static ppcode::Logger::ptr g_logger = LOG_ROOT();


void test_sleep() {

    LOG_INFO(g_logger) << "sleep start";

    sleep(3);
    LOG_INFO(g_logger) << "sleep end";
    usleep(3);

    
    LOG_INFO(g_logger) << "sleep end";
    struct timespec req;
    req.tv_nsec = 1000;
    req.tv_sec  = 3;
    nanosleep(&req, nullptr);

    LOG_INFO(g_logger) << "sleep end";
}


int main(){

    ppcode::Scheduler*sche = ppcode::Scheduler::getScheduler();
    sche->start();


    sche->createFiber(test_sleep);


    getchar();
    sleep(10000);

}