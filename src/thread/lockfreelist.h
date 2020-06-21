#pragma once

#include <stdint.h>

#include <atomic>
#include <cstddef>

#include "../util/noncopyable.h"

namespace ppcode {

using atomic_longlong = __int128_t;

union DoublePointer {
    void* vals[2];
    volatile atomic_longlong val;
} __attribute__((aligned(sizeof(atomic_longlong))));    // 对其 128位

struct LockFreeListNode {
    DoublePointer next;
    void* volatile data;
    // 打印所有结点 debug使用
    static void PrintfList(LockFreeListNode*);
};

class LockFreeList {
public:
    friend LockFreeListNode;
    // 无锁队列构造函数
    // param[in] capacity : 队列容量
    explicit LockFreeList(size_t capacity = 2048);
    ~LockFreeList();

    bool push(void* data);
    bool pop(void*& data);
    bool pop(void** data);

private:
    //申请一个结点
    LockFreeListNode* AllocNode();
    // 释放结点
    void ReleaseNode(LockFreeListNode*);
    // 初始化链表
    void InitInternalNodeList();

private:
    volatile size_t m_id;  // 解决 ABA问题 为每一个节点添加一个版本
    const size_t m_max;    // 节点的最大个数

    DoublePointer m_in;   // 队尾指针 负责入队
    DoublePointer m_out;  // 对头指针 负责出队

    DoublePointer m_head;          // 空闲指针头节点
    LockFreeListNode* m_freeList;  // 申请和释放节点
};

}  // namespace ppcode
