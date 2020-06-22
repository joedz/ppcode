#include "util.h"

#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <stdlib.h>
#include <cxxabi.h>

namespace ppcode
{

    pid_t GetThreadId()
    {
        return syscall(SYS_gettid);
    }

    size_t GetPageSize()
    {
        return syscall(_SC_PAGE_SIZE);
    }

   

} // namespace ppcode
