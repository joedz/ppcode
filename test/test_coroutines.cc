#include <ucontext.h>

#include <iostream>

#include "../src/util/macro.h"
#include "../src/fiber/fiber.h"

volatile bool isRuning = true;

void test_swapIn();

ppcode::Fiber tctx(test_swapIn, 128 * 1024);

void test_swapIn() {
    
    int i;
    for (i = 1; i <= 30; i++) {
        std::cout << "func loop: " << i << std::endl;
        // 切换上下文
        // jump_fcontext(nfc, &ofc, i, false);
        tctx.SwapOut();
    }
    // 这个函数 执行完成后线程结束
}

void test_context() {
    
    tctx.SwapIn();
    
    int i = 0;
    for (i = 1; i <= 30; ++i) {
      
        std::cout << "main loop " << i << " recv ret "  << std::endl;

        // 保存当前上下文 切换上下文
        tctx.SwapIn();
    }
    std::cout << " main end  main loop" << i << " recv ret="  
              << std::endl; 
}

fcontext_t nfc;
fcontext_t ofc;

void test_fcontext(intptr_t param) {
    std::cout << "fun recv param:" << param << std::endl;

    int i;
    for (i = 1; i <= 20; i++) {
        std::cout << "func loop: " << i << std::endl;
        // 切换上下文
        jump_fcontext(&nfc, ofc, i, false);
    }
    // 这个函数 执行完成后线程结束
}

void test_make_jump() {
    size_t size = 1024 * 128;
    void* ptr = malloc(size);
    ptr = (char*)ptr + size;
    //创建上下文 绑定执行方法 和栈地址空间
    nfc = make_fcontext(ptr, size, test_fcontext);
    // 保存当前上下文ofc 切换切换上下文nfc 传递参数
    int ret = jump_fcontext(&ofc, nfc, 12345678, false);
    int i = 0;
    for (i = 1; i <= 1000; ++i) {
        if (ret == 20) {
            std::cout << "recv " << ret << " break" << std::endl;
            break;
        }
        std::cout << "main loop " << i << " recv ret " << ret << std::endl;

        // 保存当前上下文 切换上下文
        ret = jump_fcontext(&ofc, nfc, 0, false);
    }

    std::cout << " main end  main loop" << i << " recv ret=" << ret
              << std::endl;
}

int main() { 
    test_context();
}