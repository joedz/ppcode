#pragma once

#include <assert.h>
#include <stdint.h>
#include <unistd.h>

#include <atomic>
#include <cstddef>

#include "../util/noncopyable.h"
#define atomic_cas(ptr, oldVal, newVal) \
    __sync_bool_compare_and_swap(ptr, oldVal, newVal)

#define atomic_add(ptr, margin) __sync_fetch_and_add(ptr, margin)
#define atomic_sub(ptr, margin) __sync_fetch_ann_sub(ptr, margin)
#define atomic_increment__(ptr) __sync_fetch_and_add(ptr, 1)
#define atomic_decrement(ptr) __sync_fetch_and_sub(ptr, 1)

#define atomic_add_and_fetch(ptr, op) __sync_add_and_fetch(ptr, op)
#define atomic_and_and_fetch(ptr, op) __sync_and_and_fetch(ptr, op)
#define atomic_fetch_and_add(ptr, op) __sync_fetch_and_add(ptr, op)
#define atomic_fetch_and_and(ptr, op) __sync_fetch_and_and(ptr, op)
#define atomic_fetch_and_or(ptr, op) __sync_fetch_and_or(ptr, op)

#define atomic_cas2(ptr, oldVal, newVal) \
    __sync_bool_compare_and_swap((volatile atomic_longlong*)ptr, oldVal, newVal)
#define atomic_read(ptr) __sync_fetch_and_add(ptr, 0)
#define atomic_read_double(ptr) \
    __sync_fetch_and_add((volatile atomic_longlong*)ptr, 0)
namespace ppcode {

template <class T>
class LockFreeList {
    using atomic_longlong = __int128_t;

    union DoublePointer {
        void* vals[2];
        volatile atomic_longlong val;
    } __attribute__((aligned(sizeof(atomic_longlong))));  // 对其 128位

private:
    struct LockFreeListNode {
        DoublePointer next;
        T volatile data;
    };

public:
    // friend LockFreeListNode;
    // 无锁队列构造函数
    // param[in] capacity : 队列容量
    explicit LockFreeList(size_t capacity = 2028)
        : m_id(0), m_max(capacity), m_freeList(nullptr) {
        InitInternalNodeList();

        LockFreeListNode* tmp = (LockFreeListNode*)AllocNode();
        assert(tmp);

        m_in.vals[0] = (void*)0;
        m_in.vals[1] = tmp;

        m_out = m_in;
    }
    ~LockFreeList() { delete[] m_freeList; }

    bool push(T& data) {
        //申请一个节点
        LockFreeListNode* node = AllocNode();

        if (node == nullptr) return false;

        //对申请的节点赋值
        node->data = data;
        node->next.val = 0;

        DoublePointer in, next;
        DoublePointer new_node;

        // 对m_id进行累加   版本控制 解决ABA问题
        size_t id = atomic_increment__(&m_id);

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

            // next.val == 0 当前没有线程修改next.val
            // 失败代表有线程已经修改了这个值
            //
            if (!(next.val == 0)) {
                // next.val应该等于0 ,如果不是代表有线程已经更改了这个值
                //将下一个结点赋给m_in.val
                atomic_cas2(&m_in.val, in.val, next.val);
            }

            // in.vals[1]->next.val == 0   and  in.vals[1]->next.val =
            // new_node.val
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

    bool pop(T& data) {
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

    //     bool LockFreeList::pop(void** data) {
    //     if (data == nullptr) return false;

    //     return pop(*data);
    // }

    

private:
    //申请一个结点
    LockFreeListNode* AllocNode() {
        DoublePointer old_head;

        do {
            // old_head.val = *((atomic_longlong)&m_head)
            old_head.val = atomic_read_double(&m_head);

            if (old_head.val == 0) return nullptr;

            if (atomic_cas2(&m_head.val, old_head.val,
                            ((LockFreeListNode*)m_head.vals[1])->next.val))
                break;
            //  将m_head的地址赋给 old_head.val
            //  比较交换   比较 如果 *(&m_head.val) == m_head.val ==
            //  old_head.val
            //            赋值   m_head.val =
            //            LockFreeListNode*(m_head.vals[1])->next.val

        } while (1);

        LockFreeListNode* ret = (LockFreeListNode*)old_head.vals[1];

        ret->next.val = 0;  //置空
        return ret;
    }

    // 释放结点
    void ReleaseNode(LockFreeListNode* node) {
        assert(node >= m_freeList && node < m_freeList + m_max);

        DoublePointer old_head;
        DoublePointer new_node;

        // 获取 版本 对id2进行增加
        size_t id = atomic_increment__(&m_id);

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
    // 初始化链表
    void InitInternalNodeList() {
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

private:
    volatile size_t m_id;  // 解决 ABA问题 为每一个节点添加一个版本
    const size_t m_max;  // 节点的最大个数

    DoublePointer m_in;   // 队尾指针 负责入队
    DoublePointer m_out;  // 对头指针 负责出队

    DoublePointer m_head;          // 空闲指针头节点
    LockFreeListNode* m_freeList;  // 申请和释放节点
};

}  // namespace ppcode
