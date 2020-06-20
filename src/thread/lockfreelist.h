#pragma once 

#include "../util/noncopyable.h"
#include <atomic>
#include <stdint.h>
#include <cstddef>


namespace ppcode {


using atomic_longlong = __int128_t;

union DoublePointer
{
    void* vals[2];
    volatile atomic_longlong val;
}__attribute__((alignad(sizeof(atomic_longlong))));



class LockFreeList {
public:
    // 无锁队列构造函数
    // param[in] capacity : 队列容量
    explicit LockFreeList(size_t capacity = 2048);
    ~LockFreeList();

    
    bool push(void *data);
    bool pop(void*& data);
    bool pop(void** data);

private:

    struct LockFreeListNode{
        DoublePointer next;
        void* volatile data;
        // 打印所有结点 debug使用
        static void PrintfList(LockFreeList::LockFreeListNode*);
    };

    //申请一个结点
    LockFreeListNode* AllocNode();
    // 释放结点
    void ReleaseNode(LockFreeListNode*);
    // 初始化链表
    void InitInternalNodeList();
private:

    volatile size_t m_id;
    const size_t    m_max;

    DoublePointer   m_in;
    DoublePointer   m_out;

    volatile size_t m_id2;
    DoublePointer   m_head;
    LockFreeListNode* m_freeList;

};

}
