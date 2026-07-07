//===-- Definition of type mcontext_t -------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// Note: Definitions in this file are based on the Linux kernel ABI.

#ifndef LLVM_LIBC_TYPES_RISCV_MCONTEXT_T_H
#define LLVM_LIBC_TYPES_RISCV_MCONTEXT_T_H

// The layout below must match 'struct sigcontext' in the Linux kernel on
// RISC-V (see arch/riscv/include/uapi/asm/sigcontext.h). The kernel stores
// the program counter followed by registers x1 through x31, so __gregs[0]
// is the pc and __gregs[N] holds register xN for N in [1, 31].
typedef unsigned long __riscv_mc_gp_state[32];

// Indices into __gregs, following the glibc naming convention.
enum {
  REG_PC = 0,
#define REG_PC REG_PC
  REG_RA = 1,
#define REG_RA REG_RA
  REG_SP = 2,
#define REG_SP REG_SP
  REG_TP = 4,
#define REG_TP REG_TP
  REG_S0 = 8,
#define REG_S0 REG_S0
  REG_S1 = 9,
#define REG_S1 REG_S1
  REG_A0 = 10,
#define REG_A0 REG_A0
  REG_S2 = 18,
#define REG_S2 REG_S2
};

struct __riscv_mc_f_ext_state {
  unsigned int __f[32];
  unsigned int __fcsr;
};

struct __riscv_mc_d_ext_state {
  unsigned long long __f[32];
  unsigned int __fcsr;
};

struct __riscv_mc_q_ext_state {
  _Alignas(16) unsigned long long __f[64];
  unsigned int __fcsr;
  // Reserved for expansion of the extension state and kept 16-byte aligned,
  // matching the kernel definition.
  unsigned int __reserved[3];
};

union __riscv_mc_fp_state {
  struct __riscv_mc_f_ext_state __f;
  struct __riscv_mc_d_ext_state __d;
  struct __riscv_mc_q_ext_state __q;
};

typedef struct mcontext_t {
  __riscv_mc_gp_state __gregs;
  union __riscv_mc_fp_state __fpregs;
} mcontext_t;

#endif // LLVM_LIBC_TYPES_RISCV_MCONTEXT_T_H
