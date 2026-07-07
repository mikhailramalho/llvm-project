//===-- Kernel semid64_ds definition --------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIBC_SRC_SYS_SEM_LINUX_KERNEL_SEMID_DS_H
#define LLVM_LIBC_SRC_SYS_SEM_LINUX_KERNEL_SEMID_DS_H

#include "hdr/types/struct_semid_ds.h" // For struct ipc_perm
#include "src/__support/macros/config.h"
#include "src/__support/macros/properties/architectures.h"

namespace LIBC_NAMESPACE_DECL {

// Mirror of the kernel's struct semid64_ds, the layout the semctl syscall
// reads and writes for the struct commands. It does not match the public
// struct semid_ds on every architecture (and cannot on 32-bit targets with
// 64-bit time_t, where the time fields are split across two adjacent
// 32-bit words with no alignment padding), so semctl marshals through this
// type and translates. Note that on architectures where the kernel still
// parses an IPC_64 version flag out of cmd (e.g. arm32), this layout is
// only selected when cmd is or'd with IPC_64; the architectures below all
// use semid64_ds unconditionally.
struct kernel_semid64_ds {
  struct ipc_perm sem_perm;
#if defined(LIBC_TARGET_ARCH_IS_X86_64)
  // x86_64 layout (arch/x86/include/uapi/asm/sembuf.h): a padding word
  // follows each time field.
  long sem_otime;
  unsigned long unused1;
  long sem_ctime;
  unsigned long unused2;
#elif defined(__LP64__)
  // asm-generic 64-bit layout (aarch64, riscv64).
  long sem_otime;
  long sem_ctime;
#else
  // asm-generic 32-bit layout (riscv32, arm32): 64-bit times split across
  // two 32-bit words.
#define LIBC_KERNEL_SEMID_DS_SPLIT_TIME
  unsigned long sem_otime;
  unsigned long sem_otime_high;
  unsigned long sem_ctime;
  unsigned long sem_ctime_high;
#endif
  unsigned long sem_nsems;
  unsigned long unused3;
  unsigned long unused4;
};

} // namespace LIBC_NAMESPACE_DECL

#endif // LLVM_LIBC_SRC_SYS_SEM_LINUX_KERNEL_SEMID_DS_H
