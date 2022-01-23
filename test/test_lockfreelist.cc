
#include <unistd.h>

#include <atomic>
#include <vector>

#include "../src/log.h"
#include "../src/thread.h"
#include "../src/thread/lockfreelist.h"

std::atomic_uint num = {0};
ppcode::LockFreeList<int> g_queue;

static ppcode::Logger::ptr g_logger = LOG_ROOT();

/*

void fun_customer() {
    int i = 0;
    while (1) {
        std::atomic_uint *data;

        bool flag = g_queue.pop((void **)&data);

        if (flag) {
            LOG_INFO(g_logger) << (*data);
        } else {
            // LOG_INFO(g_logger) << "no data";
            i++;
            if (i > 5) {
                break;
            }
        }
    }
}

void fun_producer() {
    int i = 10;
    while (--i) {
        std::atomic_uint *a = new std::atomic_uint;
        a->store(num);
        num++;
        std::cout << *a << std::endl;
        g_queue.push(a);
    }
}

void run_thread() {
    std::vector<ppcode::Thread::ptr> thr_producer;
    std::vector<ppcode::Thread::ptr> thr_customer;

    for (int i = 0; i < 3; i++) {
        thr_producer.push_back(std::make_shared<ppcode::Thread>(fun_producer));
    }

    for (int i = 0; i < 3; i++) {
        thr_customer.push_back(std::make_shared<ppcode::Thread>(fun_customer));
    }

    for (auto &it : thr_producer) {
        it->join();
    }

    for (auto &it : thr_customer) {
        it->join();
    }
}

void test_one_thrad() {
    ppcode::LockFreeList que(20);

    for (int i = 0; i < 10; ++i) {
        int *data = new int(i + 1);
        que.push(data);
    }

    for (int i = 0; i < 5; ++i) {
        void *data;
        bool flag = que.pop(data);

        if (flag) {
            LOG_INFO(g_logger) << (int)*(int *)(data);
        } else {
            LOG_INFO(g_logger) << "no data";
        }
    }
}
*/
// int main() { test_one_thrad(); }

#include <boost/atomic.hpp>
#include <boost/lockfree/queue.hpp>
#include <boost/thread/thread.hpp>
#include <iostream>

boost::atomic_int producer_count(0);
boost::atomic_int consumer_count(0);

//boost::lockfree::queue<int> queue(128);

void test_one_thread(){
    ppcode::LockFreeList<int> que(20);

    for (int i = 0; i < 10; ++i) {
        int data = i;
        que.push(data);
    }

    for (int i = 0; i < 5; ++i) {
        int data;
        bool flag = que.pop(data);

        if (flag) {
            LOG_INFO(g_logger) << (data);
        } else {
            LOG_INFO(g_logger) << "no data";
        }
    }
}

ppcode::LockFreeList<int> queue(100003);

const int iterations = 1000000;
const int producer_thread_count = 2;
const int consumer_thread_count = 3;


void producer(void) {
    for (int i = 0; i != iterations; ++i) {
        int value = i;
        while(!queue.push(value)){
            
        }
         ++producer_count;
    }
}

boost::atomic<bool> done(false);

void consumer(void) {
    int value;
    while (!done) {
        while(queue.pop(value))
            ++consumer_count;
    }

    while(queue.pop(value)){
         ++consumer_count;
    }

    // while(queue.empty()) {
    //     ++consumer_count;
    // }
    while(queue.pop(value)){
         ++consumer_count;
    }

}


void test_thread() {
    using namespace std;
    cout << "boost::lockfree::queueis" << endl;
    //if (!queue.is_lock_free()) cout << "not";
    cout << "lockfree" << endl;

    boost::thread_group producer_threads, consumer_threads;

    for (int i = 0; i != producer_thread_count; ++i)
        producer_threads.create_thread(producer);

    for (int i = 0; i != consumer_thread_count; ++i)
        consumer_threads.create_thread(consumer);

    producer_threads.join_all();
    done = true;

    consumer_threads.join_all();

    cout << "produced " << producer_count << " objects." << endl;
    cout << "consumed " << consumer_count << " objects." << endl;
}



int main(int argc, char *argv[]) {
    test_thread();
}
