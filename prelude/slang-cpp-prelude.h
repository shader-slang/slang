#ifndef SLANG_CPP_PRELUDE_H
#define SLANG_CPP_PRELUDE_H

// Because the signature of isnan, isfinite, and is isinf changed in C++, we use the macro
// to use the version in the std namespace.
// https://stackoverflow.com/questions/39130040/cmath-hides-isnan-in-math-h-in-c14-c11

#ifdef SLANG_LLVM
#include "slang-llvm.h"
#else // SLANG_LLVM
#if SLANG_GCC_FAMILY && __GNUC__ < 6
#include <cmath>
#define SLANG_PRELUDE_STD std::
#else
#include <math.h>
#define SLANG_PRELUDE_STD
#endif

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#endif // SLANG_LLVM

// Is intptr_t not equal to equal-width sized integer type?
#if defined(__APPLE__)
#define SLANG_INTPTR_TYPE_IS_DISTINCT 1
#else
#define SLANG_INTPTR_TYPE_IS_DISTINCT 0
#endif

#if defined(_MSC_VER)
#define SLANG_PRELUDE_SHARED_LIB_EXPORT __declspec(dllexport)
#else
#define SLANG_PRELUDE_SHARED_LIB_EXPORT __attribute__((__visibility__("default")))
// #   define SLANG_PRELUDE_SHARED_LIB_EXPORT __attribute__ ((dllexport))
// __attribute__((__visibility__("default")))
#endif

#ifdef __cplusplus
#define SLANG_PRELUDE_EXTERN_C extern "C"
#define SLANG_PRELUDE_EXTERN_C_START \
    extern "C"                       \
    {
#define SLANG_PRELUDE_EXTERN_C_END }
#else
#define SLANG_PRELUDE_EXTERN_C
#define SLANG_PRELUDE_EXTERN_C_START
#define SLANG_PRELUDE_EXTERN_C_END
#endif

#define SLANG_PRELUDE_EXPORT SLANG_PRELUDE_EXTERN_C SLANG_PRELUDE_SHARED_LIB_EXPORT
#define SLANG_PRELUDE_EXPORT_START SLANG_PRELUDE_EXTERN_C_START SLANG_PRELUDE_SHARED_LIB_EXPORT
#define SLANG_PRELUDE_EXPORT_END SLANG_PRELUDE_EXTERN_C_END

#ifndef INFINITY
// Must overflow for double
#define INFINITY float(1e+300 * 1e+300)
#endif

#ifndef SLANG_INFINITY
#define SLANG_INFINITY INFINITY
#endif

// Detect the compiler type

#ifndef SLANG_COMPILER
#define SLANG_COMPILER

/*
Compiler defines, see http://sourceforge.net/p/predef/wiki/Compilers/
NOTE that SLANG_VC holds the compiler version - not just 1 or 0
*/
#if defined(_MSC_VER)
#if _MSC_VER >= 1900
#define SLANG_VC 14
#elif _MSC_VER >= 1800
#define SLANG_VC 12
#elif _MSC_VER >= 1700
#define SLANG_VC 11
#elif _MSC_VER >= 1600
#define SLANG_VC 10
#elif _MSC_VER >= 1500
#define SLANG_VC 9
#else
#error "unknown version of Visual C++ compiler"
#endif
#elif defined(__clang__)
#define SLANG_CLANG 1
#elif defined(__SNC__)
#define SLANG_SNC 1
#elif defined(__ghs__)
#define SLANG_GHS 1
#elif defined(__GNUC__) /* note: __clang__, __SNC__, or __ghs__ imply __GNUC__ */
#define SLANG_GCC 1
#else
#error "unknown compiler"
#endif
/*
Any compilers not detected by the above logic are now now explicitly zeroed out.
*/
#ifndef SLANG_VC
#define SLANG_VC 0
#endif
#ifndef SLANG_CLANG
#define SLANG_CLANG 0
#endif
#ifndef SLANG_SNC
#define SLANG_SNC 0
#endif
#ifndef SLANG_GHS
#define SLANG_GHS 0
#endif
#ifndef SLANG_GCC
#define SLANG_GCC 0
#endif
#endif /* SLANG_COMPILER */

/*
The following section attempts to detect the target platform being compiled for.

If an application defines `SLANG_PLATFORM` before including this header,
they take responsibility for setting any compiler-dependent macros
used later in the file.

Most applications should not need to touch this section.
*/
#ifndef SLANG_PLATFORM
#define SLANG_PLATFORM
/**
Operating system defines, see http://sourceforge.net/p/predef/wiki/OperatingSystems/
*/
#if defined(WINAPI_FAMILY) && WINAPI_FAMILY == WINAPI_PARTITION_APP
#define SLANG_WINRT 1 /* Windows Runtime, either on Windows RT or Windows 8 */
#elif defined(XBOXONE)
#define SLANG_XBOXONE 1
#elif defined(_WIN64) /* note: XBOXONE implies _WIN64 */
#define SLANG_WIN64 1
#elif defined(_M_PPC)
#define SLANG_X360 1
#elif defined(_WIN32) /* note: _M_PPC implies _WIN32 */
#define SLANG_WIN32 1
#elif defined(__ANDROID__)
#define SLANG_ANDROID 1
#elif defined(__linux__) || defined(__CYGWIN__) /* note: __ANDROID__ implies __linux__ */
#define SLANG_LINUX 1
#elif defined(__APPLE__) && !defined(SLANG_LLVM)
#include "TargetConditionals.h"
#if TARGET_OS_MAC
#define SLANG_OSX 1
#else
#define SLANG_IOS 1
#endif
#elif defined(__APPLE__)
// On `slang-llvm` we can't inclue "TargetConditionals.h" in general, so for now assume its
// OSX.
#define SLANG_OSX 1
#elif defined(__CELLOS_LV2__)
#define SLANG_PS3 1
#elif defined(__ORBIS__)
#define SLANG_PS4 1
#elif defined(__SNC__) && defined(__arm__)
#define SLANG_PSP2 1
#elif defined(__ghs__)
#define SLANG_WIIU 1
#else
#error "unknown target platform"
#endif


/*
Any platforms not detected by the above logic are now now explicitly zeroed out.
*/
#ifndef SLANG_WINRT
#define SLANG_WINRT 0
#endif
#ifndef SLANG_XBOXONE
#define SLANG_XBOXONE 0
#endif
#ifndef SLANG_WIN64
#define SLANG_WIN64 0
#endif
#ifndef SLANG_X360
#define SLANG_X360 0
#endif
#ifndef SLANG_WIN32
#define SLANG_WIN32 0
#endif
#ifndef SLANG_ANDROID
#define SLANG_ANDROID 0
#endif
#ifndef SLANG_LINUX
#define SLANG_LINUX 0
#endif
#ifndef SLANG_IOS
#define SLANG_IOS 0
#endif
#ifndef SLANG_OSX
#define SLANG_OSX 0
#endif
#ifndef SLANG_PS3
#define SLANG_PS3 0
#endif
#ifndef SLANG_PS4
#define SLANG_PS4 0
#endif
#ifndef SLANG_PSP2
#define SLANG_PSP2 0
#endif
#ifndef SLANG_WIIU
#define SLANG_WIIU 0
#endif
#endif /* SLANG_PLATFORM */

/* Shorthands for "families" of compilers/platforms */
#define SLANG_GCC_FAMILY (SLANG_CLANG || SLANG_SNC || SLANG_GHS || SLANG_GCC)
#define SLANG_WINDOWS_FAMILY (SLANG_WINRT || SLANG_WIN32 || SLANG_WIN64)
#define SLANG_MICROSOFT_FAMILY (SLANG_XBOXONE || SLANG_X360 || SLANG_WINDOWS_FAMILY)
#define SLANG_LINUX_FAMILY (SLANG_LINUX || SLANG_ANDROID)
#define SLANG_APPLE_FAMILY (SLANG_IOS || SLANG_OSX) /* equivalent to #if __APPLE__ */
#define SLANG_UNIX_FAMILY \
    (SLANG_LINUX_FAMILY || SLANG_APPLE_FAMILY) /* shortcut for unix/posix platforms */

// GCC Specific
#if SLANG_GCC_FAMILY

#if INTPTR_MAX == INT64_MAX
#define SLANG_64BIT 1
#else
#define SLANG_64BIT 0
#endif

#define SLANG_BREAKPOINT(id) __builtin_trap()

// Use this macro instead of offsetof, because gcc produces warning if offsetof is used on a
// non POD type, even though it produces the correct result
#define SLANG_OFFSET_OF(T, ELEMENT) (size_t(&((T*)1)->ELEMENT) - 1)
#endif // SLANG_GCC_FAMILY

// Microsoft VC specific
#if SLANG_VC

#define SLANG_BREAKPOINT(id) __debugbreak();

#endif // SLANG_VC

// Default impls

#ifndef SLANG_OFFSET_OF
#define SLANG_OFFSET_OF(X, Y) offsetof(X, Y)
#endif

#ifndef SLANG_BREAKPOINT
// Make it crash with a write to 0!
#define SLANG_BREAKPOINT(id) (*((int*)0) = int(id));
#endif

// If slang.h has been included we don't need any of these definitions
#ifndef SLANG_H

/* Macro for declaring if a method is no throw. Should be set before the return parameter. */
#ifndef SLANG_NO_THROW
#if SLANG_WINDOWS_FAMILY && !defined(SLANG_DISABLE_EXCEPTIONS)
#define SLANG_NO_THROW __declspec(nothrow)
#endif
#endif
#ifndef SLANG_NO_THROW
#define SLANG_NO_THROW
#endif

/* The `SLANG_STDCALL` and `SLANG_MCALL` defines are used to set the calling
convention for interface methods.
*/
#ifndef SLANG_STDCALL
#if SLANG_MICROSOFT_FAMILY
#define SLANG_STDCALL __stdcall
#else
#define SLANG_STDCALL
#endif
#endif
#ifndef SLANG_MCALL
#define SLANG_MCALL SLANG_STDCALL
#endif

#ifndef SLANG_FORCE_INLINE
#define SLANG_FORCE_INLINE inline
#endif

// TODO(JS): Should these be in slang-cpp-types.h?
// They are more likely to clash with slang.h

struct SlangUUID
{
    uint32_t data1;
    uint16_t data2;
    uint16_t data3;
    uint8_t data4[8];
};

typedef int32_t SlangResult;

struct ISlangUnknown
{
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    queryInterface(SlangUUID const& uuid, void** outObject) = 0;
    virtual SLANG_NO_THROW uint32_t SLANG_MCALL addRef() = 0;
    virtual SLANG_NO_THROW uint32_t SLANG_MCALL release() = 0;
};

#define SLANG_COM_INTERFACE(a, b, c, d0, d1, d2, d3, d4, d5, d6, d7)             \
public:                                                                          \
    SLANG_FORCE_INLINE static const SlangUUID& getTypeGuid()                     \
    {                                                                            \
        static const SlangUUID guid = {a, b, c, d0, d1, d2, d3, d4, d5, d6, d7}; \
        return guid;                                                             \
    }
#endif // SLANG_H

// Includes

#include "slang-cpp-scalar-intrinsics.h"
#include "slang-cpp-types.h"

// Atomic helpers for the CPU target. The C++ emitter lowers every atomic IR
// instruction (from `Interlocked*`, `Atomic<T>` and the compiler's own
// counters, such as shader coverage) to a call of
// `_slang_atomic_<op>_<type>(ptr, ...)`:
//
//   - ops: load, store, exchange, compare_exchange, add, sub, and, or, xor,
//     min, max;
//   - types: u32, i32, u64, i64 (all ops), f32, f64 (all but the bitwise
//     ones).
//
// Each helper except `store` returns the PRIOR value (the value before the
// operation), as HLSL `Interlocked*` and GLSL `atomic*` do. The last
// parameter is the memory order, as the `MemoryOrder` values (0 relaxed,
// 1 acquire, 2 release, 3 acquire-release, 4 sequentially consistent).
// `compare_exchange` compares bit patterns, as `InterlockedCompareExchange`
// does for floats on the GPU targets. Integer operations wrap around. Float
// add/sub and all min/max are compare-and-swap loops.
//
// Compiler coverage:
//   - MSVC:        `_Interlocked*` intrinsics. They are full barriers, so any
//                  memory order is met.
//   - GCC / Clang: the `__atomic` builtins, with the requested order.
//   - Others (Sony SNC, Green Hills MULTI, ...): a non-atomic fallback. It is
//     racy if the groups of a dispatch run concurrently, but keeps the prelude
//     buildable there.
#if SLANG_VC
#include <intrin.h>
static_assert(sizeof(long) == 4, "the 32-bit _Interlocked* intrinsics operate on `long`");
static_assert(sizeof(long long) == 8, "the 64-bit _Interlocked* intrinsics operate on `long long`");
// `_SlangAtomicWord<SIZE>` is the atomic layer on MSVC: it wraps the
// `_Interlocked*` intrinsics for a machine word of SIZE bytes (4 or 8). `W` is
// the signed word type the intrinsics take, `U` the unsigned type of the same
// size, used for wrap-around arithmetic. The address must be aligned to SIZE,
// as for any `Interlocked*` operand. The word knows nothing of the element
// type: it only moves bit patterns and does two's complement add and bitwise
// operations.
template<int SIZE>
struct _SlangAtomicWord;
template<>
struct _SlangAtomicWord<4>
{
    typedef long W;
    typedef unsigned long U;
    static W load(volatile W* p)
    {
#if defined(_M_IX86) || defined(_M_X64)
        // An aligned load is atomic, and x86 does not reorder it with later
        // memory operations; the barrier keeps the compiler from doing so.
        W v = *p;
        _ReadWriteBarrier();
        return v;
#else
        return _InterlockedOr(p, 0);
#endif
    }
    static W exchange(volatile W* p, W v) { return _InterlockedExchange(p, v); }
    static W compareExchange(volatile W* p, W cmp, W v)
    {
        return _InterlockedCompareExchange(p, v, cmp);
    }
    static W add(volatile W* p, W v) { return _InterlockedExchangeAdd(p, v); }
    static W bitAnd(volatile W* p, W v) { return _InterlockedAnd(p, v); }
    static W bitOr(volatile W* p, W v) { return _InterlockedOr(p, v); }
    static W bitXor(volatile W* p, W v) { return _InterlockedXor(p, v); }
};
template<>
struct _SlangAtomicWord<8>
{
    typedef long long W;
    typedef unsigned long long U;
    static W load(volatile W* p)
    {
#if defined(_M_X64)
        W v = *p;
        _ReadWriteBarrier();
        return v;
#else
        return _InterlockedCompareExchange64(p, 0, 0);
#endif
    }
    static W exchange(volatile W* p, W v) { return _InterlockedExchange64(p, v); }
    static W compareExchange(volatile W* p, W cmp, W v)
    {
        return _InterlockedCompareExchange64(p, v, cmp);
    }
    static W add(volatile W* p, W v) { return _InterlockedExchangeAdd64(p, v); }
    static W bitAnd(volatile W* p, W v) { return _InterlockedAnd64(p, v); }
    static W bitOr(volatile W* p, W v) { return _InterlockedOr64(p, v); }
    static W bitXor(volatile W* p, W v) { return _InterlockedXor64(p, v); }
};
// `_SlangAtomic<T>` is the typed layer the `_slang_atomic_*` helpers call, for
// a 4- or 8-byte T (u32, i32, u64, i64, f32, f64) at an address aligned to
// sizeof(T). On MSVC it copies T's bits into the word of the same size and
// lets `_SlangAtomicWord` do the atomic operation. `add`, `sub` and the
// bitwise operations are only used for the integer types; float add/sub go
// through `_slang_atomic_cas_loop`.
template<typename T>
struct _SlangAtomic
{
    typedef _SlangAtomicWord<sizeof(T)> Op;
    typedef typename Op::W W;
    static volatile W* word(T* p) { return reinterpret_cast<volatile W*>(p); }
    static W bits(T v)
    {
        W w;
        memcpy(&w, &v, sizeof(T));
        return w;
    }
    static T value(W w)
    {
        T v;
        memcpy(&v, &w, sizeof(T));
        return v;
    }
    static T load(T* p, int) { return value(Op::load(word(p))); }
    static void store(T* p, T v, int) { Op::exchange(word(p), bits(v)); }
    static T exchange(T* p, T v, int) { return value(Op::exchange(word(p), bits(v))); }
    static T compareExchange(T* p, T cmp, T v, int)
    {
        return value(Op::compareExchange(word(p), bits(cmp), bits(v)));
    }
    static bool sameBits(T a, T b) { return bits(a) == bits(b); }
    static T add(T* p, T v, int) { return value(Op::add(word(p), bits(v))); }
    // Subtract by adding the two's complement negation, computed unsigned so
    // that negating the most negative value wraps instead of overflowing.
    static T sub(T* p, T v, int)
    {
        typedef typename Op::U U;
        return value(Op::add(word(p), W(U(0) - U(bits(v)))));
    }
    static T bitAnd(T* p, T v, int) { return value(Op::bitAnd(word(p), bits(v))); }
    static T bitOr(T* p, T v, int) { return value(Op::bitOr(word(p), bits(v))); }
    static T bitXor(T* p, T v, int) { return value(Op::bitXor(word(p), bits(v))); }
};
#elif SLANG_GCC || SLANG_CLANG
static inline int _slang_atomic_order(int order)
{
    switch (order)
    {
    case 0:
        return __ATOMIC_RELAXED;
    case 1:
        return __ATOMIC_ACQUIRE;
    case 2:
        return __ATOMIC_RELEASE;
    case 3:
        return __ATOMIC_ACQ_REL;
    default:
        return __ATOMIC_SEQ_CST;
    }
}
// The order of a failed compare-exchange, which is only a load: no release.
static inline int _slang_atomic_failure_order(int order)
{
    return order == 2   ? __ATOMIC_RELAXED
           : order == 3 ? __ATOMIC_ACQUIRE
                        : _slang_atomic_order(order);
}
// `_SlangAtomic<T>` on GCC and Clang: the `__atomic` builtins work on T
// directly, with the requested memory order. Signed fetch-add/sub wrap
// around (the builtins are defined as two's complement).
template<typename T>
struct _SlangAtomic
{
    static T load(T* p, int o)
    {
        T r;
        __atomic_load(p, &r, _slang_atomic_order(o));
        return r;
    }
    static void store(T* p, T v, int o) { __atomic_store(p, &v, _slang_atomic_order(o)); }
    static T exchange(T* p, T v, int o)
    {
        T r;
        __atomic_exchange(p, &v, &r, _slang_atomic_order(o));
        return r;
    }
    static T compareExchange(T* p, T cmp, T v, int o)
    {
        // On failure `cmp` receives the current value, so either way it ends
        // up holding the prior value.
        __atomic_compare_exchange(
            p,
            &cmp,
            &v,
            false,
            _slang_atomic_order(o),
            _slang_atomic_failure_order(o));
        return cmp;
    }
    static bool sameBits(T a, T b) { return __builtin_memcmp(&a, &b, sizeof(T)) == 0; }
    static T add(T* p, T v, int o) { return __atomic_fetch_add(p, v, _slang_atomic_order(o)); }
    static T sub(T* p, T v, int o) { return __atomic_fetch_sub(p, v, _slang_atomic_order(o)); }
    static T bitAnd(T* p, T v, int o) { return __atomic_fetch_and(p, v, _slang_atomic_order(o)); }
    static T bitOr(T* p, T v, int o) { return __atomic_fetch_or(p, v, _slang_atomic_order(o)); }
    static T bitXor(T* p, T v, int o) { return __atomic_fetch_xor(p, v, _slang_atomic_order(o)); }
};
#else
// The unsigned integer type of a given size, for wrap-around arithmetic on a
// signed T in the fallback below.
template<int SIZE>
struct _SlangAtomicUnsigned;
template<>
struct _SlangAtomicUnsigned<4>
{
    typedef uint32_t U;
};
template<>
struct _SlangAtomicUnsigned<8>
{
    typedef uint64_t U;
};
// `_SlangAtomic<T>` without atomics: plain loads and stores, for compilers
// with neither the `_Interlocked*` intrinsics nor the `__atomic` builtins.
// Integer add/sub wrap around through the unsigned type of the same size.
template<typename T>
struct _SlangAtomic
{
    typedef typename _SlangAtomicUnsigned<sizeof(T)>::U U;
    static U bits(T v)
    {
        U u;
        memcpy(&u, &v, sizeof(T));
        return u;
    }
    static T value(U u)
    {
        T v;
        memcpy(&v, &u, sizeof(T));
        return v;
    }
    static T load(T* p, int) { return *p; }
    static void store(T* p, T v, int) { *p = v; }
    static T exchange(T* p, T v, int)
    {
        T old = *p;
        *p = v;
        return old;
    }
    static T compareExchange(T* p, T cmp, T v, int)
    {
        T old = *p;
        if (memcmp(&old, &cmp, sizeof(T)) == 0)
            *p = v;
        return old;
    }
    static bool sameBits(T a, T b) { return memcmp(&a, &b, sizeof(T)) == 0; }
    static T add(T* p, T v, int)
    {
        T old = *p;
        *p = value(U(bits(old) + bits(v)));
        return old;
    }
    static T sub(T* p, T v, int)
    {
        T old = *p;
        *p = value(U(bits(old) - bits(v)));
        return old;
    }
    static T bitAnd(T* p, T v, int)
    {
        T old = *p;
        *p = T(old & v);
        return old;
    }
    static T bitOr(T* p, T v, int)
    {
        T old = *p;
        *p = T(old | v);
        return old;
    }
    static T bitXor(T* p, T v, int)
    {
        T old = *p;
        *p = T(old ^ v);
        return old;
    }
};
#endif

// A read-modify-write without a native instruction, as a compare-and-swap
// loop: `f` computes the new value from the current one. The loop compares
// bit patterns, so a NaN in memory cannot make it spin.
template<typename T, typename F>
static inline T _slang_atomic_cas_loop(T* p, F f, int order)
{
    T old = _SlangAtomic<T>::load(p, 0);
    for (;;)
    {
        T prior = _SlangAtomic<T>::compareExchange(p, old, f(old), order);
        if (_SlangAtomic<T>::sameBits(prior, old))
            return old;
        old = prior;
    }
}
// The new-value functors for `_slang_atomic_cas_loop`: each holds the operand
// `v` and maps the current value to the value to store.
template<typename T>
struct _SlangAtomicMinOp
{
    T v;
    T operator()(T old) const { return v < old ? v : old; }
};
template<typename T>
struct _SlangAtomicMaxOp
{
    T v;
    T operator()(T old) const { return old < v ? v : old; }
};
template<typename T>
struct _SlangAtomicAddOp
{
    T v;
    T operator()(T old) const { return old + v; }
};

#define SLANG_CPP_ATOMIC_COMMON(SUFFIX, T)                                                   \
    static inline T _slang_atomic_load_##SUFFIX(T* p, int order = 0)                         \
    {                                                                                        \
        return _SlangAtomic<T>::load(p, order);                                              \
    }                                                                                        \
    static inline void _slang_atomic_store_##SUFFIX(T* p, T v, int order = 0)                \
    {                                                                                        \
        _SlangAtomic<T>::store(p, v, order);                                                 \
    }                                                                                        \
    static inline T _slang_atomic_exchange_##SUFFIX(T* p, T v, int order = 0)                \
    {                                                                                        \
        return _SlangAtomic<T>::exchange(p, v, order);                                       \
    }                                                                                        \
    static inline T _slang_atomic_compare_exchange_##SUFFIX(T* p, T cmp, T v, int order = 0) \
    {                                                                                        \
        return _SlangAtomic<T>::compareExchange(p, cmp, v, order);                           \
    }                                                                                        \
    static inline T _slang_atomic_min_##SUFFIX(T* p, T v, int order = 0)                     \
    {                                                                                        \
        _SlangAtomicMinOp<T> f = {v};                                                        \
        return _slang_atomic_cas_loop(p, f, order);                                          \
    }                                                                                        \
    static inline T _slang_atomic_max_##SUFFIX(T* p, T v, int order = 0)                     \
    {                                                                                        \
        _SlangAtomicMaxOp<T> f = {v};                                                        \
        return _slang_atomic_cas_loop(p, f, order);                                          \
    }

#define SLANG_CPP_ATOMIC_INTEGER(SUFFIX, T)                              \
    SLANG_CPP_ATOMIC_COMMON(SUFFIX, T)                                   \
    static inline T _slang_atomic_add_##SUFFIX(T* p, T v, int order = 0) \
    {                                                                    \
        return _SlangAtomic<T>::add(p, v, order);                        \
    }                                                                    \
    static inline T _slang_atomic_sub_##SUFFIX(T* p, T v, int order = 0) \
    {                                                                    \
        return _SlangAtomic<T>::sub(p, v, order);                        \
    }                                                                    \
    static inline T _slang_atomic_and_##SUFFIX(T* p, T v, int order = 0) \
    {                                                                    \
        return _SlangAtomic<T>::bitAnd(p, v, order);                     \
    }                                                                    \
    static inline T _slang_atomic_or_##SUFFIX(T* p, T v, int order = 0)  \
    {                                                                    \
        return _SlangAtomic<T>::bitOr(p, v, order);                      \
    }                                                                    \
    static inline T _slang_atomic_xor_##SUFFIX(T* p, T v, int order = 0) \
    {                                                                    \
        return _SlangAtomic<T>::bitXor(p, v, order);                     \
    }

#define SLANG_CPP_ATOMIC_FLOAT(SUFFIX, T)                                \
    SLANG_CPP_ATOMIC_COMMON(SUFFIX, T)                                   \
    static inline T _slang_atomic_add_##SUFFIX(T* p, T v, int order = 0) \
    {                                                                    \
        _SlangAtomicAddOp<T> f = {v};                                    \
        return _slang_atomic_cas_loop(p, f, order);                      \
    }                                                                    \
    static inline T _slang_atomic_sub_##SUFFIX(T* p, T v, int order = 0) \
    {                                                                    \
        _SlangAtomicAddOp<T> f = {-v};                                   \
        return _slang_atomic_cas_loop(p, f, order);                      \
    }

SLANG_CPP_ATOMIC_INTEGER(u32, uint32_t)
SLANG_CPP_ATOMIC_INTEGER(i32, int32_t)
SLANG_CPP_ATOMIC_INTEGER(u64, uint64_t)
SLANG_CPP_ATOMIC_INTEGER(i64, int64_t)
SLANG_CPP_ATOMIC_FLOAT(f32, float)
SLANG_CPP_ATOMIC_FLOAT(f64, double)

#undef SLANG_CPP_ATOMIC_COMMON
#undef SLANG_CPP_ATOMIC_INTEGER
#undef SLANG_CPP_ATOMIC_FLOAT

// TODO(JS): Hack! Output C++ code from slang can copy uninitialized variables.
#if defined(_MSC_VER)
#pragma warning(disable : 4700)
#endif

#ifndef SLANG_UNROLL
#define SLANG_UNROLL
#endif

#endif
