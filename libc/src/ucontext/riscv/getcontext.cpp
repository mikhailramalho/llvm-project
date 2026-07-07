//===-- Implementation of getcontext for RISC-V ---------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "src/ucontext/getcontext.h"
#include "include/llvm-libc-types/ucontext_t.h"
#include "src/__support/common.h"
#include "src/__support/macros/config.h"

#include "hdr/types/size_t.h"
#include "include/llvm-libc-macros/signal-macros.h"
#include <sys/syscall.h>

#if !defined(LIBC_TARGET_ARCH_IS_ANY_RISCV)
#error "Invalid file include"
#endif

#ifdef LIBC_TARGET_ARCH_IS_RISCV32
#define REG_S "sw"
#else
#define REG_S "sd"
#endif

#define GREG(n) __builtin_offsetof(ucontext_t, uc_mcontext.__gregs[n])
#define FREG(n) __builtin_offsetof(ucontext_t, uc_mcontext.__fpregs.__d.__f[n])

namespace LIBC_NAMESPACE_DECL {

// We use naked because we need to capture the exact register state
// at the moment of the function call, avoiding any compiler prologue/epilogue.
__attribute__((naked)) LLVM_LIBC_FUNCTION(int, getcontext,
                                          (ucontext_t * ucp)) noexcept {
  asm(
      // ucp is in a0.
      //
      // Save the callee-saved general purpose registers. The return address
      // doubles as the saved pc: when this context is resumed by setcontext,
      // execution continues in our caller, as if getcontext had just
      // returned.
      REG_S " ra, %c[pc](a0)\n\t"    //
      REG_S " ra, %c[ra](a0)\n\t"    //
      REG_S " sp, %c[sp](a0)\n\t"    //
      REG_S " s0, %c[s0](a0)\n\t"    //
      REG_S " s1, %c[s1](a0)\n\t"    //
      REG_S " s2, %c[s2](a0)\n\t"    //
      REG_S " s3, %c[s3](a0)\n\t"    //
      REG_S " s4, %c[s4](a0)\n\t"    //
      REG_S " s5, %c[s5](a0)\n\t"    //
      REG_S " s6, %c[s6](a0)\n\t"    //
      REG_S " s7, %c[s7](a0)\n\t"    //
      REG_S " s8, %c[s8](a0)\n\t"    //
      REG_S " s9, %c[s9](a0)\n\t"    //
      REG_S " s10, %c[s10](a0)\n\t"  //
      REG_S " s11, %c[s11](a0)\n\t"  //
      // getcontext should return 0 when resumed by setcontext, so store
      // zero in the a0 slot of the context.
      REG_S " zero, %c[a0](a0)\n\t"
#if defined(__riscv_float_abi_double)
      // Save the callee-saved floating point registers and fcsr.
      "frcsr t1\n\t"
      "sw t1, %c[fcsr](a0)\n\t"        //
      "fsd fs0, %c[fs0](a0)\n\t"       //
      "fsd fs1, %c[fs1](a0)\n\t"       //
      "fsd fs2, %c[fs2](a0)\n\t"       //
      "fsd fs3, %c[fs3](a0)\n\t"       //
      "fsd fs4, %c[fs4](a0)\n\t"       //
      "fsd fs5, %c[fs5](a0)\n\t"       //
      "fsd fs6, %c[fs6](a0)\n\t"       //
      "fsd fs7, %c[fs7](a0)\n\t"       //
      "fsd fs8, %c[fs8](a0)\n\t"       //
      "fsd fs9, %c[fs9](a0)\n\t"       //
      "fsd fs10, %c[fs10](a0)\n\t"     //
      "fsd fs11, %c[fs11](a0)\n\t"     //
#elif defined(__riscv_float_abi_single)
#error "getcontext implementation not available for the target architecture."
#endif
      // Capture the signal mask using the rt_sigprocmask syscall.
      // rt_sigprocmask(SIG_BLOCK, NULL, &ucp->uc_sigmask, sizeof(sigset_t))
      "addi a2, a0, %c[sigmask]\n\t"  // oldset = &ucp->uc_sigmask
      "li a1, 0\n\t"                  // set = NULL
      "li a0, %c[sig_block]\n\t"      // how = SIG_BLOCK
      "li a3, %c[sigset_size]\n\t"    // sigsetsize = sizeof(sigset_t)
      "li a7, %c[syscall_num]\n\t"    //
      "ecall\n\t"
      // getcontext returns 0 on success.
      "li a0, 0\n\t"
      "ret\n\t"  //
      ::[sigset_size] "i"(sizeof(sigset_t)),
      [syscall_num] "i"(SYS_rt_sigprocmask), [sig_block] "i"(SIG_BLOCK),
      [pc] "i"(GREG(REG_PC)), [ra] "i"(GREG(REG_RA)), [sp] "i"(GREG(REG_SP)),
      [s0] "i"(GREG(REG_S0)), [s1] "i"(GREG(REG_S1)), [a0] "i"(GREG(REG_A0)),
      [s2] "i"(GREG(REG_S2)), [s3] "i"(GREG(REG_S2 + 1)),
      [s4] "i"(GREG(REG_S2 + 2)), [s5] "i"(GREG(REG_S2 + 3)),
      [s6] "i"(GREG(REG_S2 + 4)), [s7] "i"(GREG(REG_S2 + 5)),
      [s8] "i"(GREG(REG_S2 + 6)), [s9] "i"(GREG(REG_S2 + 7)),
      [s10] "i"(GREG(REG_S2 + 8)), [s11] "i"(GREG(REG_S2 + 9)),
#if defined(__riscv_float_abi_double)
      [fcsr] "i"(__builtin_offsetof(ucontext_t,
                                    uc_mcontext.__fpregs.__d.__fcsr)),
      [fs0] "i"(FREG(8)), [fs1] "i"(FREG(9)), [fs2] "i"(FREG(18)),
      [fs3] "i"(FREG(19)), [fs4] "i"(FREG(20)), [fs5] "i"(FREG(21)),
      [fs6] "i"(FREG(22)), [fs7] "i"(FREG(23)), [fs8] "i"(FREG(24)),
      [fs9] "i"(FREG(25)), [fs10] "i"(FREG(26)), [fs11] "i"(FREG(27)),
#endif
      [sigmask] "i"(__builtin_offsetof(ucontext_t, uc_sigmask))
      : "memory", "a1", "a2", "a3", "a7", "t1");
}

} // namespace LIBC_NAMESPACE_DECL
