#pragma once

#include <memory>
#include <vector>
#include "../thread.h"
#include "../util/singleton.h"

namespace ppcode {

class FdCtx : public std::enable_shared_from_this<FdCtx> {
public:
    using ptr = std::shared_ptr<FdCtx>;

    FdCtx(int fd);
    ~FdCtx() = default;

    // 是否是socket文件描述符
    bool isSocket() const { return m_isSocket; }
    // 是否被close了
    bool isClose() const { return m_isClosed; }
    // 用户设置非阻塞
    void setUserNonblock(bool v) { m_userNonblock = v; }
    // 获取用户设置非阻塞
    bool getUserNonblock() const { return m_userNonblock; }
    // 用户
    void setSysNonblock(bool v) { m_sysNonblock = v; }

    bool getSysNonblock() const { return m_sysNonblock; }

    void setTimeout(int type, uint64_t v);

    uint64_t getTimeout(int type);

private:
    void init();

private:
    /// 是否socket
    bool m_isSocket = false;
    /// 是否hook非阻塞
    bool m_sysNonblock = false;
    /// 是否用户主动设置非阻塞
    bool m_userNonblock = false;
    /// 是否关闭
    bool m_isClosed = false;
    /// 文件句柄
    int m_fd;
    /// 读超时时间毫秒
    uint64_t m_recvTimeout = -1;
    /// 写超时时间毫秒
    uint64_t m_sendTimeout = -1;
};

class FdManager : public Singleton<FdManager>{
public:
    typedef RWMutex RWMutexType;

    FdManager();

    FdCtx::ptr get(int fd, bool auto_create = false);

    void del(int fd);

private:
    /// 读写锁
    RWMutex m_mutex;
    /// 文件句柄集合
    std::vector<FdCtx::ptr> m_datas;
};



}  // namespace ppcode