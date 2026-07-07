//===-- Hermetic integration test for ucontext routines -------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// This is a hermetic integration test for getcontext and setcontext.
// We use a hermetic test here because the heavier unit test infrastructure
// (like GTest) interferes with context switching, stack frame management,
// and floating-point state restoration, causing spurious failures.

#include "test/IntegrationTest/test.h"

#include "include/llvm-libc-types/ucontext_t.h"

#include "src/__support/macros/properties/architectures.h"
#include "src/ucontext/getcontext.h"
#include "src/ucontext/setcontext.h"

void basic_stub_test() {
  ucontext_t ctx;
  static volatile int jumped = 0;

  int ret = LIBC_NAMESPACE::getcontext(&ctx);
  ASSERT_EQ(ret, 0);

  if (!jumped) {
    jumped = 1;
    LIBC_NAMESPACE::setcontext(&ctx);
    ASSERT_TRUE(false); // Should not happen
  }

  ASSERT_TRUE(true);
}

#if defined(LIBC_TARGET_ARCH_IS_X86_64)

void register_preservation_test() {
  ucontext_t ctx;
  static volatile int jumped = 0;

  long checked_r12, checked_r13, checked_r14, checked_r15;

  {
    register long r12_val asm("r12") = 0x1212121212121212;
    register long r13_val asm("r13") = 0x1313131313131313;
    register long r14_val asm("r14") = 0x1414141414141414;
    register long r15_val asm("r15") = 0x1515151515151515;

    register void *rdi_val asm("rdi") = &ctx;

    asm volatile("call *%[getcontext_ptr]"
                 : "+r"(rdi_val), "+r"(r12_val), "+r"(r13_val), "+r"(r14_val),
                   "+r"(r15_val)
                 : [getcontext_ptr] "r"((void *)LIBC_NAMESPACE::getcontext)
                 : "memory", "rax", "rcx", "rdx", "rsi");

    checked_r12 = r12_val;
    checked_r13 = r13_val;
    checked_r14 = r14_val;
    checked_r15 = r15_val;
  }

  if (!jumped) {
    jumped = 1;

    // Modify registers to ensure they are restored from context
    asm volatile("movq $0, %%r12\n\t"
                 "movq $0, %%r13\n\t"
                 "movq $0, %%r14\n\t"
                 "movq $0, %%r15\n\t" ::
                     : "r12", "r13", "r14", "r15");

    register const ucontext_t *rdi_set asm("rdi") = &ctx;
    asm volatile("call *%[setcontext_ptr]" ::"r"(rdi_set),
                 [setcontext_ptr] "r"((void *)LIBC_NAMESPACE::setcontext)
                 : "memory");

    ASSERT_TRUE(false); // Should not reach here
  }

  ASSERT_EQ(checked_r12, (long)0x1212121212121212);
  ASSERT_EQ(checked_r13, (long)0x1313131313131313);
  ASSERT_EQ(checked_r14, (long)0x1414141414141414);
  ASSERT_EQ(checked_r15, (long)0x1515151515151515);
}

void test_rbx_rdx() {
  ucontext_t ctx;
  static volatile int jumped = 0;

  long checked_rbx, checked_rdx;

  {
    register long rbx_val asm("rbx") = 0xBBBBBBBBBBBBBBBB;
    register long rdx_val asm("rdx") = 0xDDDDDDDDDDDDDDDD;

    register void *rdi_val asm("rdi") = &ctx;

    asm volatile("call *%[getcontext_ptr]"
                 : "+r"(rdi_val), "+r"(rbx_val), "+r"(rdx_val)
                 : [getcontext_ptr] "r"((void *)LIBC_NAMESPACE::getcontext)
                 : "memory", "rax", "rcx", "rsi");

    checked_rbx = rbx_val;
    checked_rdx = rdx_val;
  }

  if (!jumped) {
    jumped = 1;

    asm volatile("movq $0, %%rbx\n\t"
                 "movq $0, %%rdx\n\t" ::
                     : "rbx", "rdx");

    register const ucontext_t *rdi_set asm("rdi") = &ctx;
    asm volatile("call *%[setcontext_ptr]" ::"r"(rdi_set),
                 [setcontext_ptr] "r"((void *)LIBC_NAMESPACE::setcontext)
                 : "memory");

    ASSERT_TRUE(false);
  }

  ASSERT_EQ(checked_rbx, (long)0xBBBBBBBBBBBBBBBB);
  ASSERT_EQ(checked_rdx, (long)0xDDDDDDDDDDDDDDDD);
}

void test_r8_r11() {
  ucontext_t ctx;
  static volatile int jumped = 0;

  long checked_r8, checked_r9, checked_r10, checked_r11;

  {
    register long r8_val asm("r8") = 0x0808080808080808;
    register long r9_val asm("r9") = 0x0909090909090909;
    register long r10_val asm("r10") = 0x1010101010101010;
    register long r11_val asm("r11") = 0x1111111111111111;

    register void *rdi_val asm("rdi") = &ctx;

    asm volatile("call *%[getcontext_ptr]"
                 : "+r"(rdi_val), "+r"(r8_val), "+r"(r9_val), "+r"(r10_val),
                   "+r"(r11_val)
                 : [getcontext_ptr] "r"((void *)LIBC_NAMESPACE::getcontext)
                 : "memory", "rax", "rcx", "rdx", "rsi");

    checked_r8 = r8_val;
    checked_r9 = r9_val;
    checked_r10 = r10_val;
    checked_r11 = r11_val;
  }

  if (!jumped) {
    jumped = 1;

    asm volatile("movq $0, %%r8\n\t"
                 "movq $0, %%r9\n\t"
                 "movq $0, %%r10\n\t"
                 "movq $0, %%r11\n\t" ::
                     : "r8", "r9", "r10", "r11");

    register const ucontext_t *rdi_set asm("rdi") = &ctx;
    asm volatile("call *%[setcontext_ptr]" ::"r"(rdi_set),
                 [setcontext_ptr] "r"((void *)LIBC_NAMESPACE::setcontext)
                 : "memory");

    ASSERT_TRUE(false);
  }

  ASSERT_EQ(checked_r8, (long)0x0808080808080808);
  ASSERT_EQ(checked_r9, (long)0x0909090909090909);
  ASSERT_EQ(checked_r10, (long)0x1010101010101010);
  ASSERT_EQ(checked_r11, (long)0x1111111111111111);
}

#elif defined(LIBC_TARGET_ARCH_IS_ANY_RISCV)

// Unlike the x86_64 implementation, the RISC-V implementation only saves
// the registers the psABI requires to survive a function call, so only
// callee-saved registers are checked here. For the same reason, when
// setcontext resumes through the getcontext call below, every caller-saved
// register may hold the value it had at the point of the setcontext call,
// so they must all appear in the clobber list.
#ifdef __riscv_float_abi_double
#define GETCONTEXT_CALL_CLOBBERS                                               \
  "memory", "ra", "t0", "t1", "t2", "t3", "t4", "t5", "t6", "a1", "a2", "a3", \
      "a4", "a5", "a6", "a7", "ft0", "ft1", "ft2", "ft3", "ft4", "ft5",       \
      "ft6", "ft7", "ft8", "ft9", "ft10", "ft11", "fa0", "fa1", "fa2", "fa3", \
      "fa4", "fa5", "fa6", "fa7"
#else
#define GETCONTEXT_CALL_CLOBBERS                                               \
  "memory", "ra", "t0", "t1", "t2", "t3", "t4", "t5", "t6", "a1", "a2", "a3", \
      "a4", "a5", "a6", "a7"
#endif

void register_preservation_test() {
  ucontext_t ctx;
  static volatile int jumped = 0;

  long checked_s2, checked_s3, checked_s4, checked_s5;

  {
    register long s2_val asm("s2") = 0x12121212;
    register long s3_val asm("s3") = 0x13131313;
    register long s4_val asm("s4") = 0x14141414;
    register long s5_val asm("s5") = 0x15151515;

    register void *a0_val asm("a0") = &ctx;

    asm volatile("jalr %[getcontext_ptr]"
                 : "+r"(a0_val), "+r"(s2_val), "+r"(s3_val), "+r"(s4_val),
                   "+r"(s5_val)
                 : [getcontext_ptr] "r"((void *)LIBC_NAMESPACE::getcontext)
                 : GETCONTEXT_CALL_CLOBBERS);

    checked_s2 = s2_val;
    checked_s3 = s3_val;
    checked_s4 = s4_val;
    checked_s5 = s5_val;
  }

  if (!jumped) {
    jumped = 1;

    // Modify registers to ensure they are restored from context
    asm volatile("li s2, 0\n\t"
                 "li s3, 0\n\t"
                 "li s4, 0\n\t"
                 "li s5, 0\n\t" ::
                     : "s2", "s3", "s4", "s5");

    register const ucontext_t *a0_set asm("a0") = &ctx;
    asm volatile("jalr %[setcontext_ptr]" ::"r"(a0_set),
                 [setcontext_ptr] "r"((void *)LIBC_NAMESPACE::setcontext)
                 : "memory");

    ASSERT_TRUE(false); // Should not reach here
  }

  ASSERT_EQ(checked_s2, (long)0x12121212);
  ASSERT_EQ(checked_s3, (long)0x13131313);
  ASSERT_EQ(checked_s4, (long)0x14141414);
  ASSERT_EQ(checked_s5, (long)0x15151515);
}

#ifdef __riscv_float_abi_double
void fp_register_preservation_test() {
  ucontext_t ctx;
  static volatile int jumped = 0;

  double checked_fs2, checked_fs3;

  {
    register double fs2_val asm("fs2") = 2.5;
    register double fs3_val asm("fs3") = 3.5;

    register void *a0_val asm("a0") = &ctx;

    asm volatile("jalr %[getcontext_ptr]"
                 : "+r"(a0_val), "+f"(fs2_val), "+f"(fs3_val)
                 : [getcontext_ptr] "r"((void *)LIBC_NAMESPACE::getcontext)
                 : GETCONTEXT_CALL_CLOBBERS);

    checked_fs2 = fs2_val;
    checked_fs3 = fs3_val;
  }

  if (!jumped) {
    jumped = 1;

    // Zero the registers to ensure they are restored from context
    asm volatile("fcvt.d.w fs2, zero\n\t"
                 "fcvt.d.w fs3, zero\n\t" ::
                     : "fs2", "fs3");

    register const ucontext_t *a0_set asm("a0") = &ctx;
    asm volatile("jalr %[setcontext_ptr]" ::"r"(a0_set),
                 [setcontext_ptr] "r"((void *)LIBC_NAMESPACE::setcontext)
                 : "memory");

    ASSERT_TRUE(false); // Should not reach here
  }

  ASSERT_TRUE(checked_fs2 == 2.5);
  ASSERT_TRUE(checked_fs3 == 3.5);
}
#endif // __riscv_float_abi_double

#endif

TEST_MAIN() {
  basic_stub_test();
#if defined(LIBC_TARGET_ARCH_IS_X86_64)
  register_preservation_test();
  test_rbx_rdx();
  test_r8_r11();
#elif defined(LIBC_TARGET_ARCH_IS_ANY_RISCV)
  register_preservation_test();
#ifdef __riscv_float_abi_double
  fp_register_preservation_test();
#endif
#endif
  return 0;
}
