#include "thread_attributes.h"

#include "../log.h"
#include "../util/util.h"

namespace ppcode {

static Logger::ptr g_logger = LOG_ROOT();

Thread_Attributes::Thread_Attributes() {
    int rt = pthread_attr_init(&m_attr);
    if (rt != 0) {
        LOG_ERROR(g_logger) << "pthread_attr_init failed, rt=" << rt;
        throw std::logic_error("pthread_attr_init failed");
    }
}

Thread_Attributes::~Thread_Attributes() {
    int rt = pthread_attr_destroy(&m_attr);
    if (rt != 0) {
        LOG_ERROR(g_logger) << "pthread_attr_destroy failed, rt=" << rt;
    }
}

void Thread_Attributes::set_stack_size(size_t size) {
    // 设置线程的最小限制
#ifdef PTHREAD_STACK_MIN
    if (size < PTHREAD_STACK_MIN) size = PTHREAD_STACK_MIN;
#else
    if (size < 1024 * 1024) size = 1024 * 1024;
#endif

    size_t pageSize = GetPageSize();

    size = ((size + pageSize - 1) / pageSize) * pageSize;

    int rt = pthread_attr_setstacksize(&m_attr, size);
    if (rt != 0) {
        LOG_ERROR(g_logger) << "set_stack_size failed, rt=" << rt;
        throw std::logic_error("set_stack_size failed");
    }
}

size_t Thread_Attributes::get_stack_size() {
    size_t size;
    int rt = pthread_attr_getstacksize(&m_attr, &size);

    if (rt != 0) {
        LOG_ERROR(g_logger) << "get_stack_size failed, rt=" << rt;
        throw std::logic_error("get_stack_size failed");
    }
    return size;
}

}  // namespace ppcode