#include "../src/log.h"

#include "../src/fiber/fiber.h"
#include "../src/thread.h"
#include "../src/fiber/processer.h"
#include "../src/fiber/scheduler.h"


 static ppcode::Logger::ptr g_logger = LOG_ROOT();


ppcode::Scheduler* sche = new ppcode::Scheduler();

void fun_product() {
    for(int i = 0; i < 100; ++i) {
        sche->createFiber([i](){
            std::cout << i << std::endl;
        });
    }
}

void test_scheduler(){

    sche->start(3);

}


int main() {

test_scheduler();

sleep(4);
ppcode::Thread::ptr product(new ppcode::Thread("processer", fun_product));

product->join();

}



// ppcode::Processer proc(1);

// 接口已经被改动 
// void test_processer() {
//     proc.start();
// }



// void test_proc(){
    
//     //ppcode::Thread::ptr proc(new ppcode::Thread("processer", test_processer));

//     //ppcode::Thread::ptr product(new ppcode::Thread("product", fun_product));

//     //proc->join();
//     //product->join();
// }