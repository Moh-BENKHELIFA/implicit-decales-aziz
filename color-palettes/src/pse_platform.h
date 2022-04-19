#ifndef PSE_PLATFORM_H
#define PSE_PLATFORM_H

#include <assert.h>
#include <float.h>
#include <math.h>
#include <stddef.h>
#include <stdint.h>

/*******************************************************************************
 * Platform
 ******************************************************************************/
#if defined(__unix__) || defined(__unix) || defined(unix)
  #define PSE_OS_UNIX
#elif defined(_WIN32)
  #define PSE_OS_WINDOWS
  #if defined(__MINGW32__)
    #define MINGW
  #endif
#elif defined(__APPLE__) && defined(__MACH__)
  #define PSE_OS_MACH
#else
  #error "Unsupported OS"
#endif

/*******************************************************************************
 * Compiler
 ******************************************************************************/
#if defined(__GNUC__)
  #define COMPILER_GCC
#elif defined(_MSC_VER)
  #define COMPILER_CL
#else
  #error "Unsupported compiler"
#endif

/*******************************************************************************
 * Debug mode
 ******************************************************************************/
#if defined(DEBUG) || defined(_DEBUG)
#ifndef NDEBUG  /* NDEBUG win over DEBUG */
  #define PSE_DEBUG
#endif
#endif

/*******************************************************************************
 * Symbol visibility
 ******************************************************************************/
#if defined(COMPILER_GCC)
  #define EXPORT_SYMBOL __attribute__((visibility("default")))
  #define IMPORT_SYMBOL
  #define LOCAL_SYMBOL __attribute__((visibility("hidden")))
#elif defined(COMPILER_CL)
  #define EXPORT_SYMBOL __declspec(dllexport)
  #define IMPORT_SYMBOL __declspec(dllimport)
  #define LOCAL_SYMBOL
#else
  #error "Symbol visibility macros undefined"
#endif

/*******************************************************************************
 * Library utils
 ******************************************************************************/
#if defined(PSE_OS_WINDOWS)
#define PSE_LIB_NAME(name)  name ".dll"
#elif defined(PSE_OS_MACH)
#define PSE_LIB_NAME(name)  "lib" name ".dylib"
#else
#define PSE_LIB_NAME(name)  "lib" name ".so"
#endif

/*******************************************************************************
 * Debug utils
 ******************************************************************************/
#ifdef PSE_DEBUG
  #define PSE_DEBUG_INSTR(a) a
#else
  #define PSE_DEBUG_INSTR(a) (void)0
#endif

/*******************************************************************************
 * API Management
 ******************************************************************************/
#ifdef __cplusplus
  #define PSE_API_BEGIN extern "C" {
  #define PSE_API_END }
#else
  #define PSE_API_BEGIN
  #define PSE_API_END
#endif

/*******************************************************************************
 * Result type
 ******************************************************************************/

enum pse_res_t {
  RES_OK,
  RES_BAD_ARG,
  RES_MEM_ERR,
  RES_IO_ERR,
  RES_INVALID,
  RES_BUSY,
  RES_NOT_AUTHORIZED,
  RES_NOT_FOUND,
  RES_INTERNAL,
  RES_NOT_SUPPORTED,
  RES_ALREADY_EXISTS,
  RES_NOT_READY,
  RES_NOT_CONVERGED,
  RES_NUMERICAL_ISSUE
};

#define PSE_CALL(c)                                                            \
  { if(RES_OK != (c)) { assert(false); } } (void)0
#define PSE_CALL_OR_RETURN(res, c)                                             \
  { if(((res) = (c)) != RES_OK) { assert(false); return (res); } } (void)0
#define PSE_CALL_OR_GOTO(res, label, c)                                        \
  { if(((res) = (c)) != RES_OK) { assert(false); goto label; } } (void)0
#define PSE_TRY_CALL_OR_RETURN(res, c)                                         \
  { if(((res) = (c)) != RES_OK) { return (res); } } (void)0
#define PSE_TRY_CALL_OR_GOTO(res, label, c)                                    \
  { if(((res) = (c)) != RES_OK) { goto label; } } (void)0

#define PSE_TRY_VERIFY_OR_ELSE(t,e)                                            \
  { if( !(t) ) { e; } } (void)0
#define PSE_VERIFY_OR_ELSE(t,e)                                                \
  { if( !(t) ) { assert(false); e; } } (void)0

/*******************************************************************************
 * Atomic
 ******************************************************************************/
#if defined(COMPILER_GCC)
  /*#define PSE_ATOMIC int64_t*/
  typedef int64_t pse_atomic_t;
  #define PSE_ATOMIC_INC(A) __sync_add_and_fetch((A), 1)
  #define PSE_ATOMIC_DEC(A) __sync_sub_and_fetch((A), 1)
  #define PSE_ATOMIC_INC_AND_GET(A) __sync_add_and_fetch((A), 1)
  #define PSE_ATOMIC_DEC_AND_GET(A) __sync_sub_and_fetch((A), 1)
  #define PSE_ATOMIC_ADD(A,V) __sync_add_and_fetch((A), V)
  #define PSE_ATOMIC_SUB(A,V) __sync_sub_and_fetch((A), V)
  #define PSE_ATOMIC_CAS_AND_GET_PREV(Atom, NewVal, Comparand)                 \
    __sync_val_compare_and_swap((Atom), (Comparand), (NewVal))
#elif defined(COMPILER_CL)
  #include <Windows.h>
  /*#define PSE_ATOMIC LONGLONG*/
  typedef LONGLONG pse_atomic_t;
  #define PSE_ATOMIC_INC(A) InterlockedIncrement64((A))
  #define PSE_ATOMIC_DEC(A) InterlockedDecrement64((A))
  #define PSE_ATOMIC_INC_AND_GET(A) InterlockedIncrement64((A))
  #define PSE_ATOMIC_DEC_AND_GET(A) InterlockedDecrement64((A))
  #define PSE_ATOMIC_ADD(A,V)                                                  \
    (InterlockedExchangeAdd64((A), (LONGLONG)(V)) + (LONGLONG)(V))
  #define PSE_ATOMIC_SUB(A,V)                                                  \
    (InterlockedExchangeAdd64((A),-(LONGLONG)(V)) - (LONGLONG)(V))
  #define PSE_ATOMIC_CAS_AND_GET_PREV(Atom, NewVal, Comparand)                 \
    (InterlockedCompareExchange64(Atom, NewVal, Comparand))
#else
  #error "Undefined atomic operations"
#endif

#define PSE_ATOMIC_SET(A,V)                                                    \
  PSE_ATOMIC_CAS_AND_GET_PREV((A), V, (*A))
#define PSE_ATOMIC_GET(A) PSE_ATOMIC_ADD(A, 0)

/*******************************************************************************
 * Code inlining
 ******************************************************************************/
#if defined(COMPILER_GCC)
  #define PSE_FINLINE __inline__ __attribute__((always_inline))
  #define PSE_INLINE __inline__
  #define PSE_NOINLINE __attribute__((noinline))
#elif defined(COMPILER_CL)
  #define PSE_FINLINE __forceinline
  #define PSE_INLINE __inline
  #define PSE_NOINLINE  __declspec(noinline)
#else
  #error "Undefined inlining macros"
#endif

/*******************************************************************************
 * Code checking
 ******************************************************************************/
#ifdef COMPILER_GCC
  #define PSE_STATIC_ASSERT(Cond, Msg)                                         \
    static char PSE_CONCAT(PSE_CONCAT(PSE_CONCAT(STATIC_ASSERT_,COUNTER),_),Msg)\
      [1 -  2*(!(Cond))] __attribute__((unused))
#else
  #define PSE_STATIC_ASSERT(Cond, Msg)                                         \
    static char PSE_CONCAT(PSE_CONCAT(PSE_CONCAT(STATIC_ASSERT_,COUNTER),_),Msg)\
      [1 -  2*(!(Cond))];
#endif

/*******************************************************************************
 * Boolean type
 ******************************************************************************/

#ifndef __cplusplus
#ifndef bool
  typedef unsigned char bool;
# ifndef true
#   define true 1
# endif
# ifndef false
#   define false 0
# endif
#endif
#endif

/*******************************************************************************
 * Real type
 ******************************************************************************/

#if defined(PSE_USE_FLOAT_FOR_REAL)
typedef float pse_real_t;
#define PSE_REAL_SQRT(x)    sqrtf((x))
#define PSE_REAL_ABS(x)     fabsf((x))
#define PSE_REAL_EPS        FLT_EPSILON
#define PSE_REAL_SAFE_EPS   (FLT_EPSILON*10)
#define PSE_REAL_HUGE_VAL   HUGE_VALF
#else
typedef double pse_real_t;
#define PSE_REAL_SQRT(x)    sqrt((x))
#define PSE_REAL_ABS(x)     fabs((x))
#define PSE_REAL_EPS        DBL_EPSILON
#define PSE_REAL_SAFE_EPS   (DBL_EPSILON*10)
#define PSE_REAL_HUGE_VAL   HUGE_VAL
#endif
#define PSE_REAL_POW2(x)    ((x)*(x))

/*******************************************************************************
 * Miscellaneous
 ******************************************************************************/
#define PSE_CONCAT__(A, B) A ## B
#define PSE_CONCAT(A, B) PSE_CONCAT__(A, B)

#define PSE_AS_CSTR__(A)  #A
#define PSE_AS_CSTR(A)  PSE_AS_CSTR__(A)

#define COUNTER __COUNTER__

#define PSE_MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define PSE_MAX(a,b)        (((a) > (b)) ? (a) : (b))
#define PSE_CLAMP(a,m,M)    PSE_MIN(PSE_MAX((a), (m)),(M))
#define PSE_IS_IN(a,m,M)    (((m)<=(a))&&((a)<=(M)))

#define PSE_IS_POW2(v)  (((v) & ((v)-1)) == 0 && (v) > 0)

#define PSE_CONTAINER_OF(ptr,type,member)                                      \
  ((type*)((uintptr_t)(ptr) - offsetof(type,member)))

#endif /* PSE_PLATFORM_H */

