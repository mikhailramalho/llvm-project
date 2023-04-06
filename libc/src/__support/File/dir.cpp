//===--- Implementation of a platform independent Dir data structure ------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "dir.h"

#include "src/__support/CPP/new.h"
#include "src/__support/error_or.h"
#include "src/errno/libc_errno.h" // For error macros

#include <stdlib.h>

namespace __llvm_libc {

ErrorOr<Dir *> Dir::open(const char *path) {
  auto fd = platform_opendir(path);
  if (!fd)
    return __llvm_libc::Error(fd.error());

  __llvm_libc::AllocChecker ac;
  Dir *dir = new (ac) Dir(fd);
  if (!ac)
    return __llvm_libc::Error(ENOMEM);
  return dir;
}

ErrorOr<struct ::dirent *> Dir::read() {
  MutexLock lock(&mutex);
  if (readptr >= fillsize) {
    auto readsize = platform_fetch_dirents(fd, buffer);
    if (!readsize)
      return __llvm_libc::Error(readsize.error());
    fillsize = readsize;
    readptr = 0;
  }
  if (fillsize == 0)
    return nullptr;

#ifdef SYS_getdents64
  struct ::dirent *d = reinterpret_cast<struct ::dirent *>(buffer + readptr);
#elif defined(SYS_getdents)
  // from https://man7.org/linux/man-pages/man2/getdents.2.html, SYS_getdents
  // returns a slightly different dirent struct:
  struct linux_dirent {
    unsigned long d_ino;
    unsigned long d_off;
    unsigned short d_reclen;
    char d_name[1];
    /*
    char           pad;       // Zero padding byte
    char           d_type;    // File type (only since Linux
                              // 2.6.4); offset is (d_reclen - 1)
    */
  };

  // TODO: so we need to convert the contents from buffer (i.e., a series of
  // struct linux_dirent) structs into struct dirent and return to the user.
#else
#error                                                                         \
    "SYS_getdents and SYS_getdents64 syscalls not available to perform a read operation."
#endif

#ifdef __unix__
  // The d_reclen field is available on Linux but not required by POSIX.
  readptr += d->d_reclen;
#else
  // Other platforms have to implement how the read pointer is to be updated.
#error "DIR read pointer update is missing."
#endif
  return d;
}

int Dir::close() {
  {
    MutexLock lock(&mutex);
    int retval = platform_closedir(fd);
    if (retval != 0)
      return retval;
  }
  delete this;
  return 0;
}

} // namespace __llvm_libc
