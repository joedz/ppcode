#include "lockfreelist.h"
#include <assert.h>
#include <new>
#include <stdlib.h>

namespace ppcode {


// csa operation 

#define atomic_cas(ptr, oldVal, newVal) __sync_bool_compare_and_swap(ptr, oldVal, newVal)

#define atomic_add(ptr, margin) __sync_fetch_and_add(ptr, margin)
#define atomic_sub(ptr, margin) __sync_fetch_ann_sub(ptr, margin)
#define atomic_increment(ptr)   __sync_fetch_and_add(ptr, 1)
#define atomic_decrement(ptr)   __sync_fetch_and_sub(ptr, 1)

#define atomic_add_and_fetch(ptr, op) __sync_add_and_fetch(ptr, op)
#define atomic_and_and_fetch(ptr, op) __sync_and_and_fetch(ptr, op)
#define atomic_fetch_and_add(ptr, op) __sync_fetch_and_add(ptr, op)
#define atomic_fetch_and_and(ptr, op) __sync_fetch_and_and(ptr, op)
#define atomic_fetch_and_or (ptr, op) __sync_fetch_and_or (ptr, op)


#define atomic_cas2(ptr, oldVal, newVal)   __sync_bool_compare_and_swap((volatile atomic_longlong*)ptr, oldVal, newVal)
#define atomic_read(ptr)  __sync_fetch_and_add(ptr, 0)
#define atomic_read_double(ptr)  __sync_fetch_and_add((volatile atomic_longlong*)ptr, 0)




LockFreeList::LockFreeList(size_t capacity)
    :m_id(0)
    ,m_max(capacity)
    ,m_id2(0)
    ,m_freeList(nullptr){

    InitInternalNodeList();
    LockFreeListNode *tmp = (LockFreeListNode*)AllocNode();
    assert(tmp);
    m_in.vals[0] = (void*)0;
    m_in.vals[1] = tmp;

    m_id = 1;
    m_out = m_in;
}

LockFreeList::~LockFreeList() {
    delete[] m_freeList;
}


void LockFreeList::InitInternalNodeList(){

    assert(m_freeList == nullptr);
    m_freeList = (LockFreeListNode*)new LockFreeListNode[m_max];

    size_t i = 0;
    for(; i < m_max - 1; ++i) {
        m_freeList[i].next.vals[0] = (void*)(i + 1);
        m_freeList[i].next.vals[1] = &m_freeList[i + 1];
    }

    // 将最后一个结点的值赋值成0
    m_freeList[i].next.val = 0;

    m_head.vals[0] = 0;
    m_head.vals[1] = m_freeList;
    m_id2 = i;
}

LockFreeList::LockFreeListNode* LockFreeList::AllocNode() {

    DoublePointer old_head;

    do{
        //old_head.val = atomic_read(&m_head);
        old_head.val = (atomic_longlong)&m_head;

        if(old_head.val == 0) return nullptr;

        if(atomic_cas2(&m_head.val, old_head.val, ((LockFreeListNode*)m_head.vals[1])->next.val))
            break;
        //  将m_head的地址赋给 old_head.val
        //  比较交换   比较 &m_head.val == old_head.val == &m_head
        //  赋值       &m_head.val = LockFreeListNode*(m_head.vals[1])->next.val 

    }while(1);
    
    LockFreeListNode* ret = (LockFreeListNode*)old_head.vals[1];

    ret->next.val = 0;
    return ret;
}




}