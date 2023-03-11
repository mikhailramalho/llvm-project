//===-- Implementation of longjmp -----------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "src/setjmp/longjmp.h"
#include "src/__support/common.h"
#include "src/__support/macros/properties/architectures.h"

#include <setjmp.h>

namespace __llvm_libc {

#if defined(LIBC_TARGET_ARCH_IS_RISCV64)
__attribute__((naked)) // Don't generate function epilogue and prologue
#endif
LLVM_LIBC_FUNCTION(void, longjmp, (__jmp_buf * buf, int val)) __NOEXCEPT {
#ifdef LIBC_TARGET_ARCH_IS_X86_64
  register __UINT64_TYPE__ rbx __asm__("rbx");
  register __UINT64_TYPE__ rbp __asm__("rbp");
  register __UINT64_TYPE__ r12 __asm__("r12");
  register __UINT64_TYPE__ r13 __asm__("r13");
  register __UINT64_TYPE__ r14 __asm__("r14");
  register __UINT64_TYPE__ r15 __asm__("r15");
  register __UINT64_TYPE__ rsp __asm__("rsp");
  register __UINT64_TYPE__ rax __asm__("rax");

  // ABI requires that the return value should be stored in rax. So, we store
  // |val| in rax. Note that this has to happen before we restore the registers
  // from values in |buf|. Otherwise, once rsp and rbp are updated, we cannot
  // read |val|.
  LIBC_INLINE_ASM("mov %1, %0\n\t" : "=r"(rax) : "m"(val) :);
  LIBC_INLINE_ASM("mov %1, %0\n\t" : "=r"(rbx) : "m"(buf->rbx) :);
  LIBC_INLINE_ASM("mov %1, %0\n\t" : "=r"(rbp) : "m"(buf->rbp) :);
  LIBC_INLINE_ASM("mov %1, %0\n\t" : "=r"(r12) : "m"(buf->r12) :);
  LIBC_INLINE_ASM("mov %1, %0\n\t" : "=r"(r13) : "m"(buf->r13) :);
  LIBC_INLINE_ASM("mov %1, %0\n\t" : "=r"(r14) : "m"(buf->r14) :);
  LIBC_INLINE_ASM("mov %1, %0\n\t" : "=r"(r15) : "m"(buf->r15) :);
  LIBC_INLINE_ASM("mov %1, %0\n\t" : "=r"(rsp) : "m"(buf->rsp) :);
  LIBC_INLINE_ASM("jmp *%0\n\t" : : "m"(buf->rip));
#elif defined(LIBC_TARGET_ARCH_IS_RISCV64)
  register long a0 __asm__("a0");
  register long a1 __asm__("a1");
  register long ra __asm__("ra");
  register long s0 __asm__("s0");
  register long s1 __asm__("s1");
  register long s2 __asm__("s2");
  register long s3 __asm__("s3");
  register long s4 __asm__("s4");
  register long s5 __asm__("s5");
  register long s6 __asm__("s6");
  register long s7 __asm__("s7");
  register long s8 __asm__("s8");
  register long s9 __asm__("s9");
  register long s10 __asm__("s10");
  register long s11 __asm__("s11");
  register long sp __asm__("sp");

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wuninitialized"
  LIBC_INLINE_ASM("ld %0, 0(%1)\n\t" : "=r"(ra) : "r"(a0) :);
  LIBC_INLINE_ASM("ld %0, 8(%1)\n\t" : "=r"(s0) : "r"(a0) :);
  LIBC_INLINE_ASM("ld %0, 16(%1)\n\t" : "=r"(s1) : "r"(a0) :);
  LIBC_INLINE_ASM("ld %0, 24(%1)\n\t" : "=r"(s2) : "r"(a0) :);
  LIBC_INLINE_ASM("ld %0, 32(%1)\n\t" : "=r"(s3) : "r"(a0) :);
  LIBC_INLINE_ASM("ld %0, 40(%1)\n\t" : "=r"(s4) : "r"(a0) :);
  LIBC_INLINE_ASM("ld %0, 48(%1)\n\t" : "=r"(s5) : "r"(a0) :);
  LIBC_INLINE_ASM("ld %0, 56(%1)\n\t" : "=r"(s6) : "r"(a0) :);
  LIBC_INLINE_ASM("ld %0, 64(%1)\n\t" : "=r"(s7) : "r"(a0) :);
  LIBC_INLINE_ASM("ld %0, 72(%1)\n\t" : "=r"(s8) : "r"(a0) :);
  LIBC_INLINE_ASM("ld %0, 80(%1)\n\t" : "=r"(s9) : "r"(a0) :);
  LIBC_INLINE_ASM("ld %0, 88(%1)\n\t" : "=r"(s10) : "r"(a0) :);
  LIBC_INLINE_ASM("ld %0, 96(%1)\n\t" : "=r"(s11) : "r"(a0) :);
  LIBC_INLINE_ASM("ld %0, 104(%1)\n\t" : "=r"(sp) : "r"(a0) :);

#if __riscv_float_abi_double
  register double fs0 __asm__("fs0");
  register double fs1 __asm__("fs1");
  register double fs2 __asm__("fs2");
  register double fs3 __asm__("fs3");
  register double fs4 __asm__("fs4");
  register double fs5 __asm__("fs5");
  register double fs6 __asm__("fs6");
  register double fs7 __asm__("fs7");
  register double fs8 __asm__("fs8");
  register double fs9 __asm__("fs9");
  register double fs10 __asm__("fs10");
  register double fs11 __asm__("fs11");

  LIBC_INLINE_ASM("fld %0, 112(%1)\n\t" : "=f"(fs0) : "r"(a0) :);
  LIBC_INLINE_ASM("fld %0, 120(%1)\n\t" : "=f"(fs1) : "r"(a0) :);
  LIBC_INLINE_ASM("fld %0, 128(%1)\n\t" : "=f"(fs2) : "r"(a0) :);
  LIBC_INLINE_ASM("fld %0, 136(%1)\n\t" : "=f"(fs3) : "r"(a0) :);
  LIBC_INLINE_ASM("fld %0, 144(%1)\n\t" : "=f"(fs4) : "r"(a0) :);
  LIBC_INLINE_ASM("fld %0, 152(%1)\n\t" : "=f"(fs5) : "r"(a0) :);
  LIBC_INLINE_ASM("fld %0, 160(%1)\n\t" : "=f"(fs6) : "r"(a0) :);
  LIBC_INLINE_ASM("fld %0, 168(%1)\n\t" : "=f"(fs7) : "r"(a0) :);
  LIBC_INLINE_ASM("fld %0, 176(%1)\n\t" : "=f"(fs8) : "r"(a0) :);
  LIBC_INLINE_ASM("fld %0, 184(%1)\n\t" : "=f"(fs9) : "r"(a0) :);
  LIBC_INLINE_ASM("fld %0, 192(%1)\n\t" : "=f"(fs10) : "r"(a0) :);
  LIBC_INLINE_ASM("fld %0, 200(%1)\n\t" : "=f"(fs11) : "r"(a0) :);
#elif defined(__riscv_float_abi_single)
#error "longjmp implementation not available for the target architecture."
#endif

  LIBC_INLINE_ASM("seqz %0, %1" : "=r"(a0) : "r"(a1) :);
  LIBC_INLINE_ASM("add %0, %0, %1" : "=r"(a0) : "r"(a1), "r"(a0) :);
#pragma GCC diagnostic pop
#else
#error "longjmp implementation not available for the target architecture."
#endif
}

} // namespace __llvm_libc
