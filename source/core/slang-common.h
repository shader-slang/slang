#pragma once

#include "slang-signal.h"
#include "slang.h"

#include <assert.h>
#include <cstddef>
#include <cstring>
#include <stdint.h>
#include <type_traits>

#define VARIADIC_TEMPLATE

namespace Slang
{

/// Signed 32-bit integer.
///
/// This type should be used when the exact size
/// in bits is important (e.g., when dealing with
/// explicit binary file formats, etc.). Otherwise
/// prefer the plain `Int` type or a semantically
/// richer type like `Count` or `Index`.
///
typedef int32_t Int32;

/// Unsigned 32-bit integer.
///
/// This type should be used when the exact size
/// in bits is important (e.g., when dealing with
/// explicit binary file formats, etc.). Otherwise
/// prefer the plain `Int` type or a semantically
/// richer type like `Count` or `Index`.
///
typedef uint32_t UInt32;

/// Signed 64-bit integer.
///
/// This type should be used when the exact size
/// in bits is important (e.g., when dealing with
/// explicit binary file formats, etc.). Otherwise
/// prefer the plain `Int` type or a semantically
/// richer type like `Count` or `Index`.
///
typedef int64_t Int64;

/// Unsigned 64-bit integer.
///
/// This type should be used when the exact size
/// in bits is important (e.g., when dealing with
/// explicit binary file formats, etc.). Otherwise
/// prefer the plain `Int` type or a semantically
/// richer type like `Count` or `Index`.
///
typedef uint64_t UInt64;

/// "Default" integer type for the Slang codebase.
///
/// When there is not a clear reason to another
/// integer type, use this one.
///
/// Note that this type is currently defined to be
/// the same as the `SlangInt` type exposed through
/// the public Slang API, but this may not be the
/// case forever.
///
typedef SlangInt Int;

/// "Default" unsigned integer type for the Slang codebase.
///
/// Only use this type when you explicitly need
/// an unsigned type that's the same size as `Int`.
/// Otherwise you should probably just be using `Int`.
///
/// Note that this type is currently defined to be
/// the same as the `SlangUInt` type exposed through
/// the public Slang API, but this may not be the
/// case forever.
///
typedef SlangUInt UInt;

static const UInt kMaxUInt = ~UInt(0);
static const Int kMaxInt = Int(kMaxUInt >> 1);

typedef intptr_t PtrInt;

/// Default type for indices.
///
/// This is (and should always be) an alias for `Int`.
///
/// Use this type to document the intention that an
/// integer parameter/variable/etc. represents an
/// index into some kind of sequence, or any other
/// kind of ordinal number.
///
typedef Int Index;

static const Index kMaxIndex = kMaxInt;

/// Unsigned equivalent of `Index`.
///
/// Please don't use this unless you have a good reason.
///
typedef UInt UIndex;

/// Default type for counts.
///
/// This is (and should always be) an alias for `Int`.
///
/// Use this type to document the intention that an
/// integer parameter/variable/etc. represents a
/// count of the number of elements in some container,
/// or any other kind of cardinal number.
///
typedef Int Count;

/// Unsigned equivalent of `Count`.
///
/// Please don't use this unless you have a good reason.
///
typedef UInt UCount;


/// Explicit type for when manipulating bytes.
///
/// Use this type to document the intention that a
/// parameter/variable/etc. represents an 8-bit byte, with
/// no particular interpretation of that byte as
/// any higher-level type.
///
/// Note that the `char` types have special semantics
/// when it comes to "type punning" that are not shared
/// with other types like `uint8_t`. Using a variation
/// of `char` here helps avoid the possibility of undefined
/// behavior when code reads other types to/from arrays
/// of `Byte`s.
///
/// We are not using `std::byte` here because that is
/// defined as an `enum class` and does not support
/// mathematical or bitwise operations, which a lot
/// of the Slang codebase does on `Byte`s.
///
typedef unsigned char Byte;

/// Preferred integer type for sizes measured in bytes.
///
/// Use this type to document the intention that an
/// integer parameter/variable/etc. represents the
/// size of something, in bytes, rather than being
/// some other kind of integer.
///
/// Note that this type is unsigned, despite the stated
/// default in the Slang codebase being signed integer
/// types. The reason for this is that variables
/// holding sizes are often compared against the
/// result of the `sizeof` operator, which yields
/// a `size_t`. Our hands are, to some extent, tied
/// on this matter.
///
using Size = size_t;

// TODO(JS):
// Perhaps these should be named Utf8, Utf16 and UnicodePoint/Rune/etc? For now, just keep it simple
//
typedef char Char8;
// 16 bit character. Note much like in utf8, a character may or may not represent a code point (it
// can be part of a code point).
typedef uint16_t Char16;

// Can always hold a unicode code point.
typedef uint32_t Char32;

template<typename T>
inline T&& _Move(T& obj)
{
    return static_cast<T&&>(obj);
}

template<typename T>
inline void Swap(T& v0, T& v1)
{
    T tmp = _Move(v0);
    v0 = _Move(v1);
    v1 = _Move(tmp);
}

// Make these interfaces have more convenient names
typedef ISlangCastable ICastable;
typedef ISlangClonable IClonable;

// Convenience function for using clonable
template<typename T>
SLANG_FORCE_INLINE T* clone(IClonable* clonable)
{
    return (T*)clonable->clone(T::getTypeGuid());
}

template<typename T>
inline bool isBitSet(T value, T bitToTest)
{
    static_assert(sizeof(T) <= sizeof(uint32_t), "Only support up to 32 bit enums");
    return (T)((uint32_t)value & (uint32_t)bitToTest) == bitToTest;
}

template<typename To, typename From>
typename std::enable_if_t<
    sizeof(To) == sizeof(From) && std::is_trivially_copyable_v<From> &&
        std::is_trivially_copyable_v<To> && std::is_trivially_constructible_v<To>,
    To>
bitCast(const From& src)
{
    To dst;
    std::memcpy(&dst, &src, sizeof(To));
    return dst;
}

} // namespace Slang

// SLANG_DEFER
template<typename F>
class SlangDeferImpl
{
    F f;

public:
    SlangDeferImpl(F&& f)
        : f(Slang::_Move(f))
    {
    }
    ~SlangDeferImpl() { f(); }
};

#ifndef SLANG_DEFER_LAMBDA
#define SLANG_DEFER_LAMBDA(x) auto SLANG_CONCAT(slang_defer_, __LINE__) = SlangDeferImpl(x)
#define SLANG_DEFER(x) auto SLANG_CONCAT(slang_defer_, __LINE__) = SlangDeferImpl([&]() { x; })
#endif

//
// Some macros for avoiding boilerplate
// TODO: could probably deduce the size with templates, and move the whole
// thing into a template
//
#if __cplusplus >= 202002L
#define SLANG_COMPONENTWISE_EQUALITY_1(type) bool operator==(const type& other) const = default;
#define SLANG_COMPONENTWISE_EQUALITY_2(type) bool operator==(const type& other) const = default;
#define SLANG_COMPONENTWISE_EQUALITY_3(type) bool operator==(const type& other) const = default;
#else
#define SLANG_COMPONENTWISE_EQUALITY_1(type) \
    bool operator==(const type& other) const \
    {                                        \
        const auto& [m1] = *this;            \
        const auto& [o1] = other;            \
        return m1 == o1;                     \
    }                                        \
    bool operator!=(const type& other) const \
    {                                        \
        return !(*this == other);            \
    }

#define SLANG_COMPONENTWISE_EQUALITY_2(type) \
    bool operator==(const type& other) const \
    {                                        \
        const auto& [m1, m2] = *this;        \
        const auto& [o1, o2] = other;        \
        return m1 == o1 && m2 == o2;         \
    }                                        \
    bool operator!=(const type& other) const \
    {                                        \
        return !(*this == other);            \
    }

#define SLANG_COMPONENTWISE_EQUALITY_3(type)     \
    bool operator==(const type& other) const     \
    {                                            \
        const auto& [m1, m2, m3] = *this;        \
        const auto& [o1, o2, o3] = other;        \
        return m1 == o1 && m2 == o2 && m3 == o3; \
    }                                            \
    bool operator!=(const type& other) const     \
    {                                            \
        return !(*this == other);                \
    }
#endif

// TODO: Shouldn't these be SLANG_ prefixed?
#ifdef _MSC_VER
#define UNREACHABLE_RETURN(x)
#else
#define UNREACHABLE_RETURN(x) return x;
#endif

#if SLANG_GCC
#define SLANG_EXHAUSTIVE_SWITCH_BEGIN \
    _Pragma("GCC diagnostic push");   \
    _Pragma("GCC diagnostic error \"-Wswitch-enum\"");
#define SLANG_EXHAUSTIVE_SWITCH_END _Pragma("GCC diagnostic pop");
#elif SLANG_CLANG
#define SLANG_EXHAUSTIVE_SWITCH_BEGIN \
    _Pragma("clang diagnostic push"); \
    _Pragma("clang diagnostic error \"-Wswitch-enum\"");
#define SLANG_EXHAUSTIVE_SWITCH_END _Pragma("clang diagnostic pop");
#elif SLANG_VC
#define SLANG_EXHAUSTIVE_SWITCH_BEGIN \
    _Pragma("warning(push)");         \
    _Pragma("warning(error : 4062)");
#define SLANG_EXHAUSTIVE_SWITCH_END _Pragma("warning(pop)");
#else
#define SLANG_EXHAUSTIVE_SWITCH_BEGIN
#define SLANG_EXHAUSTIVE_SWITCH_END
#endif

#if SLANG_GCC
#define SLANG_ALLOW_DEPRECATED_BEGIN \
    _Pragma("GCC diagnostic push");  \
    _Pragma("GCC diagnostic ignored \"-Wdeprecated-declarations\"");
#define SLANG_ALLOW_DEPRECATED_END _Pragma("GCC diagnostic pop");
#elif SLANG_CLANG
#define SLANG_ALLOW_DEPRECATED_BEGIN  \
    _Pragma("clang diagnostic push"); \
    _Pragma("clang diagnostic ignored \"-Wdeprecated-declarations\"");
#define SLANG_ALLOW_DEPRECATED_END _Pragma("clang diagnostic pop");
#elif SLANG_VC
#define SLANG_ALLOW_DEPRECATED_BEGIN \
    _Pragma("warning(push)");        \
    _Pragma("warning(disable : 4996)");
#define SLANG_ALLOW_DEPRECATED_END _Pragma("warning(pop)");
#else
#define SLANG_ALLOW_DEPRECATED_BEGIN
#define SLANG_ALLOW_DEPRECATED_END
#endif

//
// Use `SLANG_ASSUME(myBoolExpression);` to inform the compiler that the condition is true.
// Do not rely on side effects of the condition being performed.
//
#if defined(__cpp_assume)
#define SLANG_ASSUME(X) [[assume(X)]]
#elif SLANG_GCC
#define SLANG_ASSUME(X)              \
    do                               \
    {                                \
        if (!(X))                    \
            __builtin_unreachable(); \
    } while (0)
#elif SLANG_CLANG
#define SLANG_ASSUME(X) __builtin_assume(X)
#elif SLANG_VC
#define SLANG_ASSUME(X) __assume(X)
#else
[[noreturn]] inline void invokeUndefinedBehaviour() {}
#define SLANG_ASSUME(X)                 \
    do                                  \
    {                                   \
        if (!(X))                       \
            invokeUndefinedBehaviour(); \
    } while (0)
#endif

//
// Assertions abort in debug builds, but inform the compiler of true
// assumptions in release builds
//
#ifdef _DEBUG
#define SLANG_ASSERT(VALUE)               \
    do                                    \
    {                                     \
        if (!(VALUE)) [[unlikely]]        \
            SLANG_ASSERT_FAILURE(#VALUE); \
    } while (0)
#else
#define SLANG_ASSERT(VALUE) SLANG_ASSUME(VALUE)
#endif

#define SLANG_RELEASE_ASSERT(VALUE) \
    if (VALUE) [[likely]]           \
    {                               \
    }                               \
    else                            \
        SLANG_ASSERT_FAILURE(#VALUE)

template<typename T>
void slang_use_obj(T&)
{
}

#define SLANG_UNREFERENCED_PARAMETER(P) slang_use_obj(P)
#define SLANG_UNREFERENCED_VARIABLE(P) slang_use_obj(P)

#if defined(SLANG_RT_DYNAMIC)
#if defined(_MSC_VER)
#ifdef SLANG_RT_DYNAMIC_EXPORT
#define SLANG_RT_API SLANG_DLL_EXPORT
#else
#define SLANG_RT_API __declspec(dllimport)
#endif
#else
// TODO: need to consider compiler capabilities
// #     ifdef SLANG_RT_DYNAMIC_EXPORT
#define SLANG_RT_API SLANG_DLL_EXPORT
// #     endif
#endif
#endif

#if defined(_MSC_VER)
#define SLANG_ATTR_PRINTF(string_index, varargs_index)
#else
#define SLANG_ATTR_PRINTF(string_index, varargs_index) \
    __attribute__((format(printf, string_index, varargs_index)))
#endif

#ifndef SLANG_RT_API
#define SLANG_RT_API
#endif
