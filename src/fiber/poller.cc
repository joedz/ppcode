#include "poller.h"

#include <errno.h>
#include <fcntl.h>
#include <sys/epoll.h>

#include <string>

#include "hook.h"
#include "scheduler.h"

namespace ppcode {

static Logger::ptr g_logger = LOG_ROOT();

FdContext::EventContext& FdContext::getContext(Event event) {
    if (event == Event::READ) {
        return read_;
    } else if (event == Event::WRITE) {
        return write_;
    }

    ASSERT_BT2(false, "Unknown fdcontext type");
}

void FdContext::resetContext(EventContext& ctx) {
    ctx.cb_ = nullptr;
    ctx.scheduler_ = nullptr;
}

// 创建协程事件
void FdContext::triggerEvent(Event event) {
    // 删除事件
    events_ = (Event)((char)events_ & ~(char)event);

    EventContext& ctx = getContext(event);
    ctx.scheduler_->createFiber(ctx.cb_);
}

Poller::Poller(Scheduler* sche) : Timer(), m_sche(sche) {
    ASSERT_BT(pipe2(m_pipe, O_NONBLOCK) != -1);

    int rt = m_epoll.epoll_add_fd(m_pipe[0], EPOLLIN | EPOLLET);

    ASSERT_BT(rt >= 0);
    LOG_DEBUG(g_logger) << "The network poller is already running";

    resize(64);
}

Poller::~Poller() {
    for (auto& fd : m_fdContexts) {
        if (fd) {
            delete fd;
        }
    }
}

void Poller::resize(size_t size) {
    m_fdContexts.resize(size);

    for (size_t i = 0; i < m_fdContexts.size(); ++i) {
        if (!m_fdContexts[i]) {
            m_fdContexts[i] = new FdContext();
            m_fdContexts[i]->fd_ = i;
        }
    }
}

int Poller::addEvent(int fd, FdContext::Event event, std::function<void()> cb) {
    FdContext* fd_ctx = nullptr;

    RWMutexType::ReadLock rlock(m_mutex);
    if ((int)m_fdContexts.size() < fd) {
        resize(fd * 2);
    }

    fd_ctx = m_fdContexts[fd];
    rlock.unlock();

    // 细粒度锁
    FdContext::MutexType::Mutex::Lock lock(fd_ctx->mutex_);

    int op = fd_ctx->events_ ? EPOLL_CTL_MOD : EPOLL_CTL_ADD;
    struct epoll_event epevent;
    epevent.events = EPOLLET | event | fd_ctx->events_;
    epevent.data.ptr = fd_ctx;

    int rt = m_epoll.epoll_ctl(fd, op, &epevent);
    if (rt) {
        LOG_ERROR(g_logger) << "epoll_ctl add " << fd << " error";
        return rt;
    }

    ++m_pendingEventCount;
    fd_ctx->events_ = (FdContext::Event)(fd_ctx->events_ | event);

    // 再指定文件描述符的事件上绑定执行器
    FdContext::EventContext& event_ctx = fd_ctx->getContext(event);
    event_ctx.scheduler_ = m_sche;
    event_ctx.cb_.swap(cb);
    return 0;
}

bool Poller::delEvent(int fd, FdContext::Event event) {
    RWMutexType::ReadLock lock(m_mutex);

    if ((int)m_fdContexts.size() < fd) {
        return false;
    }

    FdContext* fd_ctx = m_fdContexts[fd];
    lock.unlock();
    // 细粒度锁
    FdContext::MutexType::Mutex::Lock lock2(fd_ctx->mutex_);

    if (!(fd_ctx->events_ & event)) {
        return false;
    }

    FdContext::Event new_event = (FdContext::Event)(fd_ctx->events_ & ~event);

    int op = new_event ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
    struct epoll_event epevent;
    epevent.events = EPOLLET | new_event;
    epevent.data.ptr = fd_ctx;

    int rt = m_epoll.epoll_ctl(fd, op, &epevent);

    // EEXIST 代表epoll已经存在
    if (rt && errno != EEXIST) {
        LOG_ERROR(g_logger) << "epoll_ctl del" << fd << "error";
        return false;
    }

    --m_pendingEventCount;
    fd_ctx->events_ = new_event;
    FdContext::EventContext& event_ctx = fd_ctx->getContext(event);
    fd_ctx->resetContext(event_ctx);
    return true;
}

bool Poller::cancelEvent(int fd, FdContext::Event event) {
    RWMutexType::ReadLock lock(m_mutex);

    if ((int)m_fdContexts.size() < fd) {
        return false;
    }

    FdContext* fd_ctx = m_fdContexts[fd];
    lock.unlock();
    // 细粒度锁
    FdContext::MutexType::Mutex::Lock lock2(fd_ctx->mutex_);

    if (!(fd_ctx->events_ & event)) {
        return false;
    }

    FdContext::Event new_event = (FdContext::Event)(fd_ctx->events_ & ~event);

    int op = new_event ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
    struct epoll_event epevent;
    epevent.events = EPOLLET | new_event;
    epevent.data.ptr = fd_ctx;

    int rt = m_epoll.epoll_ctl(fd, op, &epevent);

    if (rt) {
        LOG_ERROR(g_logger) << "epoll_ctl del" << fd << "error";
        return false;
    }

    fd_ctx->triggerEvent(event);
    --m_pendingEventCount;
    return true;
}
bool Poller::cancelAll(int fd) {
    RWMutexType::ReadLock lock(m_mutex);

    if ((int)m_fdContexts.size() < fd) {
        return false;
    }

    FdContext* fd_ctx = m_fdContexts[fd];
    lock.unlock();
    // 细粒度锁
    FdContext::MutexType::Mutex::Lock lock2(fd_ctx->mutex_);

    struct epoll_event epevent;
    epevent.events = 0;
    epevent.data.ptr = fd_ctx;

    int rt = m_epoll.epoll_ctl(fd, EPOLL_CTL_DEL, &epevent);

    if (rt) {
        LOG_ERROR(g_logger) << "epoll_ctl del" << fd << "error";
        return false;
    }

    if (fd_ctx->events_ & FdContext::READ) {
        fd_ctx->triggerEvent(FdContext::READ);
        --m_pendingEventCount;
    }
    if (fd_ctx->events_ & FdContext::WRITE) {
        fd_ctx->triggerEvent(FdContext::WRITE);
        --m_pendingEventCount;
    }

    return true;
}

bool Poller::stopping() { return m_state == TaskState::stop; }

void Poller::networkPoller() {
    struct epoll_event* eventArray = new struct epoll_event[1024];
    std::shared_ptr<struct epoll_event> shared_event(
        eventArray, [](epoll_event* ptr) { delete ptr; });
    m_state = TaskState::running;

    while (true) {
        uint64_t next_timeout = Timer::getNextExecuteTime();

        // 网路事件监听器结束
        if (stopping() && m_sche->isStopping()) {
            break;
        }

        static const int MAX_TIMEOUT = 3000;

        // 定时器
        next_timeout = next_timeout > MAX_TIMEOUT ? MAX_TIMEOUT : next_timeout;
        next_timeout = next_timeout == 0 ? MAX_TIMEOUT : next_timeout;

        int rt = m_epoll.epoll_wait(eventArray, 1024, next_timeout);

        if (rt < 0 && errno == EINTR) {
            continue;
        }

        if (stopping() && m_sche->isStopping()) {
            break;
        }

        // LOG_INFO(g_logger) << "timeout and m_state=" <<
        // ToStringTaskState(m_state)
        // << " Scheduler state=" <<ToStringTaskState( m_sche->getState());

        {
            // 处理定时器
            std::vector<std::function<void()>> cbs;
            getExecuteTask(cbs);

            if (!cbs.empty()) {
                for (auto& task : cbs) {
                    if (!stopping()) m_sche->createFiber(task);
                }
            }
        }

        for (int i = 0; i < rt; ++i) {
            struct epoll_event& event = eventArray[i];

            // 当前有消息通知
            if (event.data.fd == m_pipe[0]) {
                uint8_t buff;
                while (read(m_pipe[0], &buff, 1) == 1);
                LOG_ERROR(g_logger) << "The data was retrieved from the pipe";
                continue;
            }

            // 处理网路数据
            FdContext* fd_ctx = (FdContext*)event.data.ptr;
            FdContext::MutexType::Lock lock(fd_ctx->mutex_);

            // 错误 或者中断的错误
            if (event.events & (EPOLLERR | EPOLLHUP)) {
                event.events |= EPOLLIN | EPOLLOUT;
            }

            uint32_t real_events = FdContext::NONE;

            // 获取触发的事件
            if (event.events & EPOLLOUT) {
                real_events |= FdContext::READ;
            }
            if (event.events & EPOLLIN) {
                real_events |= FdContext::WRITE;
            }
            // 若不存在读写事件
            if ((fd_ctx->events_ & real_events) == FdContext::NONE) {
                continue;
            }

            // 将被触发的事件再次添加到或者已经触发的所有事件的文件描述符删除
            int left_events = ((fd_ctx->events_) & ~real_events);
            int op = left_events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
            event.events = EPOLLET | left_events;
            int rt = m_epoll.epoll_ctl(fd_ctx->fd_, op, &event);

            if (rt) {
                LOG_ERROR(g_logger)
                    << "epoll_ctl del" << fd_ctx->fd_ << "error";
                continue;
            }

            // 执行事件
            if (real_events & FdContext::READ) {
                fd_ctx->triggerEvent(FdContext::READ);
                --m_pendingEventCount;
            }
            if (real_events & FdContext::WRITE) {
                fd_ctx->triggerEvent(FdContext::WRITE);
                --m_pendingEventCount;
            }
        }
    }

    LOG_ERROR(g_logger) << "The network pollster has stopped and there are now "
                        << m_pendingEventCount
                        << " more events being listened on.";
}

void Poller::onTimerInsertedAtFront() {
    int rt = write(m_pipe[1], "w", 1);
    ASSERT_BT(rt >= 1);
    // int rt = write(m_pipe.m_pipe.write_, "w", 1);
    // notify();
    // ASSERT_BT(rt == 1);
}

void Poller::notify() {
    int rt = write(m_pipe[1], "w", 1);
    //    LOG_INFO(g_logger) << rt;
    ASSERT_BT(rt >= 1);
}

}  // namespace ppcode