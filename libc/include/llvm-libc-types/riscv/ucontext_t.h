//===-- Definition of type ucontext_t -------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// Note: Definitions in this file are based on the Linux kernel ABI.

#ifndef LLVM_LIBC_TYPES_RISCV_UCONTEXT_T_H
#define LLVM_LIBC_TYPES_RISCV_UCONTEXT_T_H

#include "../sigset_t.h"
#include "../stack_t.h"
#include "mcontext_t.h"

typedef struct ucontext_t {
  // The following fields must match the Linux kernel's struct ucontext
  // on RISC-V (see arch/riscv/include/uapi/asm/ucontext.h) to ensure ABI
  // compatibility for signal handling.
  unsigned long uc_flags;
  struct ucontext_t *uc_link;
  stack_t uc_stack;
  sigset_t uc_sigmask;
  // The kernel reserves 1024 bits for the signal mask; pad out the
  // difference so uc_mcontext lands at the kernel's offset. There is no
  // explicit alignment padding needed after this: mcontext_t is 16-byte
  // aligned (via the Q extension state), same as in the kernel.
  unsigned char __unused[1024 / 8 - sizeof(sigset_t)];
  mcontext_t uc_mcontext;
} ucontext_t;

#endif // LLVM_LIBC_TYPES_RISCV_UCONTEXT_T_H
