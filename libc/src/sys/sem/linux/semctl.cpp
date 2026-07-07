//===-- Linux implementation of semctl ------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "src/sys/sem/semctl.h"

#include "hdr/errno_macros.h"
#include "hdr/sys_ipc_macros.h"
#include "hdr/sys_sem_macros.h"
#include "hdr/types/struct_semid_ds.h"
#include "hdr/types/struct_seminfo.h"
#include "src/__support/OSUtil/syscall.h"
#include "src/__support/common.h"
#include "src/__support/libc_errno.h"
#include "src/sys/sem/linux/kernel_semid_ds.h"
#include <stdarg.h>
#include <sys/syscall.h>

namespace LIBC_NAMESPACE_DECL {

LLVM_LIBC_FUNCTION(int, semctl, (int semid, int semnum, int cmd, ...)) {
  // used to parse the fourth varargs argument
  // its expected to be explicitly declared by application as an union type:
  // union semun {
  //  int val;
  //  struct semid_ds *buf;
  //  unsigned short  *array;
  //  struct seminfo  *__buf; (* linux specific *)
  // } arg;
  unsigned long cmd_arg = 0;
  struct semid_ds *user_buf = nullptr;
  kernel_semid64_ds kernel_buf = {};

  // parse cmd_arg based on the flags
  switch (cmd) {
    // does not use the vargs
  case IPC_RMID:
  case GETVAL:
  case GETPID:
  case GETNCNT:
  case GETZCNT:
    break;

    // use vargs as int, semun->val
  case SETVAL: {
    va_list vargs;
    va_start(vargs, cmd);
    cmd_arg = static_cast<unsigned long>(va_arg(vargs, int));
    va_end(vargs);
    break;
  }

    // use vargs as semid_ds*, semun->buf
  case IPC_SET:
  case IPC_STAT:
  case SEM_STAT:
  case SEM_STAT_ANY: {
    va_list vargs;
    va_start(vargs, cmd);
    user_buf = va_arg(vargs, struct semid_ds *);
    va_end(vargs);
    // The kernel reads and writes the semid64_ds layout, which does not
    // match the public struct semid_ds on all architectures, so marshal
    // through a kernel-layout buffer and translate below.
    if (cmd == IPC_SET)
      kernel_buf.sem_perm = user_buf->sem_perm;
    cmd_arg = reinterpret_cast<unsigned long>(&kernel_buf);
    break;
  }

    // use vargs as short*, semun->array
  case GETALL:
  case SETALL: {
    va_list vargs;
    va_start(vargs, cmd);
    cmd_arg = reinterpret_cast<unsigned long>(va_arg(vargs, unsigned short *));
    va_end(vargs);
    break;
  }

    // linux specific, use vargs as seminfo*, semun->__buf
  case IPC_INFO:
  case SEM_INFO: {
    va_list vargs;
    va_start(vargs, cmd);
    cmd_arg = reinterpret_cast<unsigned long>(va_arg(vargs, struct seminfo *));
    va_end(vargs);
    break;
  }

    // unrecognized flags
  default:
    libc_errno = EINVAL;
    return -1;
  }

  int ret = LIBC_NAMESPACE::syscall_impl<int>(SYS_semctl, semid, semnum, cmd,
                                              cmd_arg);
  if (ret < 0) {
    libc_errno = -ret;
    return -1;
  }

  // Translate the kernel layout back into the user's struct for the
  // stat-like commands.
  if (user_buf != nullptr && cmd != IPC_SET) {
    user_buf->sem_perm = kernel_buf.sem_perm;
#ifdef LIBC_KERNEL_SEMID_DS_SPLIT_TIME
    user_buf->sem_otime =
        static_cast<time_t>(kernel_buf.sem_otime) |
        (static_cast<time_t>(kernel_buf.sem_otime_high) << 32);
    user_buf->sem_ctime =
        static_cast<time_t>(kernel_buf.sem_ctime) |
        (static_cast<time_t>(kernel_buf.sem_ctime_high) << 32);
#else
    user_buf->sem_otime = kernel_buf.sem_otime;
    user_buf->sem_ctime = kernel_buf.sem_ctime;
#endif
    user_buf->sem_nsems = kernel_buf.sem_nsems;
  }
  return ret;
}

} // namespace LIBC_NAMESPACE_DECL
