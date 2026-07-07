//===-- Utilities for suspending threads ----------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIBC_SRC___SUPPORT_THREADS_SLEEP_H
#define LLVM_LIBC_SRC___SUPPORT_THREADS_SLEEP_H

#include "src/__support/macros/attributes.h"
#include "src/__support/macros/config.h"

namespace LIBC_NAMESPACE_DECL {

/// Suspend the thread briefly to assist the thread scheduler during busy loops.
LIBC_INLINE void sleep_briefly() {
#if defined(LIBC_TARGET_ARCH_IS_NVPTX)
  if (__nvvm_reflect("__CUDA_ARCH") >= 700)
    LIBC_INLINE_ASM("nanosleep.u32 64;" ::: "memory");
#elif defined(LIBC_TARGET_ARCH_IS_AMDGPU)
  __builtin_amdgcn_s_sleep(2);
#elif defined(LIBC_TARGET_ARCH_IS_X86)
  __builtin_ia32_pause();
#elif defined(LIBC_TARGET_ARCH_IS_AARCH64) && __has_builtin(__builtin_arm_isb)
  __builtin_arm_isb(0xf);
#elif defined(LIBC_TARGET_ARCH_IS_AARCH64)
  asm volatile("isb\n" ::: "memory");
#elif defined(LIBC_TARGET_ARCH_IS_ANY_RISCV) && defined(__riscv_zihintpause)
  asm volatile("pause\n" ::: "memory");
#else
  // There is no pause/yield instruction on this platform, but this must
  // still be a compiler barrier: a busy-wait loop calling sleep_briefly()
  // would otherwise be an infinite loop without side effects, which is
  // undefined behavior and is removed by the compiler.
  asm volatile("" ::: "memory");
#endif
}

} // namespace LIBC_NAMESPACE_DECL

#endif // LLVM_LIBC_SRC___SUPPORT_THREADS_SLEEP_H
