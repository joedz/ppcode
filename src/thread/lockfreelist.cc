#include "lockfreelist.h"

#include <assert.h>
#include <stdlib.h>

#include <iostream>
#include <new>
namespace ppcode {

// csa operation

#define atomic_cas(ptr, oldVal, newVal) __sync_bool_compare_and_swap(ptr, oldVal, newVal)

#define atomic_add(ptr, margin) __sync_fetch_and_add(ptr, margin)
#define atomic_sub(ptr, margin) __sync_fetch_ann_sub(ptr, margin)
#define atomic_increment(ptr) __sync_fetch_and_add(ptr, 1)
#define atomic_decrement(ptr) __sync_fetch_and_sub(ptr, 1)

#define atomic_add_and_fetch(ptr, op) __sync_add_and_fetch(ptr, op)
#define atomic_and_and_fetch(ptr, op) __sync_and_and_fetch(ptr, op)
#define atomic_fetch_and_add(ptr, op) __sync_fetch_and_add(ptr, op)
#define atomic_fetch_and_and(ptr, op) __sync_fetch_and_and(ptr, op)
#define atomic_fetch_and_or(ptr, op) __sync_fetch_and_or(ptr, op)

#define atomic_cas2(ptr, oldVal, newVal)  __sync_bool_compare_and_swap((volatile atomic_longlong*)ptr, oldVal, newVal)
#define atomic_read(ptr) __sync_fetch_and_add(ptr, 0)
#define atomic_read_double(ptr)  __sync_fetch_and_add((volatile atomic_longlong*)ptr, 0)

void LockFreeListNode::PrintfList(LockFreeListNode* head) {
    using namespace std;

    int co = 0;

    while (head) {
        std::cout << "addr:" << std::hex << head << ", data:" << std::hex
                  << head->data << ", id:" << std::hex << head->next.vals[0]
                  << std::endl;

        head = (LockFreeListNode*)(head->next.vals[1]);
        ++co;
    }

    std::cout << "total:" << co << std::endl;
}

LockFreeList::LockFreeList(size_t capacity)
    : m_id(0), m_max(capacity), m_freeList(nullptr) {
    InitInternalNodeList();
    LockFreeListNode* tmp = (LockFreeListNode*)AllocNode();
    assert(tmp);
    m_in.vals[0] = (void*)0;
    m_in.vals[1] = tmp;

    m_out = m_in;
}

LockFreeList::~LockFreeList() { delete[] m_freeList; }

void LockFreeList::InitInternalNodeList() {
    assert(m_freeList == nullptr);
    m_freeList = (LockFreeListNode*)new LockFreeListNode[m_max];

    size_t i = 0;
    for (; i < m_max - 1; ++i) {
        m_freeList[i].next.vals[0] = (void*)(i + 1);
        m_freeList[i].next.vals[1] = &m_freeList[i + 1];
    }

    // 将最后一个结点的值赋值成0
    m_freeList[i].next.val = 0;

    m_head.vals[0] = 0;
    m_head.vals[1] = m_freeList;

    m_id = i;
}

LockFreeListNode* LockFreeList::AllocNode() {
    DoublePointer old_head;

    do {
        // old_head.val = *((atomic_longlong)&m_head)
        old_head.val = atomic_read_double(&m_head);

        if (old_head.val == 0) return nullptr;

        if (atomic_cas2(&m_head.val, old_head.val,
                        ((LockFreeListNode*)m_head.vals[1])->next.val))
            break;
        //  将m_head的地址赋给 old_head.val
        //  比较交换   比较 如果 *(&m_head.val) == m_head.val == old_head.val
        //            赋值   m_head.val =
        //            LockFreeListNode*(m_head.vals[1])->next.val

    } while (1);

    LockFreeListNode* ret = (LockFreeListNode*)old_head.vals[1];

    ret->next.val = 0;  //置空
    return ret;
}

void LockFreeList::ReleaseNode(LockFreeListNode* node) {
    assert(node >= m_freeList && node < m_freeList + m_max);

    DoublePointer old_head;
    DoublePointer new_node;

    // 获取 版本 对id2进行增加
    size_t id = atomic_increment(&m_id);

    new_node.vals[0] = (void*)id;
    new_node.vals[1] = node;
    // 结点的值为 版本 + node

    do {
        // old_head.val = m_head.val
        old_head.val = atomic_read_double(&m_head);

        node->next = old_head;

        // m_head.val == old_head.val    m_head.val = new_node.val
        if (atomic_cas2(&m_head.val, old_head.val, new_node.val)) break;
    } while (1);
}

bool LockFreeList::push(void* data) {
    //申请一个节点
    LockFreeListNode* node = AllocNode();

    if (node == nullptr) return false;

    //对申请的节点赋值
    node->data = data;
    node->next.val = 0;

    DoublePointer in, next;
    DoublePointer new_node;

    // 对m_id进行累加   版本控制 解决ABA问题
    size_t id = atomic_increment(&m_id);

    //申请的新节点   将新申请的节点的 id和地址赋给new_node
    new_node.vals[0] = (void*)id;
    new_node.vals[1] = node;

    while (1) {
        // 读一个队尾指针m_in的的val值赋给in.val
        in.val = atomic_read_double(&m_in);

        // 将 node_tail = in.vals[1]
        LockFreeListNode* node_tail =
            (LockFreeListNode*)atomic_read(&in.vals[1]);

        //  next.val = node_tail->next = in.vals[1] = m_in.vals[1]
        next.val = atomic_read_double(&node_tail->next);

        // next.val == 0 当前没有线程修改next.val 失败代表有线程已经修改了这个值
        //
        if (!(next.val == 0)) {
            // next.val应该等于0 ,如果不是代表有线程已经更改了这个值
            //将下一个结点赋给m_in.val
            atomic_cas2(&m_in.val, in.val, next.val);
        }

        // in.vals[1]->next.val == 0   and  in.vals[1]->next.val = new_node.val
        if (atomic_cas2(&(((LockFreeListNode*)(in.vals[1]))->next.val), 0,
                        new_node.val)) {
            // in.val[1]->next.val == m_in.val[1]->next.val;
            break;
        }
    }

    //  m_in指针指向下一个节点   存在失败的可能性
    //  但是在下一个节点添加的时候将会修复
    atomic_cas2(&m_in.val, in.val, new_node.val);
    return true;
}

//出栈
bool LockFreeList::pop(void*& data) {
    DoublePointer out, in, next;

    while (1) {
        // out.val = m_out.val
        out.val = atomic_read_double(&m_out);
        // in.val = m_in.val
        in.val = atomic_read_double(&m_in);

        // node_head = out.val[1]
        LockFreeListNode* node_head =
            (LockFreeListNode*)atomic_read(&out.vals[1]);

        // next.val = node_head->next.val
        next.val = atomic_read_double(&node_head->next);

        // 若next.val == 0 代表当前队列中没有值了
        if (next.val == 0) return false;

        // next 不等于空， 但是in == out 代表 in 没有指向真正的尾巴节点
        // 需要更新m_in
        if (in.val == out.val) {
            atomic_cas2(&(m_in.val), in.val, next.val);
            continue;
        }
        // 获取数据节点
        data = ((LockFreeListNode*)(next.vals[1]))->data;

        // m_out.val == out.val  m_out.val = next.val
        if (atomic_cas2(&(m_out.val), out.val, next.val)) break;
    }
    // 获取现在正在释放的节点
    LockFreeListNode* cur = (LockFreeListNode*)(out.vals[1]);

    // 释放节点到空闲链表
    ReleaseNode(cur);

    return true;
}

bool LockFreeList::pop(void** data) {
    if (data == nullptr) return false;

    return pop(*data);
}

}  // namespace ppcode