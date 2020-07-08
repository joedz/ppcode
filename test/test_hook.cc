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

void test_fun() {
    LOG_INFO(g_logger) << "connection read" ;
}

void test_hook() {
    ppcode::Scheduler*sche = ppcode::Scheduler::getScheduler();
    ppcode::Fiber::ptr fb = ppcode::Processer::getCurrentFiber();

    sche->getPoller()->createTimer(std::bind(
        &ppcode::Scheduler::addFiber, sche, fb
    ), 3 * 1000);

   // sche->getPoller()->createTimer(test_fun, 1);
LOG_INFO(g_logger) << "toYield begin" ;
    ppcode::Processer::toYield();
LOG_INFO(g_logger) << "toYield bcak" ;

}

std::atomic_uint32_t g_count{0};

void fun1() {
    LOG_INFO(g_logger) << g_count;
    ++g_count;

   // ppcode::Processer::toYield();
    //poller->createTimer(fun2, 2, true);
   // LOG_INFO(g_logger) << g_count;
}

int main(){

    ppcode::Scheduler*sche = ppcode::Scheduler::getScheduler();
    sche->start();
    //int fd = socket_f(AF_INET, SOCK_STREAM, 0);

   //std::cout << fd << std::endl;
    //sche->getPoller()->createTimer(fun1, 2, true);
   
    //sche->createFiber(test_sleep);
    sche->createFiber(test_hook);

    getchar();


}