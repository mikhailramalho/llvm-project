//===-- Implementation of setcontext for RISC-V ---------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "src/ucontext/setcontext.h"
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
#define REG_L "lw"
#else
#define REG_L "ld"
#endif

#define GREG(n) __builtin_offsetof(ucontext_t, uc_mcontext.__gregs[n])
#define FREG(n) __builtin_offsetof(ucontext_t, uc_mcontext.__fpregs.__d.__f[n])

namespace LIBC_NAMESPACE_DECL {

__attribute__((naked)) LLVM_LIBC_FUNCTION(int, setcontext,
                                          (const ucontext_t *ucp)) noexcept {
  asm(
      // ucp is in a0. Keep it in t0 across the syscall: the kernel only
      // clobbers a0, and every register we care about is restored from the
      // context afterwards anyway.
      "mv t0, a0\n\t"
      // Restore the signal mask using the rt_sigprocmask syscall.
      // rt_sigprocmask(SIG_SETMASK, &ucp->uc_sigmask, NULL, sizeof(sigset_t))
      // Note: Restoring the signal mask early means that if a signal
      // arrives before the context switch is complete, it will run on
      // the old stack with the new mask.
      "addi a1, a0, %c[sigmask]\n\t"  // set = &ucp->uc_sigmask
      "li a2, 0\n\t"                  // oldset = NULL
      "li a0, %c[sig_setmask]\n\t"    // how = SIG_SETMASK
      "li a3, %c[sigset_size]\n\t"    // sigsetsize = sizeof(sigset_t)
      "li a7, %c[syscall_num]\n\t"    //
      "ecall\n\t"
#if defined(__riscv_float_abi_double)
      // Restore the callee-saved floating point registers and fcsr.
      "lw t1, %c[fcsr](t0)\n\t"
      "fscsr t1\n\t"
      "fld fs0, %c[fs0](t0)\n\t"    //
      "fld fs1, %c[fs1](t0)\n\t"    //
      "fld fs2, %c[fs2](t0)\n\t"    //
      "fld fs3, %c[fs3](t0)\n\t"    //
      "fld fs4, %c[fs4](t0)\n\t"    //
      "fld fs5, %c[fs5](t0)\n\t"    //
      "fld fs6, %c[fs6](t0)\n\t"    //
      "fld fs7, %c[fs7](t0)\n\t"    //
      "fld fs8, %c[fs8](t0)\n\t"    //
      "fld fs9, %c[fs9](t0)\n\t"    //
      "fld fs10, %c[fs10](t0)\n\t"  //
      "fld fs11, %c[fs11](t0)\n\t"  //
#elif defined(__riscv_float_abi_single)
#error "setcontext implementation not available for the target architecture."
#endif
      // Restore the callee-saved general purpose registers.
      REG_L " ra, %c[ra](t0)\n\t"    //
      REG_L " sp, %c[sp](t0)\n\t"    //
      REG_L " s0, %c[s0](t0)\n\t"    //
      REG_L " s1, %c[s1](t0)\n\t"    //
      REG_L " s2, %c[s2](t0)\n\t"    //
      REG_L " s3, %c[s3](t0)\n\t"    //
      REG_L " s4, %c[s4](t0)\n\t"    //
      REG_L " s5, %c[s5](t0)\n\t"    //
      REG_L " s6, %c[s6](t0)\n\t"    //
      REG_L " s7, %c[s7](t0)\n\t"    //
      REG_L " s8, %c[s8](t0)\n\t"    //
      REG_L " s9, %c[s9](t0)\n\t"    //
      REG_L " s10, %c[s10](t0)\n\t"  //
      REG_L " s11, %c[s11](t0)\n\t"  //
      // Restore a0 (holds getcontext's saved return value, i.e. zero) and
      // jump to the saved pc.
      REG_L " a0, %c[a0](t0)\n\t"  //
      REG_L " t1, %c[pc](t0)\n\t"  //
      "jr t1\n\t"                  //
      ::[sigset_size] "i"(sizeof(sigset_t)),
      [syscall_num] "i"(SYS_rt_sigprocmask), [sig_setmask] "i"(SIG_SETMASK),
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
      : "memory");
}

} // namespace LIBC_NAMESPACE_DECL
