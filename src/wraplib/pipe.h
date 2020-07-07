#pragma once
#include <unistd.h>
#include <fcntl.h>
#include "../log.h"
#include "../util/macro.h"

namespace ppcode{

struct Pipe{
public:
    union
    {
        int pipes_[2];
        int read_;  // 读端
        int write_; // 写端
    }m_pipe;

    Pipe(){
        if(pipe(m_pipe.pipes_) != 0) {
            LOG_DEBUG(LOG_ROOT()) << "Pipe failed to open";
        }
        LOG_INFO(LOG_ROOT()) << "read fd=" << m_pipe.read_
            << "write=" << m_pipe.write_;
        ASSERT_BT(m_pipe.read_ > 0 && m_pipe.write_ > 0);
    }
    
    ~Pipe(){
        close(m_pipe.read_);
        close(m_pipe.write_);
    }
    INLINE int read( char* buffer, size_t size) {
        return ::read(m_pipe.read_, buffer, size);
    }
    INLINE int write(const char* buffer, size_t size){
        return ::write(m_pipe.write_, buffer, size);
    }
};

} // namespace ppcode