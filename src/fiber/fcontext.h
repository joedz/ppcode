#pragma once

#include <stddef.h>

typedef long intptr_t;

extern "C" {

typedef void* fcontext_t;
typedef void (*fn_t)(intptr_t);

intptr_t jump_fcontext(fcontext_t* ofc, fcontext_t nfc, intptr_t vp,
                       bool preserve_fpu = false);

fcontext_t make_fcontext(void* stack, size_t size, fn_t fn);

}  // extern "C"
