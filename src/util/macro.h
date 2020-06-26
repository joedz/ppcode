#include <assert.h>
#include <string>
#include "../log.h"
#include "util.h"


#if defined(__GNUC__) && (__GNUC__ > 3 ||(__GNUC__ == 3 && __GNUC_MINOR__ >= 1))
//强制内联
#define INLINE __attribute__ ((always_inline)) inline 
#else
#define INLINE inline
#endif


#if defined(__GNUC__)
// LIKELY(X)    告诉编译器这里的条件大概率成立
# define LIKELY(x) __builtin_expect(!!(x), 1)
// UNLIKELY(x) 告诉编译器这里的条件大概率不成立
# define UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
# define LIKELY(x) x
# define UNLIKELY(x) x
#endif


// 断言 打印栈信息
#define ASSERT_BT(x)                                    \
    if(UNLIKELY(!(x))) {                                \
        LOG_ERROR(LOG_ROOT()) << "ASSERTION: " #x       \
            << "\nbacktrace:\n"                         \
            << ppcode::BacktraceToString();             \
        assert(x);                                      \
    }                                                   



// 断言 打印栈信息
#define ASSERT_BT2(x, w)                                \
    if(UNLIKELY(!(x))) {                                \
        LOG_ERROR(LOG_ROOT()) << "ASSERTION: " #x       \
            << "\n" << w                                \
            << "\nbacktrace:\n"                         \
            << ppcode::BacktraceToString();             \
        assert(x);                                      \
    }                                                   
