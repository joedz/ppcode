
#include <unistd.h>

#include <atomic>
#include <vector>

#include "../src/log.h"
#include "../src/thread.h"
#include "../src/thread/lockfreelist.h"

std::atomic_uint num = {0};
ppcode::LockFreeList g_queue;

ppcode::Logger::ptr g_logger = LOG_ROOT();

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

int main() { run_thread(); }
