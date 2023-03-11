if(LIBC_TARGET_ARCHITECTURE_IS_RISCV64)
  set(FRAME_POINTER -fomit-frame-pointer)
else()
  # The implementation assumes frame pointer on to the stack
  set(FRAME_POINTER -fno-omit-frame-pointer)
endif()

add_entrypoint_object(
  setjmp
  SRCS
    setjmp.cpp
  HDRS
    setjmp_impl.h
  COMPILE_OPTIONS
    -O3 # We do not want any local variables in setjmp
    "${FRAME_POINTER}"
  DEPENDS
    libc.include.setjmp
)

add_entrypoint_object(
  longjmp
  SRCS
    longjmp.cpp
  HDRS
    longjmp.h
  COMPILE_OPTIONS
    -O3 # We do not want any local variables in longjmp
  DEPENDS
    libc.include.setjmp
)
