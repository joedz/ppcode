#include "hook.h"

#include <dlfcn.h>

#include <cstdarg>

#include "../config/config.h"
#include "../log.h"
#include "fdcontext.h"
#include "scheduler.h"




namespace ppcode {

static thread_local bool t_hook_enable = false;

static Logger::ptr g_logger = LOG_ROOT();
static uint64_t s_connect_timeout = -1;

void initHook();
bool is_hook_enable() { return t_hook_enable; }
void set_hook_enable(bool flag) { t_hook_enable = flag; }
}  // namespace ppcode

struct timer_info {
    int canclelled = 0;
};

template <class OriginFun, class... Args>
static ssize_t do_io(int fd, OriginFun fun, const char *hook_fun_name,
                     uint32_t event, int timeout, Args &&... args) {
    if (!ppcode::t_hook_enable) {
        return fun(fd, std::forward<Args>(args)...);
    }

    ppcode::FdCtx::ptr fd_ctx = ppcode::FdMgr::getInstance()->get(fd);

    if (!fd_ctx) {
        return fun(fd, std::forward<Args>(args)...);
    }

    if (fd_ctx->isClose()) {
        errno = EBADF;
        return -1;
    }

    if (!fd_ctx->isSocket() || fd_ctx->getUserNonblock()) {
        // 如果是用户设置非阻塞的不需要管理
        return fun(fd, std::forward<Args>(args)...);
    }

    uint64_t to = fd_ctx->getTimeout(timeout);
    std::shared_ptr<timer_info> tinfo(new timer_info);

    ssize_t n;

    while (true) {
        n = fun(fd, std::forward<Args>(args)...);
        while (n == 1 && errno == EINTR) {
            n = fun(fd, std::forward<Args>(args)...);
        }

        if (n == -1 && errno == EAGAIN) {
            // 资源不满足条件
            ppcode::Scheduler *sche = ppcode::Scheduler::getScheduler();
            // ppcode::Fiber::ptr fb = ppcode::Processer::getCurrentFiber();

            ppcode ::TimerTask::ptr timerTask;
            std::weak_ptr<timer_info> winfo(tinfo);

            if (to != (uint64_t)-1) {
                // 设置超时时间
                timerTask = sche->getPoller()->createCondTimer(
                    [winfo, fd, sche, event]() {
                        std::shared_ptr<timer_info> t = winfo.lock();
                        if (!t || t->canclelled) {
                            return;
                        }
                        t->canclelled = ETIMEDOUT;
                        sche->getPoller()->cancelEvent(
                            fd, (ppcode::FdContext::Event)event);
                    },
                    winfo, to);
            }
            // 添加事件监听
            int rt = sche->getPoller()->addEvent(
                fd, (ppcode::FdContext::Event)event);
            if (rt) {
                // epoll_ctl添加错误
                if (timerTask) {
                    timerTask->cancel();
                }
                return -1;
            } else {
                ppcode::Processer::toYield();
                if (timerTask) {
                    timerTask->cancel();
                }
                if (tinfo->canclelled) {
                    errno = tinfo->canclelled;
                    return -1;
                }
            }
            continue;
        }  // end if(n == -1 && errno == EAGAIN)
        break;

    }  // end while(true)

    return n;
}

extern "C" {
/*
****************************************************************************
**************************** hook函数列表 ***********************************
****************************************************************************
*/

sleep_fun sleep_f = nullptr;
usleep_fun usleep_f = nullptr;
nanosleep_fun nanosleep_f = nullptr;
socket_fun socket_f = nullptr;
connect_fun connect_f = nullptr;
accept_fun accept_f = nullptr;
read_fun read_f = nullptr;
readv_fun readv_f = nullptr;
recv_fun recv_f = nullptr;
recvfrom_fun recvfrom_f = nullptr;
recvmsg_fun recvmsg_f = nullptr;
write_fun write_f = nullptr;
writev_fun writev_f = nullptr;
send_fun send_f = nullptr;
sendto_fun sendto_f = nullptr;
sendmsg_fun sendmsg_f = nullptr;
close_fun close_f = nullptr;
fcntl_fun fcntl_f = nullptr;
ioctl_fun ioctl_f = nullptr;
getsockopt_fun getsockopt_f = nullptr;
setsockopt_fun setsockopt_f = nullptr;

/*
****************************************************************************
************************ sleep 系列函数hook *********************************
****************************************************************************
*/

// sleep 函数 使当前线程挂起一段时间 单位秒
unsigned int sleep(unsigned int seconds) {
    if (!ppcode::t_hook_enable) {
        ppcode::initHook();
        return sleep_f(seconds);
    }

    ppcode::Scheduler *sche = ppcode::Scheduler::getScheduler();
    ppcode::Fiber::ptr fb = ppcode::Processer::getCurrentFiber();

    sche->getPoller()->createTimer(
        std::bind(&ppcode::Scheduler::addFiber, sche, fb), seconds * 1000);

    ppcode::Processer::toYield();
    return 0;
}

// usleep 函数 使当前线程挂起一段时间 单位微秒
int usleep(useconds_t usec) {
    if (!ppcode::t_hook_enable) {
        ppcode::initHook();
        return usleep_f(usec);
    }

    ppcode::Scheduler *sche = ppcode::Scheduler::getScheduler();
    ppcode::Fiber::ptr fb = ppcode::Processer::getCurrentFiber();

    sche->getPoller()->createTimer(
        std::bind(&ppcode::Scheduler::addFiber, sche, fb), usec / 1000);

    ppcode::Processer::toYield();
    return 0;
}

// nanosleep 函数 使当前线程挂起一段时间
// eq->tv_sec是以秒为单位，而tv_nsec以纳秒为单位（10的-9次方秒）
int nanosleep(const struct timespec *req, struct timespec *rem) {
    if (!ppcode::t_hook_enable) {
        return nanosleep_f(req, rem);
    }

    int timeout = req->tv_sec * 1000 + req->tv_nsec / 1000 / 1000;
    ppcode::Scheduler *sche = ppcode::Scheduler::getScheduler();
    ppcode::Fiber::ptr fb = ppcode::Processer::getCurrentFiber();

    sche->getPoller()->createTimer(
        std::bind(&ppcode::Scheduler::addFiber, sche, fb), timeout);

    ppcode::Processer::toYield();
    return 0;
}

/*
****************************************************************************
************************ socket 系列函数hook *********************************
****************************************************************************
*/
// 创建socket文件描述符
int socket(int domain, int type, int protocol) {
    if (!ppcode::t_hook_enable) {
        return socket_f(domain, type, protocol);
    }

    int fd = socket_f(domain, type, protocol);
    if (fd == -1) {
        return fd;
    }

    ppcode::FdMgr::getInstance()->get(fd, true);
    return fd;
}

int connect_with_timeout(int sockfd, const struct sockaddr *addr,
                         socklen_t addrlen, uint64_t timeout);

// connect 客户端连接
int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    if (!ppcode::t_hook_enable) {
        return connect_f(sockfd, addr, addrlen);
    }
    return connect_with_timeout(sockfd, addr, addrlen,
                                ppcode::s_connect_timeout);
}

// 接受一个新的连接
int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {
    int fd = do_io(sockfd, accept_f, "accept", ppcode::FdContext::READ,
                   SO_RCVTIMEO, addr, addrlen);
    if (fd >= 0) {
        ppcode::FdMgr::getInstance()->get(fd, true);
    }
    return fd;
}

// 关闭文件描述符
int close(int fd) {
    if (!ppcode::t_hook_enable) {
        return close_f(fd);
    }

    ppcode::FdCtx::ptr fd_ctx = ppcode::FdMgr::getInstance()->get(fd);
    if (fd_ctx) {
        ppcode::Scheduler *sche = ppcode::Scheduler::getScheduler();
        if (sche) {
            sche->getPoller()->cancelAll(fd);
        }
        ppcode::FdMgr::getInstance()->del(fd);
    }

    return close_f(fd);
}

/*
****************************************************************************
************************ read 系列函数hook *********************************
****************************************************************************
*/

ssize_t read(int fd, void *buf, size_t count) {
    return do_io(fd, read_f, "read", ppcode::FdContext::READ, SO_RCVTIMEO, buf,
                 count);
}

ssize_t readv(int fd, const struct iovec *iov, int iovcnt) {
    return do_io(fd, readv_f, "readv", ppcode::FdContext::READ, SO_RCVTIMEO,
                 iov, iovcnt);
}

ssize_t recv(int sockfd, void *buf, size_t len, int flags) {
    return do_io(sockfd, recv_f, "recv", ppcode::FdContext::READ, SO_RCVTIMEO,
                 buf, len, flags);
}

ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags,
                 struct sockaddr *src_addr, socklen_t *addrlen) {
    return do_io(sockfd, recvfrom_f, "recvfrom", ppcode::FdContext::READ,
                 SO_RCVTIMEO, buf, len, flags, src_addr, addrlen);
}

ssize_t recvmsg(int sockfd, struct msghdr *msg, int flags) {
    return do_io(sockfd, recvmsg_f, "recvmsg", ppcode::FdContext::READ,
                 SO_RCVTIMEO, msg, flags);
}

/*
****************************************************************************
************************ write 系列函数hook *********************************
****************************************************************************
*/
ssize_t write(int fd, const void *buf, size_t count) {
    return do_io(fd, write_f, "write", ppcode::FdContext::WRITE, SO_SNDTIMEO,
                 buf, count);
}

ssize_t writev(int fd, const struct iovec *iov, int iovcnt) {
    return do_io(fd, writev_f, "writev", ppcode::FdContext::WRITE, SO_SNDTIMEO,
                 iov, iovcnt);
}

ssize_t send(int s, const void *msg, size_t len, int flags) {
    return do_io(s, send_f, "send", ppcode::FdContext::WRITE, SO_SNDTIMEO, msg,
                 len, flags);
}

ssize_t sendto(int s, const void *msg, size_t len, int flags,
               const struct sockaddr *to, socklen_t tolen) {
    return do_io(s, sendto_f, "sendto", ppcode::FdContext::WRITE, SO_SNDTIMEO,
                 msg, len, flags, to, tolen);
}

ssize_t sendmsg(int s, const struct msghdr *msg, int flags) {
    return do_io(s, sendmsg_f, "sendmsg", ppcode::FdContext::WRITE, SO_SNDTIMEO,
                 msg, flags);
}

int connect_with_timeout(int sockfd, const struct sockaddr *addr,
                         socklen_t addrlen, uint64_t timeout) {
    if (sockfd < 0) {
        return -1;
    }

    ppcode::FdCtx::ptr fd_ctx = ppcode::FdMgr::getInstance()->get(sockfd);

    if (!fd_ctx || fd_ctx->isClose()) {
        errno = EBADF;
        return -1;
    }
    // 如果文件描述符不是socket 或者已经默认被设置了非阻塞状态
    if (!fd_ctx->isSocket() || fd_ctx->getUserNonblock()) {
        return connect_f(sockfd, addr, addrlen);
    }
    int n = connect_f(sockfd, addr, addrlen);
    // connect 返回0代表已经成功连接
    if (n == 0) {
        return 0;
        // 返回不是>0 或者 错误码不是非阻塞连接超时 那么返回n
    } else if (n != -1 || errno != EINPROGRESS) {
        return n;
    }

    ppcode::Scheduler *sche = ppcode::Scheduler::getScheduler();
    ppcode::TimerTask::ptr timerTask;
    std::shared_ptr<timer_info> tinfo(new timer_info);
    std::weak_ptr<timer_info> winfo(tinfo);

    if (timeout != (uint64_t)-1) {
        //创建条件定时器
        timerTask = sche->getPoller()->createCondTimer(
            [winfo, sockfd, sche]() {
                auto t = winfo.lock();
                if (!t || t->canclelled) {
                    return;
                }
                t->canclelled = ETIMEDOUT;
                sche->getPoller()->cancelEvent(sockfd,
                                               ppcode::FdContext::WRITE);
            },
            winfo, timeout);
    }

    // 设置sockfd的写事件
    int rt = sche->getPoller()->addEvent(sockfd, ppcode::FdContext::WRITE);
    if (rt == 0) {
        ppcode::Processer::toYield();
        if (timerTask) {
            timerTask->cancel();
        }

        if (tinfo->canclelled) {
            // 这里已经触发了条件定时器, 那么代表连接超时
            errno = tinfo->canclelled;
            return -1;
        }
    } else {
        // epoll_ctl 失败 取消定时器
        if (timerTask) {
            timerTask->cancel();
        }
    }

    int error = 0;
    socklen_t len = sizeof(int);

    // 获取套接字选项获取SOL_SOCKET层次 获取待处理错误并清楚
    if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &len) == -1) {
        return -1;
    }

    if (!error) {
        return 0;
    } else {
        errno = error;
        return -1;
    }
}

/*
****************************************************************************
************************ socket设置 系列函数hook ****************************
****************************************************************************
*/
int fcntl(int fd, int cmd, ... /* arg */) {
    va_list va;
    va_start(va, cmd);
    switch (cmd) {
        case F_SETFL: {
            int arg = va_arg(va, int);
            va_end(va);
            ppcode::FdCtx::ptr fd_ctx = ppcode::FdMgr::getInstance()->get(fd);
            if (!fd_ctx || fd_ctx->isClose() || !fd_ctx->isSocket()) {
                return fcntl_f(fd, cmd, arg);
            }
            fd_ctx->setUserNonblock(arg & O_NONBLOCK);
            if (fd_ctx->getSysNonblock()) {
                arg |= O_NONBLOCK;
            } else {
                arg &= ~O_NONBLOCK;
            }
            return fcntl_f(fd, cmd, arg);
        } break;
        case F_GETFL: {
            va_end(va);
            int arg = fcntl_f(fd, cmd);

            ppcode::FdCtx::ptr fd_ctx = ppcode::FdMgr::getInstance()->get(fd);
            if (!fd_ctx || fd_ctx->isClose() || !fd_ctx->isSocket()) {
                return arg;
            }
            if (fd_ctx->getUserNonblock()) {
                return arg | O_NONBLOCK;
            } else {
                return arg & ~O_NONBLOCK;
            }
        } break;
        case F_DUPFD:
        case F_DUPFD_CLOEXEC:
        case F_SETFD:
        case F_SETOWN:
        case F_SETSIG:
        case F_SETLEASE:
        case F_NOTIFY:
#ifdef F_SETPIPE_SZ
        case F_SETPIPE_SZ:
#endif
        {
            int arg = va_arg(va, int);
            va_end(va);
            return fcntl_f(fd, cmd, arg);
        } break;
        case F_GETFD:
        case F_GETOWN:
        case F_GETSIG:
        case F_GETLEASE:
#ifdef F_GETPIPE_SZ
        case F_GETPIPE_SZ:
#endif
        {
            va_end(va);
            return fcntl_f(fd, cmd);
        } break;
        case F_SETLK:
        case F_SETLKW:
        case F_GETLK: {
            struct flock *arg = va_arg(va, struct flock *);
            va_end(va);
            return fcntl_f(fd, cmd, arg);
        } break;
        case F_GETOWN_EX:
        case F_SETOWN_EX: {
            struct f_owner_exlock *arg = va_arg(va, struct f_owner_exlock *);
            va_end(va);
            return fcntl_f(fd, cmd, arg);
        } break;
        default:
            va_end(va);
            return fcntl_f(fd, cmd);
    }
}

int ioctl(int fd, unsigned long int request, ...) {
    va_list va;
    va_start(va, request);
    void *arg = va_arg(va, void *);
    va_end(va);

    if (FIONBIO == request) {
        bool user_nonblock = !!*(int *)arg;
        ppcode::FdCtx::ptr fd_ctx = ppcode::FdMgr::getInstance()->get(fd);
        // 获取文件描述符
        if (!fd_ctx || fd_ctx->isClose() || !fd_ctx->isSocket()) {
            return ioctl_f(fd, request, arg);
        }
        // 设置用户非阻塞
        fd_ctx->setUserNonblock(user_nonblock);
    }
    return ioctl_f(fd, request, arg);
}

int getsockopt(int sockfd, int level, int optname, void *optval,
               socklen_t *optlen) {
    return getsockopt_f(sockfd, level, optname, optval, optlen);
}

int setsockopt(int sockfd, int level, int optname, const void *optval,
               socklen_t optlen) {
    if (!ppcode::t_hook_enable) {
        return setsockopt_f(sockfd, level, optname, optval, optlen);
    }
    if (level == SOL_SOCKET) {
        if (optname == SO_RCVTIMEO || optname == SO_SNDTIMEO) {
            // 获取需要设置的文件描述符
            ppcode::FdCtx::ptr fd_ctx =
                ppcode::FdMgr::getInstance()->get(sockfd);
            if (fd_ctx) {
                // 记录文件描述设置的超时时间
                const timeval *v = (const timeval *)optval;
                fd_ctx->setTimeout(optname,
                                   v->tv_sec * 1000 + v->tv_usec / 1000);
            }
        }
    }
    return setsockopt_f(sockfd, level, optname, optval, optlen);
}
}

namespace ppcode {

void initHook() {
    static bool hasHook = false;
    if (hasHook) {
        return;
    }
    static void *handle = NULL;
    if (!handle) {
        handle = dlopen("libc.so.6", RTLD_LAZY);
        sleep_f = (sleep_fun)dlsym(handle, "sleep");
        usleep_f = (usleep_fun)dlsym(handle, "usleep");
        nanosleep_f = (nanosleep_fun)dlsym(handle, "nanosleep");
        socket_f = (socket_fun)dlsym(handle, "socket");
        connect_f = (connect_fun)dlsym(handle, "connect");
        accept_f = (accept_fun)dlsym(handle, "accept");
        read_f = (read_fun)dlsym(handle, "read");
        readv_f = (readv_fun)dlsym(handle, "readv");
        recv_f = (recv_fun)dlsym(handle, "recv");
        recvfrom_f = (recvfrom_fun)dlsym(handle, "recvfrom");
        recvmsg_f = (recvmsg_fun)dlsym(handle, "recvmsg");
        write_f = (write_fun)dlsym(handle, "write");
        writev_f = (writev_fun)dlsym(handle, "writev");
        send_f = (send_fun)dlsym(handle, "send");
        sendto_f = (sendto_fun)dlsym(handle, "sendto");
        sendmsg_f = (sendmsg_fun)dlsym(handle, "sendmsg");
        close_f = (close_fun)dlsym(handle, "close");
        fcntl_f = (fcntl_fun)dlsym(handle, "fcntl");
        ioctl_f = (ioctl_fun)dlsym(handle, "ioctl");
        getsockopt_f = (getsockopt_fun)dlsym(handle, "getsockopt");
        setsockopt_f = (setsockopt_fun)dlsym(handle, "setsockopt");
    }
    hasHook = true;
}

struct _HookIniter {
    _HookIniter() {
        initHook();
        static ppcode::ConfigVar<int>::ptr g_tcp_connect_timeout =
            ppcode::Config::Lookup("tcp.connect.timeout", 5000, "tcp connect timeout");
        s_connect_timeout = g_tcp_connect_timeout->getValue();

        g_tcp_connect_timeout->addListener(
            [](const int &old_value, const int &new_value) {
                LOG_INFO(g_logger) << "tcp connect timeout changed from"
                                   << old_value << " ot " << new_value;
                s_connect_timeout = new_value;
            });
    }
};

 _HookIniter s_hook_initer;

}  // namespace ppcode