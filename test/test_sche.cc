#include "../src/fiber/scheduler.h"
#include "../src/fiber/processer.h"
#include "../src/log.h"



int main(){

    std::cout << 1 << std::endl;
    
    LOG_DEBUG(LOG_ROOT()) << "start" << std::endl;

    ppcode::Scheduler();
    ppcode::Scheduler* sche = ppcode::Scheduler::GetScheduler();

    std::cout << "schedler create" << std::endl;

    sche->start(3);

    std::cout << "thread start" << std::endl;

    sche->createFiber([]{
        std::cout << 1 << std::endl;

        std::cout << 1 << std::endl;
    });


    sleep(5);
    

}
