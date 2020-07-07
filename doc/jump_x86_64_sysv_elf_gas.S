/*
            Copyright Oliver Kowalke 2009.
   Distributed under the Boost Software License, Version 1.0.
      (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

/****************************************************************************************
 *                                                                                      *
 *  ----------------------------------------------------------------------------------  *
 *  |    0    |    1    |    2    |    3    |    4     |    5    |    6    |    7    |  *
 *  ----------------------------------------------------------------------------------  *
 *  |   0x0   |   0x4   |   0x8   |   0xc   |   0x10   |   0x14  |   0x18  |   0x1c  |  *
 *  ----------------------------------------------------------------------------------  *
 *  | fc_mxcsr|fc_x87_cw|        R12        |         R13        |        R14        |  *
 *  ----------------------------------------------------------------------------------  *
 *  ----------------------------------------------------------------------------------  *
 *  |    8    |    9    |   10    |   11    |    12    |    13   |    14   |    15   |  *
 *  ----------------------------------------------------------------------------------  *
 *  |   0x20  |   0x24  |   0x28  |  0x2c   |   0x30   |   0x34  |   0x38  |   0x3c  |  *
 *  ----------------------------------------------------------------------------------  *
 *  |        R15        |        RBX        |         RBP        |        RIP        |  *
 *  ----------------------------------------------------------------------------------  *
 *  ----------------------------------------------------------------------------------  *
 *  |    16   |   17    |                                                            |  *
 *  ----------------------------------------------------------------------------------  *
 *  |   0x40  |   0x44  |                                                            |  *
 *  ----------------------------------------------------------------------------------  *
 *  |        EXIT       |                                                            |  *
 *  ----------------------------------------------------------------------------------  *
 *                                                                                      *
 ****************************************************************************************/
//指定了后续编译出来的内容放在代码段【可执行】
.text

// 声明 jump_fcontext 为全局可见符号 告诉编译器后续跟的是一个全局可见的名字
.globl jump_fcontext

.type jump_fcontext,@function

//对齐伪指令，num 必须是2的整数幂
//告诉汇编程序，本伪指令下面的内存变量必须从下一个能被Num整除的地址开始分配
.align 16

// X86-64 的所有寄存器都是 64 位，相对于 32 位系统来说，仅仅是标识符发生变化，如 %ebp->%rbp；
// X86-64 新增 %r8~%r15 8个寄存器；


jump_fcontext:
    // 保存当前协程的数据存储的寄存器, 压栈保护
    // %rbx，%rbp，%r12，%r13，%14，%15 用作数据存储，遵循被调用者使用规则，简单说就是随便用，调用子函数之前要备份它，以防他被修改
    pushq  %rbp  /* save RBP */
    pushq  %rbx  /* save RBX */
    pushq  %r15  /* save R15 */
    pushq  %r14  /* save R14 */
    pushq  %r13  /* save R13 */
    pushq  %r12  /* save R12 */
/*

# X86-64 寄存器说明
%rax 作为函数返回值使用
%rsp 栈指针寄存器，指向栈顶
%rdi，%rsi，%rdx，%rcx，%r8，%r9 用作函数参数，依次对应第1参数，第2参数。。。
%r10，%r11 用作数据存储，遵循调用者使用规则，简单说就是使用之前要先保存原值
*/

    // rsp 栈顶寄存器下移8个字节, 为新的协程FPU浮点计算预留
    /* prepare stack for FPU */
    leaq  -0x8(%rsp), %rsp

    // %rcx 为函数的第四个参数, je进行判断, 等于则跳转到标识符为1的地方, f(forword)
    //  fpu 浮点运算寄存器
    /* test for flag preserve_fpu */
    cmp  $0, %rcx
    je  1f

    //保存当前 的 MXCSR 内存rsp寄存器
    /* save MMX control- and status-word */
    stmxcsr  (%rsp)
    // 保存当前FPU状态字到rsp + 4的位置
    /* save x87 control-word */
    fnstcw   0x4(%rsp)

1:
    // 保留当前的栈顶位置到 rdi
    /* store RSP (pointing to context-data) in RDI */
    movq  %rsp, (%rdi)

    // 修改栈顶地址, 为新协程的地址
    /* restore RSP (pointing to context-data) from RSI */
    movq  %rsi, %rsp

    // 
    /* test for flag preserve_fpu */
    cmp  $0, %rcx
    je  2f

    /* restore MMX control- and status-word */
    ldmxcsr  (%rsp)
    /* restore x87 control-word */
    fldcw  0x4(%rsp)

2:
    // rsp 栈顶寄存器上移 8 字节，恢复为 FPU 浮点运算预留空间
    /* prepare stack for FPU */
    leaq  0x8(%rsp), %rsp
    
    // 将当前新协程的寄存器恢复
    popq  %r12  /* restrore R12 */
    popq  %r13  /* restrore R13 */
    popq  %r14  /* restrore R14 */
    popq  %r15  /* restrore R15 */
    popq  %rbx  /* restrore RBX */
    popq  %rbp  /* restrore RBP */
    // 将返回地址放到 r8 寄存器中
    /* restore return-address */
    popq  %r8
      // 原协程所属的 task 作为函数返回值存入 rax 寄存器
    /* use third arg as return-value after jump */
    movq  %rdx, %rax
     // 将当前协程的 task 地址放到第一个参数的位置（即替换当前协程的上下文地址）
    /* use third arg as first arg in context function */
    movq  %rdx, %rdi
     // 跳转到返回地址处
    /* indirect jump to context */
    jmp  *%r8
.size jump_fcontext,.-jump_fcontext

/* Mark that we don't need executable stack.  */
.section .note.GNU-stack,"",%progbits
