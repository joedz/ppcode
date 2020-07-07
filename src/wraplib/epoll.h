#include <sys/epoll.h>
#include "../util/macro.h"
namespace ppcode {
struct Epoll {
public:
    int epoll_fd;

    Epoll(){
        epoll_fd = epoll_create(0x1234);
        ASSERT_BT(epoll_fd > 0);
    } 

    ~Epoll() {
        close(epoll_fd);
    }

    int epoll_ctl( int __fd, int __op, struct epoll_event *_event){
        return ::epoll_ctl(epoll_fd, __op, __fd, _event);
    }
    
    int epoll_wait(struct epoll_event *__events,int __maxevents, int __timeout){
        return ::epoll_wait(epoll_fd, __events, __maxevents, __timeout);
    }
    
    int epoll_add(int __fd, struct epoll_event *_event){
        return ::epoll_ctl(epoll_fd, EPOLL_CTL_ADD, __fd, _event);
    }
    int epoll_mod(int __fd, struct epoll_event *_event){
        return ::epoll_ctl(epoll_fd,EPOLL_CTL_MOD , __fd, _event);
    }
    int epoll_del(int __fd, struct epoll_event *_event){
         return ::epoll_ctl(epoll_fd,EPOLL_CTL_DEL , __fd, _event);
    }

    int epoll_add_fd(int __fd, uint32_t events){
        struct epoll_event event;
        event.events = events;
        event.data.fd = __fd;
         return ::epoll_ctl(epoll_fd, EPOLL_CTL_ADD, __fd, &event);
    }
    int epoll_mod_fd(int __fd, uint32_t events){
        struct epoll_event event;
        event.events = events;
        event.data.fd = __fd;
         return ::epoll_ctl(epoll_fd, EPOLL_CTL_MOD, __fd, &event);
    }
    int epoll_del_fd(int __fd, uint32_t events){
        struct epoll_event event;
        event.events = events;
        event.data.fd = __fd;
         return ::epoll_ctl(epoll_fd, EPOLL_CTL_DEL, __fd, &event);
    }

};
}