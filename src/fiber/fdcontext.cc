#include "fdcontext.h"

//#include "../log.h"
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "hook.h"

namespace ppcode {

// static Logger::ptr g_logger = LOG_ROOT();

FdCtx::FdCtx(int fd) : m_fd(fd) { init(); }

void FdCtx::init() {
    struct stat st;

    if (fstat(m_fd, &st) == -1) {
        m_isSocket = false;
        // LOG_DEBUG(g_logger) << "The wrong file descriptor may have been
        // used";
    } else {
        m_isSocket = S_ISSOCK(st.st_mode);
    }

    if (m_isSocket) {
        int flags = fcntl(m_fd, F_GETFL, 0);
        if (!(flags & O_NONBLOCK)) {
            fcntl(m_fd, F_SETFL, flags | O_NONBLOCK);
        }
        m_sysNonblock = true;
    } else {
        m_sysNonblock = false;
    }

    m_userNonblock = false;
}

void FdCtx::setTimeout(int type, uint64_t v) {
    if (type == SO_RCVTIMEO) {
        m_recvTimeout = v;
    } else {
        m_sendTimeout = v;
    }
}

uint64_t FdCtx::getTimeout(int type) {
    if (type == SO_RCVTIMEO) {
        return m_recvTimeout;
    } else {
        return m_sendTimeout;
    }
}
FdManager::FdManager() { m_datas.resize(128); }

FdCtx::ptr FdManager::get(int fd, bool auto_create) {
    if (fd < 0) {
        return nullptr;
    }

    {
        RWMutexType::ReadLock lock(m_mutex);
        if (m_datas.size() <= (size_t)fd) {
            if (auto_create) {
                return nullptr;
            }
        } else {
            if (m_datas[fd] || !auto_create) {
                return m_datas[fd];
            }
        }
    }

    RWMutexType::WriteLock lock(m_mutex);
    FdCtx::ptr ctx(new FdCtx(fd));
    if (fd >= (int)m_datas.size()) {
        m_datas.resize(fd * 1.5);
    }
    m_datas[fd] = ctx;
    return ctx;
}

void FdManager::del(int fd) {
    RWMutexType::WriteLock lock(m_mutex);
    if ((int)m_datas.size() <= fd) {
        return;
    }
    m_datas[fd].reset();
}
}  // namespace ppcode