//===-- Definition of type struct dirent ----------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef __LLVM_LIBC_TYPES_STRUCT_DIRENT_H__
#define __LLVM_LIBC_TYPES_STRUCT_DIRENT_H__

#include <sys/syscall.h> // For syscall definitions.

#ifdef SYS_getdents64
#include <llvm-libc-types/ino64_t.h>
#include <llvm-libc-types/off64_t.h>
#else
#include <llvm-libc-types/ino_t.h>
#include <llvm-libc-types/off_t.h>
#endif

#ifdef SYS_getdents64
typedef ino64_t ino_t;
typedef off64_t off_t;
#endif

struct dirent {
  ino_t d_ino;
#ifdef __unix__
  off_t d_off;
  unsigned short d_reclen;
#endif
  unsigned char d_type;
  // The user code should use strlen to determine actual the size of d_name.
  // Likewise, it is incorrect and prohibited by the POSIX standard to detemine
  // the size of struct dirent type using sizeof. The size should be got using
  // a different method, for example, from the d_reclen field on Linux.
  char d_name[1];
};

#endif // __LLVM_LIBC_TYPES_STRUCT_DIRENT_H__
