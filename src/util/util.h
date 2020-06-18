#pragma once 
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>

namespace ppcode {

    pid_t GetThreadId();

}