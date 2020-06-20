#include "../src/thread.h"
#include "../src/log.h"

#include <pthread.h>
#include <unistd.h>
#include <functional>
#include <atomic>

using namespace ppcode;

Logger::ptr g_logger = LOG_ROOT();

void thread_fun1(int a, int b, int c){

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
    LOG_INFO(g_logger) << a << b << c;
}



int thread_fun2(int a, int b){
     LOG_INFO(g_logger) << "hello world thread_fun2"<< a << b;
        return a+ b;
}

auto thread_fun3 = [](int a, int b){
    LOG_INFO(g_logger) << "hello world thread_fun2"<< a << b;
};


uint64_t u_num = 0;
int i_num = 0;
Mutex g_lock;
Semaphore g_sem;
Spinlock g_spinlock;
std::atomic<uint64_t> int64_num;

void thread_fun4() {
   for(int i = 0; i < 10000000; ++i) {
       ++int64_num;
    }
}

void thread_fun5() {

    for(int i = 0; i < 10000000; ++i) {
       ++i_num;
    }
}




void test_thread(){
    

    Thread::ptr thr1(new Thread(thread_fun1, 10, 20, 20));
    Thread::ptr thr2(new Thread(thread_fun2, 20,10));
    Thread::ptr thr3(new Thread("thread_1", thread_fun3, 20,30));
    
    thr1->join();
    thr2->join();
    thr3->join();

    std::cout << "---------------------------------------------" << std::endl;

    std::vector<Thread::ptr> vec_thr;

    for(int i = 0; i < 5; ++i) {
        vec_thr.push_back(
            std::make_shared<Thread>(thread_fun4)
        );
    }

    for(auto it : vec_thr){
        it->join();
    }
    LOG_INFO(g_logger) << int64_num << " " << (int64_num.is_lock_free() ? "无锁": "有锁");
    std::cout << "---------------------------------------------" << std::endl;

    std::vector<Thread::ptr> vec_thr2;

    for(int i = 0; i < 5; ++i) {
        vec_thr2.push_back(
            std::make_shared<Thread>(thread_fun5)
        );
    }
    for(auto it : vec_thr2){
        it->join();
    }
    LOG_INFO(g_logger) <<i_num ;

}

void test_mutex() {

    Spinlock splock;
    Spinlock::Lock myscoped(splock);

    std::cout << "ddd" << std::endl;
    

}


int main(){

   //test_thread();

    test_mutex(); 

}