#include "../src/fiber/fiber.h"
#include "../src/fiber/processer.h"
#include "../src/fiber/scheduler.h"
#include "../src/log.h"
#include "../src/thread.h"
//#include "../src/fiber/fiber_warp.h"

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

static ppcode::Logger::ptr g_logger = LOG_ROOT();

using namespace ppcode;

static Scheduler* sche = new Scheduler();

std::atomic_uint32_t g_count{0};



auto fun2 = []() {
     LOG_INFO(g_logger)  << g_count;
    // ++ g_count;
};

void fun1() {
    LOG_INFO(g_logger) << "begin";
    ++g_count;

    ppcode::Processer::toYield();

    //poller->createTimer(fun2, 2, true);

    LOG_INFO(g_logger) << "end";
}



void test_socket(Poller* poller) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);

    fcntl(fd, F_SETFL, O_NONBLOCK);

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);
    addr.sin_addr.s_addr = inet_addr("180.101.49.12");

    connect(fd, (const sockaddr*)&addr, sizeof(addr));

    poller->addEvent(fd, FdContext::Event::READ,
                     []() { LOG_INFO(g_logger) << "connection read"; });

    poller->addEvent(fd, FdContext::Event::WRITE, [=]() {
        LOG_INFO(g_logger) << "connection write";
        // poller->cancelEvent(fd, FdContext::Event::READ);
        close(fd);
    });

    // poller->cancelEvent(fd, FdContext::Event::WRITE);
    // poller->addEvent()
}

int main() {
    sche->start();
    
    Poller* poller = sche->getPoller();
    //sche->createFiber(fun1);
    //sleep(3);
    test_socket(poller);
    //int i = 100000;
   // while(i--)

    //poller->createTimer(fun1, 2, true);

    // i = 100000;
    // while(i--)
    // poller->createTimer(fun2, 1, true);

    sleep(3);

    // LOG_INFO(g_logger)  << g_count;
    delete sche;
}
