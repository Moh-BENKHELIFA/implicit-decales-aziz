#ifndef TYPES_HPP
#define TYPES_HPP

//! \brief Return code for error management
enum ERet {
  ERet_OK,
  ERet_Unavailable,
  ERet_OutOfMemory,
  ERet_IOErr,
  ERet_BadArg,
  ERet_Invalid,
  ERet_NotImplemented,
  ERet_AlreadyExists,
  ERet_NotFound
};

#define CHECK_OR_DO(truth, code)                                               \
  if(!(truth)) { code; } (void)0

#define CALL(call)                                                             \
  CHECK_OR_DO((call) == ERet_OK, assert(false))
#define CALL_OR_GOTO(ret,label,call)                                           \
  CHECK_OR_DO(((ret) = (call)) == ERet_OK, goto label)


#if defined(__GNUC__)
#define COMPILER_GCC
#elif defined(_MSC_VER)
#define COMPILER_CL
#else
#error Compiler not managed yet.
#endif

#if defined(COMPILER_GCC)
#define FORCE_INLINE  __inline__ __attribute__((always_inline))
#elif defined(COMPILER_CL)
#define FORCE_INLINE __forceinline
#else
#error Inline macros must be defined for this compiler
#endif

#endif // TYPES_HPP
