#include "util.h"

#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <stdlib.h>


namespace ppcode{


pid_t GetThreadId() {
    return syscall(SYS_gettid);
}





} // namespace name
