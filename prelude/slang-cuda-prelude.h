#ifndef SLANG_CUDA_PRELUDE_H
#define SLANG_CUDA_PRELUDE_H

#define SLANG_PRELUDE_EXPORT

#ifdef __CUDACC_RTC__
#define SLANG_CUDA_RTC 1
#else
#define SLANG_CUDA_RTC 0
#endif

#if SLANG_CUDA_RTC

#else

#include <cstdint>
#include <stdio.h>

#endif

// Define SLANG_CUDA_ENABLE_HALF to use the cuda_fp16 include to add half support.
// For this to work NVRTC needs to have the path to the CUDA SDK.
//
// As it stands the includes paths defined for Slang are passed down to NVRTC. Similarly defines
// defined for the Slang compile are passed down.

#ifdef SLANG_CUDA_ENABLE_HALF
// We don't want half2 operators from cuda_fp16.h (comparison returns bool). Arithmetic for
// __half2 is defined in the macro SLANG_CUDA_VECTOR_FLOAT_OP_HALF2 below (CUDA intrinsics).
#define __CUDA_NO_HALF2_OPERATORS__
#include <cuda_fp16.h>
#endif

#ifdef SLANG_CUDA_ENABLE_FP8
#include <cuda_fp8.h>
#endif

#ifdef SLANG_CUDA_ENABLE_BF16
#include <cuda_bf16.h>
#endif

#ifdef SLANG_CUDA_ENABLE_OPTIX
#include <optix.h>
#endif

// Define slang offsetof implementation
#ifndef SLANG_OFFSET_OF
#define SLANG_OFFSET_OF(type, member) (size_t)((char*)&(((type*)0)->member) - (char*)0)
#endif

// Must be large enough to cause overflow and therefore infinity
#ifndef SLANG_INFINITY
#define SLANG_INFINITY ((float)(1e+300 * 1e+300))
#endif

// For now we'll disable any asserts in this prelude
#define SLANG_PRELUDE_ASSERT(x)

#ifndef SLANG_CUDA_WARP_SIZE
#define SLANG_CUDA_WARP_SIZE 32
#endif

#define SLANG_CUDA_WARP_MASK \
    (SLANG_CUDA_WARP_SIZE - 1) // Used for masking threadIdx.x to the warp lane index
#define SLANG_CUDA_WARP_BITMASK (~int(0))

//
#define SLANG_FORCE_INLINE inline

#define SLANG_CUDA_CALL __device__

#define SLANG_FORCE_INLINE inline
#define SLANG_INLINE inline


// Since we are using unsigned arithmatic care is need in this comparison.
// It is *assumed* that sizeInBytes >= elemSize. Which means (sizeInBytes >= elemSize) >= 0
// Which means only a single test is needed

// Asserts for bounds checking.
// It is assumed index/count are unsigned types.
#define SLANG_BOUND_ASSERT(index, count) SLANG_PRELUDE_ASSERT(index < count);
#define SLANG_BOUND_ASSERT_BYTE_ADDRESS(index, elemSize, sizeInBytes) \
    SLANG_PRELUDE_ASSERT(index <= (sizeInBytes - elemSize) && (index & 3) == 0);

// Macros to zero index if an access is out of range
#define SLANG_BOUND_ZERO_INDEX(index, count) index = (index < count) ? index : 0;
#define SLANG_BOUND_ZERO_INDEX_BYTE_ADDRESS(index, elemSize, sizeInBytes) \
    index = (index <= (sizeInBytes - elemSize)) ? index : 0;

// The 'FIX' macro define how the index is fixed. The default is to do nothing. If
// SLANG_ENABLE_BOUND_ZERO_INDEX the fix macro will zero the index, if out of range
#ifdef SLANG_ENABLE_BOUND_ZERO_INDEX
#define SLANG_BOUND_FIX(index, count) SLANG_BOUND_ZERO_INDEX(index, count)
#define SLANG_BOUND_FIX_BYTE_ADDRESS(index, elemSize, sizeInBytes) \
    SLANG_BOUND_ZERO_INDEX_BYTE_ADDRESS(index, elemSize, sizeInBytes)
#define SLANG_BOUND_FIX_FIXED_ARRAY(index, count) \
    SLANG_BOUND_ZERO_INDEX(index, count) SLANG_BOUND_ZERO_INDEX(index, count)
#else
#define SLANG_BOUND_FIX(index, count)
#define SLANG_BOUND_FIX_BYTE_ADDRESS(index, elemSize, sizeInBytes)
#define SLANG_BOUND_FIX_FIXED_ARRAY(index, count)
#endif

#ifndef SLANG_BOUND_CHECK
#define SLANG_BOUND_CHECK(index, count) \
    SLANG_BOUND_ASSERT(index, count) SLANG_BOUND_FIX(index, count)
#endif

#ifndef SLANG_BOUND_CHECK_BYTE_ADDRESS
#define SLANG_BOUND_CHECK_BYTE_ADDRESS(index, elemSize, sizeInBytes) \
    SLANG_BOUND_ASSERT_BYTE_ADDRESS(index, elemSize, sizeInBytes)    \
    SLANG_BOUND_FIX_BYTE_ADDRESS(index, elemSize, sizeInBytes)
#endif

#ifndef SLANG_BOUND_CHECK_FIXED_ARRAY
#define SLANG_BOUND_CHECK_FIXED_ARRAY(index, count) \
    SLANG_BOUND_ASSERT(index, count) SLANG_BOUND_FIX_FIXED_ARRAY(index, count)
#endif

// This macro handles how out-of-range surface coordinates are handled;
// I can equal
// cudaBoundaryModeClamp, in which case out-of-range coordinates are clamped to the valid range
// cudaBoundaryModeZero, in which case out-of-range reads return zero and out-of-range writes are
// ignored cudaBoundaryModeTrap, in which case out-of-range accesses cause the kernel execution to
// fail.

#ifndef SLANG_CUDA_BOUNDARY_MODE
#define SLANG_CUDA_BOUNDARY_MODE cudaBoundaryModeZero

// Can be one of SLANG_CUDA_PTX_BOUNDARY_MODE. Only applies *PTX* emitted CUDA operations
// which currently is just RWTextureRW format writes
//
// .trap         causes an execution trap on out-of-bounds addresses
// .clamp        stores data at the nearest surface location (sized appropriately)
// .zero         drops stores to out-of-bounds addresses

#define SLANG_PTX_BOUNDARY_MODE "zero"
#endif

struct TypeInfo
{
    size_t typeSize;
};

template<typename T, size_t SIZE>
struct FixedArray
{
    SLANG_CUDA_CALL const T& operator[](size_t index) const
    {
        SLANG_BOUND_CHECK_FIXED_ARRAY(index, SIZE);
        return m_data[index];
    }
    SLANG_CUDA_CALL T& operator[](size_t index)
    {
        SLANG_BOUND_CHECK_FIXED_ARRAY(index, SIZE);
        return m_data[index];
    }

    T m_data[SIZE];
};

// An array that has no specified size, becomes a 'Array'. This stores the size so it can
// potentially do bounds checking.
template<typename T>
struct Array
{
    SLANG_CUDA_CALL const T& operator[](size_t index) const
    {
        SLANG_BOUND_CHECK(index, count);
        return data[index];
    }
    SLANG_CUDA_CALL T& operator[](size_t index)
    {
        SLANG_BOUND_CHECK(index, count);
        return data[index];
    }

    T* data;
    size_t count;
};

// Typically defined in cuda.h, but we can't ship/rely on that, so just define here
typedef unsigned long long CUtexObject;
typedef unsigned long long CUsurfObject;

// On CUDA sampler state is actually bound up with the texture object. We have a SamplerState type,
// backed as a pointer, to simplify code generation, with the downside that such a binding will take
// up uniform space, even though it will have no effect.
// TODO(JS): Consider ways to strip use of variables of this type so have no binding,
struct SamplerStateUnused;
typedef SamplerStateUnused* SamplerState;


// TODO(JS): Not clear yet if this can be handled on CUDA, by just ignoring.
// For now, just map to the index type.
typedef size_t NonUniformResourceIndex;

// Code generator will generate the specific type
template<typename T, int ROWS, int COLS>
struct Matrix;

// Boolean vector types should follow CUDA's builtin vector alignment rules
// Align boolX the same as charX according to CUDA spec:
// char1/uchar1: 1-byte aligned, char2/uchar2: 2-byte aligned
// char3/uchar3: 1-byte aligned, char4/uchar4: 4-byte aligned
struct __align__(1) bool1
{
    bool x;

    SLANG_FORCE_INLINE SLANG_CUDA_CALL bool& operator[](int idx)
    {
        return (&x)[idx];
    }
    SLANG_FORCE_INLINE SLANG_CUDA_CALL const bool& operator[](int idx) const
    {
        return (&x)[idx];
    }
};

struct __align__(2) bool2
{
    bool x, y;

    SLANG_FORCE_INLINE SLANG_CUDA_CALL bool& operator[](int idx)
    {
        return (&x)[idx];
    }
    SLANG_FORCE_INLINE SLANG_CUDA_CALL const bool& operator[](int idx) const
    {
        return (&x)[idx];
    }
};

struct __align__(1) bool3
{
    bool x, y, z;

    SLANG_FORCE_INLINE SLANG_CUDA_CALL bool& operator[](int idx)
    {
        return (&x)[idx];
    }
    SLANG_FORCE_INLINE SLANG_CUDA_CALL const bool& operator[](int idx) const
    {
        return (&x)[idx];
    }
};

struct __align__(4) bool4
{
    bool x, y, z, w;

    SLANG_FORCE_INLINE SLANG_CUDA_CALL bool& operator[](int idx)
    {
        return (&x)[idx];
    }
    SLANG_FORCE_INLINE SLANG_CUDA_CALL const bool& operator[](int idx) const
    {
        return (&x)[idx];
    }
};

SLANG_FORCE_INLINE SLANG_CUDA_CALL bool __ldg(const bool* ptr)
{
    return (bool)(__ldg((const char*)ptr));
}

SLANG_FORCE_INLINE SLANG_CUDA_CALL bool2 __ldg(const bool2* ptr)
{
    auto val = __ldg((const char2*)ptr);
    return {val.x != 0, val.y != 0};
}

SLANG_FORCE_INLINE SLANG_CUDA_CALL bool4 __ldg(const bool4* ptr)
{
    auto val = __ldg((const char4*)ptr);
    return {val.x != 0, val.y != 0, val.z != 0, val.w != 0};
}

#if SLANG_CUDA_RTC

typedef signed char int8_t;
typedef short int16_t;
typedef int int32_t;
typedef long long int64_t;
typedef ptrdiff_t intptr_t;

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;
typedef size_t uintptr_t;

typedef long long longlong;
typedef unsigned long long ulonglong;

#else

// When not using NVRTC, match the platform's int64_t definition for signed type
// On Linux: int64_t is 'long', on Windows: int64_t is 'long long'
typedef int64_t longlong;
// ulonglong must remain 'unsigned long long' to match CUDA's atomic operations
typedef unsigned long long ulonglong;

#endif

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;

#if SLANG_CUDA_ENABLE_HALF
typedef __half half;
#endif

union Union32
{
    uint32_t u;
    int32_t i;
    float f;
};

union Union64
{
    uint64_t u;
    int64_t i;
    double d;
};

template<typename T>
SLANG_FORCE_INLINE SLANG_CUDA_CALL float make_float(T val)
{
    return (float)val;
}

SLANG_FORCE_INLINE SLANG_CUDA_CALL float _slang_fmod(float x, float y)
{
    return ::fmodf(x, y);
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL double _slang_fmod(double x, double y)
{
    return ::fmod(x, y);
}

#if SLANG_CUDA_ENABLE_HALF

// Add the other vector half types
struct __half1
{
    __half x;
};
struct __align__(4) __half3
{
    __half x, y, z;
};
struct __align__(4) __half4
{
    __half x, y, z, w;
};
#endif

#if SLANG_CUDA_ENABLE_BF16

// Add the other vector bfloat16 types
struct __nv_bfloat161
{
    __nv_bfloat16 x;
};
struct __nv_bfloat163
{
    __nv_bfloat16 x, y, z;
};
struct __nv_bfloat164
{
    __nv_bfloat16 x, y, z, w;
};
#endif

#if SLANG_CUDA_ENABLE_FP8

// Add the other vector fp8 types
struct __nv_fp8_e4m31
{
    __nv_fp8_e4m3 x;
};
struct __nv_fp8_e4m32
{
    __nv_fp8_e4m3 x, y;
};
struct __nv_fp8_e4m33
{
    __nv_fp8_e4m3 x, y, z;
};
struct __nv_fp8_e4m34
{
    __nv_fp8_e4m3 x, y, z, w;
};
struct __nv_fp8_e5m21
{
    __nv_fp8_e5m2 x;
};
struct __nv_fp8_e5m22
{
    __nv_fp8_e5m2 x, y;
};
struct __nv_fp8_e5m23
{
    __nv_fp8_e5m2 x, y, z;
};
struct __nv_fp8_e5m24
{
    __nv_fp8_e5m2 x, y, z, w;
};
#endif

#define SLANG_VECTOR_GET_ELEMENT(T)                                                   \
    SLANG_FORCE_INLINE SLANG_CUDA_CALL T _slang_vector_get_element(T##1 x, int index) \
    {                                                                                 \
        return ((T*)(&x))[index];                                                     \
    }                                                                                 \
    SLANG_FORCE_INLINE SLANG_CUDA_CALL T _slang_vector_get_element(T##2 x, int index) \
    {                                                                                 \
        return ((T*)(&x))[index];                                                     \
    }                                                                                 \
    SLANG_FORCE_INLINE SLANG_CUDA_CALL T _slang_vector_get_element(T##3 x, int index) \
    {                                                                                 \
        return ((T*)(&x))[index];                                                     \
    }                                                                                 \
    SLANG_FORCE_INLINE SLANG_CUDA_CALL T _slang_vector_get_element(T##4 x, int index) \
    {                                                                                 \
        return ((T*)(&x))[index];                                                     \
    }
SLANG_VECTOR_GET_ELEMENT(int)
SLANG_VECTOR_GET_ELEMENT(bool)
SLANG_VECTOR_GET_ELEMENT(uint)
SLANG_VECTOR_GET_ELEMENT(short)
SLANG_VECTOR_GET_ELEMENT(ushort)
SLANG_VECTOR_GET_ELEMENT(char)
SLANG_VECTOR_GET_ELEMENT(uchar)
SLANG_VECTOR_GET_ELEMENT(longlong)
SLANG_VECTOR_GET_ELEMENT(ulonglong)
SLANG_VECTOR_GET_ELEMENT(float)
SLANG_VECTOR_GET_ELEMENT(double)

#define SLANG_VECTOR_GET_ELEMENT_PTR(T)                                                            \
    SLANG_FORCE_INLINE SLANG_CUDA_CALL T* _slang_vector_get_element_ptr(const T##1 * x, int index) \
    {                                                                                              \
        return ((T*)(x)) + index;                                                                  \
    }                                                                                              \
    SLANG_FORCE_INLINE SLANG_CUDA_CALL T* _slang_vector_get_element_ptr(const T##2 * x, int index) \
    {                                                                                              \
        return ((T*)(x)) + index;                                                                  \
    }                                                                                              \
    SLANG_FORCE_INLINE SLANG_CUDA_CALL T* _slang_vector_get_element_ptr(const T##3 * x, int index) \
    {                                                                                              \
        return ((T*)(x)) + index;                                                                  \
    }                                                                                              \
    SLANG_FORCE_INLINE SLANG_CUDA_CALL T* _slang_vector_get_element_ptr(const T##4 * x, int index) \
    {                                                                                              \
        return ((T*)(x)) + index;                                                                  \
    }
SLANG_VECTOR_GET_ELEMENT_PTR(int)
SLANG_VECTOR_GET_ELEMENT_PTR(bool)
SLANG_VECTOR_GET_ELEMENT_PTR(uint)
SLANG_VECTOR_GET_ELEMENT_PTR(short)
SLANG_VECTOR_GET_ELEMENT_PTR(ushort)
SLANG_VECTOR_GET_ELEMENT_PTR(char)
SLANG_VECTOR_GET_ELEMENT_PTR(uchar)
SLANG_VECTOR_GET_ELEMENT_PTR(longlong)
SLANG_VECTOR_GET_ELEMENT_PTR(ulonglong)
SLANG_VECTOR_GET_ELEMENT_PTR(float)
SLANG_VECTOR_GET_ELEMENT_PTR(double)

#if SLANG_CUDA_ENABLE_HALF
SLANG_VECTOR_GET_ELEMENT(__half)
SLANG_VECTOR_GET_ELEMENT_PTR(__half)
#endif

#if SLANG_CUDA_ENABLE_BF16
SLANG_VECTOR_GET_ELEMENT(__nv_bfloat16)
SLANG_VECTOR_GET_ELEMENT_PTR(__nv_bfloat16)

SLANG_FORCE_INLINE SLANG_CUDA_CALL __nv_bfloat16
_slang_vector_dot(__nv_bfloat162 v0, __nv_bfloat162 v1)
{
    __nv_bfloat16 result = __nv_bfloat16(0.0f);
    for (int i = 0; i < 2; i++)
    {
        result += _slang_vector_get_element(v0, i) * _slang_vector_get_element(v1, i);
    }
    return result;
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL __nv_bfloat16
_slang_vector_dot(__nv_bfloat163 v0, __nv_bfloat163 v1)
{
    __nv_bfloat16 result = __nv_bfloat16(0.0f);
    for (int i = 0; i < 3; i++)
    {
        result += _slang_vector_get_element(v0, i) * _slang_vector_get_element(v1, i);
    }
    return result;
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL __nv_bfloat16
_slang_vector_dot(__nv_bfloat164 v0, __nv_bfloat164 v1)
{
    __nv_bfloat16 result = __nv_bfloat16(0.0f);
    for (int i = 0; i < 4; i++)
    {
        result += _slang_vector_get_element(v0, i) * _slang_vector_get_element(v1, i);
    }
    return result;
}
#endif

#if SLANG_CUDA_ENABLE_FP8
SLANG_VECTOR_GET_ELEMENT(__nv_fp8_e4m3)
SLANG_VECTOR_GET_ELEMENT_PTR(__nv_fp8_e4m3)
SLANG_VECTOR_GET_ELEMENT(__nv_fp8_e5m2)
SLANG_VECTOR_GET_ELEMENT_PTR(__nv_fp8_e5m2)
#endif

#define SLANG_CUDA_VECTOR_BINARY_OP(T, n, op)                                                 \
    SLANG_FORCE_INLINE SLANG_CUDA_CALL T##n operator op(T##n thisVal, T##n other)             \
    {                                                                                         \
        T##n result;                                                                          \
        for (int i = 0; i < n; i++)                                                           \
            *_slang_vector_get_element_ptr(&result, i) =                                      \
                _slang_vector_get_element(thisVal, i) op _slang_vector_get_element(other, i); \
        return result;                                                                        \
    }
#define SLANG_CUDA_VECTOR_BINARY_COMPARE_OP(T, n, op)                                           \
    SLANG_FORCE_INLINE SLANG_CUDA_CALL bool##n operator op(T##n thisVal, T##n other)            \
    {                                                                                           \
        bool##n result;                                                                         \
        for (int i = 0; i < n; i++)                                                             \
            *_slang_vector_get_element_ptr(&result, i) =                                        \
                (_slang_vector_get_element(thisVal, i) op _slang_vector_get_element(other, i)); \
        return result;                                                                          \
    }
#define SLANG_CUDA_VECTOR_UNARY_OP(T, n, op)                                                       \
    SLANG_FORCE_INLINE SLANG_CUDA_CALL T##n operator op(T##n thisVal)                              \
    {                                                                                              \
        T##n result;                                                                               \
        for (int i = 0; i < n; i++)                                                                \
            *_slang_vector_get_element_ptr(&result, i) = op _slang_vector_get_element(thisVal, i); \
        return result;                                                                             \
    }

#define SLANG_CUDA_VECTOR_INT_OP(T, n)            \
    SLANG_CUDA_VECTOR_BINARY_OP(T, n, +)          \
    SLANG_CUDA_VECTOR_BINARY_OP(T, n, -)          \
    SLANG_CUDA_VECTOR_BINARY_OP(T, n, *)          \
    SLANG_CUDA_VECTOR_BINARY_OP(T, n, /)          \
    SLANG_CUDA_VECTOR_BINARY_OP(T, n, %)          \
    SLANG_CUDA_VECTOR_BINARY_OP(T, n, ^)          \
    SLANG_CUDA_VECTOR_BINARY_OP(T, n, &)          \
    SLANG_CUDA_VECTOR_BINARY_OP(T, n, |)          \
    SLANG_CUDA_VECTOR_BINARY_OP(T, n, &&)         \
    SLANG_CUDA_VECTOR_BINARY_OP(T, n, ||)         \
    SLANG_CUDA_VECTOR_BINARY_OP(T, n, >>)         \
    SLANG_CUDA_VECTOR_BINARY_OP(T, n, <<)         \
    SLANG_CUDA_VECTOR_BINARY_COMPARE_OP(T, n, >)  \
    SLANG_CUDA_VECTOR_BINARY_COMPARE_OP(T, n, <)  \
    SLANG_CUDA_VECTOR_BINARY_COMPARE_OP(T, n, >=) \
    SLANG_CUDA_VECTOR_BINARY_COMPARE_OP(T, n, <=) \
    SLANG_CUDA_VECTOR_BINARY_COMPARE_OP(T, n, ==) \
    SLANG_CUDA_VECTOR_BINARY_COMPARE_OP(T, n, !=) \
    SLANG_CUDA_VECTOR_UNARY_OP(T, n, !)           \
    SLANG_CUDA_VECTOR_UNARY_OP(T, n, -)           \
    SLANG_CUDA_VECTOR_UNARY_OP(T, n, ~)

#define SLANG_CUDA_VECTOR_INT_OPS(T) \
    SLANG_CUDA_VECTOR_INT_OP(T, 2)   \
    SLANG_CUDA_VECTOR_INT_OP(T, 3)   \
    SLANG_CUDA_VECTOR_INT_OP(T, 4)

SLANG_CUDA_VECTOR_INT_OPS(int)
SLANG_CUDA_VECTOR_INT_OPS(bool)
SLANG_CUDA_VECTOR_INT_OPS(uint)
SLANG_CUDA_VECTOR_INT_OPS(ushort)
SLANG_CUDA_VECTOR_INT_OPS(short)
SLANG_CUDA_VECTOR_INT_OPS(char)
SLANG_CUDA_VECTOR_INT_OPS(uchar)
SLANG_CUDA_VECTOR_INT_OPS(longlong)
SLANG_CUDA_VECTOR_INT_OPS(ulonglong)

#define SLANG_CUDA_VECTOR_FLOAT_OP(T, n)          \
    SLANG_CUDA_VECTOR_BINARY_OP(T, n, +)          \
    SLANG_CUDA_VECTOR_BINARY_OP(T, n, -)          \
    SLANG_CUDA_VECTOR_BINARY_OP(T, n, *)          \
    SLANG_CUDA_VECTOR_BINARY_OP(T, n, /)          \
    SLANG_CUDA_VECTOR_BINARY_OP(T, n, &&)         \
    SLANG_CUDA_VECTOR_BINARY_OP(T, n, ||)         \
    SLANG_CUDA_VECTOR_BINARY_COMPARE_OP(T, n, >)  \
    SLANG_CUDA_VECTOR_BINARY_COMPARE_OP(T, n, <)  \
    SLANG_CUDA_VECTOR_BINARY_COMPARE_OP(T, n, >=) \
    SLANG_CUDA_VECTOR_BINARY_COMPARE_OP(T, n, <=) \
    SLANG_CUDA_VECTOR_BINARY_COMPARE_OP(T, n, ==) \
    SLANG_CUDA_VECTOR_BINARY_COMPARE_OP(T, n, !=) \
    SLANG_CUDA_VECTOR_UNARY_OP(T, n, -)
/* Special case __half2: use CUDA intrinsics (__hadd2, __hsub2, etc.) so we get one add.f16x2
   per op; generic macro would give two add.f16. Compare/logical stay element-wise for bool2. */
#define SLANG_CUDA_VECTOR_FLOAT_OP_HALF2 \
    SLANG_FORCE_INLINE SLANG_CUDA_CALL __half2 operator+(const __half2& lh, const __half2& rh) { return __hadd2(lh, rh); } \
    SLANG_FORCE_INLINE SLANG_CUDA_CALL __half2 operator-(const __half2& lh, const __half2& rh) { return __hsub2(lh, rh); } \
    SLANG_FORCE_INLINE SLANG_CUDA_CALL __half2 operator*(const __half2& lh, const __half2& rh) { return __hmul2(lh, rh); } \
    SLANG_FORCE_INLINE SLANG_CUDA_CALL __half2 operator/(const __half2& lh, const __half2& rh) { return __h2div(lh, rh); } \
    SLANG_FORCE_INLINE SLANG_CUDA_CALL __half2 operator-(const __half2& h) { return __hneg2(h); } \
    SLANG_FORCE_INLINE SLANG_CUDA_CALL __half2& operator+=(__half2& lh, const __half2& rh) { lh = __hadd2(lh, rh); return lh; } \
    SLANG_FORCE_INLINE SLANG_CUDA_CALL __half2& operator-=(__half2& lh, const __half2& rh) { lh = __hsub2(lh, rh); return lh; } \
    SLANG_FORCE_INLINE SLANG_CUDA_CALL __half2& operator*=(__half2& lh, const __half2& rh) { lh = __hmul2(lh, rh); return lh; } \
    SLANG_FORCE_INLINE SLANG_CUDA_CALL __half2& operator/=(__half2& lh, const __half2& rh) { lh = __h2div(lh, rh); return lh; } \
    SLANG_CUDA_VECTOR_BINARY_OP(__half, 2, &&) \
    SLANG_CUDA_VECTOR_BINARY_OP(__half, 2, ||) \
    SLANG_CUDA_VECTOR_BINARY_COMPARE_OP(__half, 2, >) \
    SLANG_CUDA_VECTOR_BINARY_COMPARE_OP(__half, 2, <) \
    SLANG_CUDA_VECTOR_BINARY_COMPARE_OP(__half, 2, >=) \
    SLANG_CUDA_VECTOR_BINARY_COMPARE_OP(__half, 2, <=) \
    SLANG_CUDA_VECTOR_BINARY_COMPARE_OP(__half, 2, ==) \
    SLANG_CUDA_VECTOR_BINARY_COMPARE_OP(__half, 2, !=)
/* Explicit per-type expansion (no dispatch, no token-paste with __half) so NVRTC and all compilers behave. */
#define SLANG_CUDA_VECTOR_FLOAT_OPS_float \
    SLANG_CUDA_VECTOR_FLOAT_OP(float, 2) \
    SLANG_CUDA_VECTOR_FLOAT_OP(float, 3) \
    SLANG_CUDA_VECTOR_FLOAT_OP(float, 4)
#define SLANG_CUDA_VECTOR_FLOAT_OPS_double \
    SLANG_CUDA_VECTOR_FLOAT_OP(double, 2) \
    SLANG_CUDA_VECTOR_FLOAT_OP(double, 3) \
    SLANG_CUDA_VECTOR_FLOAT_OP(double, 4)
#define SLANG_CUDA_VECTOR_FLOAT_OPS___half \
    SLANG_CUDA_VECTOR_FLOAT_OP_HALF2 \
    SLANG_CUDA_VECTOR_FLOAT_OP(__half, 3) \
    SLANG_CUDA_VECTOR_FLOAT_OP(__half, 4)
#define SLANG_CUDA_VECTOR_FLOAT_OPS(T) SLANG_CUDA_VECTOR_FLOAT_OPS_##T

SLANG_CUDA_VECTOR_FLOAT_OPS(float)
SLANG_CUDA_VECTOR_FLOAT_OPS(double)
#if SLANG_CUDA_ENABLE_HALF
SLANG_CUDA_VECTOR_FLOAT_OPS(__half)
#endif
#define SLANG_CUDA_FLOAT_VECTOR_MOD_IMPL(T, n)                                             \
    SLANG_FORCE_INLINE SLANG_CUDA_CALL T##n operator%(const T##n& left, const T##n& right) \
    {                                                                                      \
        T##n result;                                                                       \
        for (int i = 0; i < n; i++)                                                        \
            *_slang_vector_get_element_ptr(&result, i) = _slang_fmod(                      \
                _slang_vector_get_element(left, i),                                        \
                _slang_vector_get_element(right, i));                                      \
        return result;                                                                     \
    }
#define SLANG_CUDA_FLOAT_VECTOR_MOD(T)     \
    SLANG_CUDA_FLOAT_VECTOR_MOD_IMPL(T, 2) \
    SLANG_CUDA_FLOAT_VECTOR_MOD_IMPL(T, 3) \
    SLANG_CUDA_FLOAT_VECTOR_MOD_IMPL(T, 4)

SLANG_CUDA_FLOAT_VECTOR_MOD(float)
SLANG_CUDA_FLOAT_VECTOR_MOD(double)

#if SLANG_CUDA_RTC || SLANG_CUDA_ENABLE_HALF
#define SLANG_MAKE_VECTOR(T)                                                \
    SLANG_FORCE_INLINE SLANG_CUDA_CALL T##2 make_##T##2(T x, T y)           \
    {                                                                       \
        return T##2 {x, y};                                                 \
    }                                                                       \
    SLANG_FORCE_INLINE SLANG_CUDA_CALL T##3 make_##T##3(T x, T y, T z)      \
    {                                                                       \
        return T##3 {x, y, z};                                              \
    }                                                                       \
    SLANG_FORCE_INLINE SLANG_CUDA_CALL T##4 make_##T##4(T x, T y, T z, T w) \
    {                                                                       \
        return T##4 {x, y, z, w};                                           \
    }
#endif

#if SLANG_CUDA_RTC
SLANG_MAKE_VECTOR(int)
SLANG_MAKE_VECTOR(uint)
SLANG_MAKE_VECTOR(short)
SLANG_MAKE_VECTOR(ushort)
SLANG_MAKE_VECTOR(char)
SLANG_MAKE_VECTOR(uchar)
SLANG_MAKE_VECTOR(float)
SLANG_MAKE_VECTOR(double)
SLANG_MAKE_VECTOR(longlong)
SLANG_MAKE_VECTOR(ulonglong)
#endif

#if SLANG_CUDA_ENABLE_HALF
SLANG_MAKE_VECTOR(__half)
#endif

#if SLANG_CUDA_ENABLE_BF16
SLANG_MAKE_VECTOR(__nv_bfloat16)
#endif

#if SLANG_CUDA_ENABLE_FP8
SLANG_MAKE_VECTOR(__nv_fp8_e4m3)
SLANG_MAKE_VECTOR(__nv_fp8_e5m2)
#endif

SLANG_FORCE_INLINE SLANG_CUDA_CALL bool1 make_bool1(bool x)
{
    return bool1{x};
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL bool2 make_bool2(bool x, bool y)
{
    return bool2{x, y};
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL bool3 make_bool3(bool x, bool y, bool z)
{
    return bool3{x, y, z};
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL bool4 make_bool4(bool x, bool y, bool z, bool w)
{
    return bool4{x, y, z, w};
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL bool2 make_bool2(bool x)
{
    return bool2{x, x};
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL bool3 make_bool3(bool x)
{
    return bool3{x, x, x};
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL bool4 make_bool4(bool x)
{
    return bool4{x, x, x, x};
}

#if SLANG_CUDA_RTC
#define SLANG_MAKE_VECTOR_FROM_SCALAR(T)                     \
    SLANG_FORCE_INLINE SLANG_CUDA_CALL T##1 make_##T##1(T x) \
    {                                                        \
        return T##1 {x};                                     \
    }                                                        \
    SLANG_FORCE_INLINE SLANG_CUDA_CALL T##2 make_##T##2(T x) \
    {                                                        \
        return make_##T##2(x, x);                            \
    }                                                        \
    SLANG_FORCE_INLINE SLANG_CUDA_CALL T##3 make_##T##3(T x) \
    {                                                        \
        return make_##T##3(x, x, x);                         \
    }                                                        \
    SLANG_FORCE_INLINE SLANG_CUDA_CALL T##4 make_##T##4(T x) \
    {                                                        \
        return make_##T##4(x, x, x, x);                      \
    }
#else
#define SLANG_MAKE_VECTOR_FROM_SCALAR(T)                     \
    SLANG_FORCE_INLINE SLANG_CUDA_CALL T##2 make_##T##2(T x) \
    {                                                        \
        return make_##T##2(x, x);                            \
    }                                                        \
    SLANG_FORCE_INLINE SLANG_CUDA_CALL T##3 make_##T##3(T x) \
    {                                                        \
        return make_##T##3(x, x, x);                         \
    }                                                        \
    SLANG_FORCE_INLINE SLANG_CUDA_CALL T##4 make_##T##4(T x) \
    {                                                        \
        return make_##T##4(x, x, x, x);                      \
    }
#endif
SLANG_MAKE_VECTOR_FROM_SCALAR(int)
SLANG_MAKE_VECTOR_FROM_SCALAR(uint)
SLANG_MAKE_VECTOR_FROM_SCALAR(short)
SLANG_MAKE_VECTOR_FROM_SCALAR(ushort)
SLANG_MAKE_VECTOR_FROM_SCALAR(char)
SLANG_MAKE_VECTOR_FROM_SCALAR(uchar)
SLANG_MAKE_VECTOR_FROM_SCALAR(longlong)
SLANG_MAKE_VECTOR_FROM_SCALAR(ulonglong)
SLANG_MAKE_VECTOR_FROM_SCALAR(float)
SLANG_MAKE_VECTOR_FROM_SCALAR(double)
#if SLANG_CUDA_ENABLE_HALF
SLANG_MAKE_VECTOR_FROM_SCALAR(__half)
#if !SLANG_CUDA_RTC
SLANG_FORCE_INLINE SLANG_CUDA_CALL __half1 make___half1(__half x)
{
    return __half1{x};
}
#endif
#endif
#if SLANG_CUDA_ENABLE_BF16
SLANG_MAKE_VECTOR_FROM_SCALAR(__nv_bfloat16)
#if !SLANG_CUDA_RTC
SLANG_FORCE_INLINE SLANG_CUDA_CALL __nv_bfloat16 make___nv_bfloat161(__nv_bfloat16 x)
{
    return __nv_bfloat16{x};
}
#endif
#endif

#if SLANG_CUDA_ENABLE_FP8
SLANG_MAKE_VECTOR_FROM_SCALAR(__nv_fp8_e4m3)
SLANG_MAKE_VECTOR_FROM_SCALAR(__nv_fp8_e5m2)
#if !SLANG_CUDA_RTC
SLANG_FORCE_INLINE SLANG_CUDA_CALL __nv_fp8_e4m3 make___nv_fp8_e4m31(__nv_fp8_e4m3 x)
{
    return __nv_fp8_e4m3{x};
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL __nv_fp8_e5m2 make___nv_fp8_e5m21(__nv_fp8_e5m2 x)
{
    return __nv_fp8_e5m2{x};
}
#endif
#endif

#define SLANG_CUDA_VECTOR_ATOMIC_BINARY_IMPL(Fn, T, N)                                            \
    SLANG_FORCE_INLINE SLANG_CUDA_CALL T##N Fn(T##N* address, T##N val)                           \
    {                                                                                             \
        T##N result;                                                                              \
        for (int i = 0; i < N; i++)                                                               \
            *_slang_vector_get_element_ptr(&result, i) =                                          \
                Fn(_slang_vector_get_element_ptr(address, i), _slang_vector_get_element(val, i)); \
        return result;                                                                            \
    }

#if defined(__CUDA_ARCH__) && __CUDA_ARCH__ < 900
SLANG_CUDA_VECTOR_ATOMIC_BINARY_IMPL(atomicAdd, float, 2)
SLANG_CUDA_VECTOR_ATOMIC_BINARY_IMPL(atomicAdd, float, 4)
#endif
#if defined(SLANG_CUDA_ENABLE_HALF) && defined(__CUDA_ARCH__) && (__CUDA_ARCH__ >= 700)
SLANG_CUDA_VECTOR_ATOMIC_BINARY_IMPL(atomicAdd, __half, 3)
SLANG_CUDA_VECTOR_ATOMIC_BINARY_IMPL(atomicAdd, __half, 4)
#endif
SLANG_CUDA_VECTOR_ATOMIC_BINARY_IMPL(atomicAdd, float, 3)
SLANG_CUDA_VECTOR_ATOMIC_BINARY_IMPL(atomicAdd, int, 2)
SLANG_CUDA_VECTOR_ATOMIC_BINARY_IMPL(atomicAdd, int, 3)
SLANG_CUDA_VECTOR_ATOMIC_BINARY_IMPL(atomicAdd, int, 4)
SLANG_CUDA_VECTOR_ATOMIC_BINARY_IMPL(atomicAdd, uint, 2)
SLANG_CUDA_VECTOR_ATOMIC_BINARY_IMPL(atomicAdd, uint, 3)
SLANG_CUDA_VECTOR_ATOMIC_BINARY_IMPL(atomicAdd, uint, 4)
SLANG_CUDA_VECTOR_ATOMIC_BINARY_IMPL(atomicAdd, ulonglong, 2)
SLANG_CUDA_VECTOR_ATOMIC_BINARY_IMPL(atomicAdd, ulonglong, 3)
SLANG_CUDA_VECTOR_ATOMIC_BINARY_IMPL(atomicAdd, ulonglong, 4)

template<typename T, int n>
struct GetVectorTypeImpl
{
};

#define GET_VECTOR_TYPE_IMPL(T, n)                                     \
    template<>                                                         \
    struct GetVectorTypeImpl<T, n>                                     \
    {                                                                  \
        typedef T##n type;                                             \
        static SLANG_FORCE_INLINE SLANG_CUDA_CALL T##n fromScalar(T v) \
        {                                                              \
            return make_##T##n(v);                                     \
        }                                                              \
    };
#define GET_VECTOR_TYPE_IMPL_N(T) \
    GET_VECTOR_TYPE_IMPL(T, 1)    \
    GET_VECTOR_TYPE_IMPL(T, 2)    \
    GET_VECTOR_TYPE_IMPL(T, 3)    \
    GET_VECTOR_TYPE_IMPL(T, 4)

GET_VECTOR_TYPE_IMPL_N(int)
GET_VECTOR_TYPE_IMPL_N(bool)
GET_VECTOR_TYPE_IMPL_N(uint)
GET_VECTOR_TYPE_IMPL_N(short)
GET_VECTOR_TYPE_IMPL_N(ushort)
GET_VECTOR_TYPE_IMPL_N(char)
GET_VECTOR_TYPE_IMPL_N(uchar)
GET_VECTOR_TYPE_IMPL_N(longlong)
GET_VECTOR_TYPE_IMPL_N(ulonglong)
GET_VECTOR_TYPE_IMPL_N(float)
GET_VECTOR_TYPE_IMPL_N(double)
#if SLANG_CUDA_ENABLE_HALF
GET_VECTOR_TYPE_IMPL_N(__half)
#endif
#if SLANG_CUDA_ENABLE_BF16
GET_VECTOR_TYPE_IMPL_N(__nv_bfloat16)
#endif
#if SLANG_CUDA_ENABLE_FP8
GET_VECTOR_TYPE_IMPL_N(__nv_fp8_e4m3)
GET_VECTOR_TYPE_IMPL_N(__nv_fp8_e5m2)
#endif

template<typename T, int n>
using Vector = typename GetVectorTypeImpl<T, n>::type;

template<typename T, int n, typename OtherT, int m>
SLANG_FORCE_INLINE SLANG_CUDA_CALL Vector<T, n> _slang_vector_reshape(const Vector<OtherT, m> other)
{
    Vector<T, n> result;
    for (int i = 0; i < n; i++)
    {
        OtherT otherElement = T(0);
        if (i < m)
            otherElement = _slang_vector_get_element(other, i);
        *_slang_vector_get_element_ptr(&result, i) = (T)otherElement;
    }
    return result;
}

template<typename T, int ROWS, int COLS>
struct Matrix
{
    Vector<T, COLS> rows[ROWS];
    SLANG_FORCE_INLINE SLANG_CUDA_CALL Vector<T, COLS>& operator[](size_t index)
    {
        return rows[index];
    }

    SLANG_FORCE_INLINE SLANG_CUDA_CALL const Vector<T, COLS>& operator[](size_t index) const
    {
        return rows[index];
    }
};


template<typename T, int ROWS, int COLS>
SLANG_FORCE_INLINE SLANG_CUDA_CALL Matrix<T, ROWS, COLS> makeMatrix(T scalar)
{
    Matrix<T, ROWS, COLS> result;
    for (int i = 0; i < ROWS; i++)
        result.rows[i] = GetVectorTypeImpl<T, COLS>::fromScalar(scalar);
    return result;
}

template<typename T, int ROWS, int COLS>
SLANG_FORCE_INLINE SLANG_CUDA_CALL Matrix<T, ROWS, COLS> makeMatrix(const Vector<T, COLS>& row0)
{
    Matrix<T, ROWS, COLS> result;
    result.rows[0] = row0;
    return result;
}

template<typename T, int ROWS, int COLS>
SLANG_FORCE_INLINE SLANG_CUDA_CALL Matrix<T, ROWS, COLS> makeMatrix(
    const Vector<T, COLS>& row0,
    const Vector<T, COLS>& row1)
{
    Matrix<T, ROWS, COLS> result;
    result.rows[0] = row0;
    result.rows[1] = row1;
    return result;
}

template<typename T, int ROWS, int COLS>
SLANG_FORCE_INLINE SLANG_CUDA_CALL Matrix<T, ROWS, COLS> makeMatrix(
    const Vector<T, COLS>& row0,
    const Vector<T, COLS>& row1,
    const Vector<T, COLS>& row2)
{
    Matrix<T, ROWS, COLS> result;
    result.rows[0] = row0;
    result.rows[1] = row1;
    result.rows[2] = row2;
    return result;
}

template<typename T, int ROWS, int COLS>
SLANG_FORCE_INLINE SLANG_CUDA_CALL Matrix<T, ROWS, COLS> makeMatrix(
    const Vector<T, COLS>& row0,
    const Vector<T, COLS>& row1,
    const Vector<T, COLS>& row2,
    const Vector<T, COLS>& row3)
{
    Matrix<T, ROWS, COLS> result;
    result.rows[0] = row0;
    result.rows[1] = row1;
    result.rows[2] = row2;
    result.rows[3] = row3;
    return result;
}

template<typename T, int ROWS, int COLS, typename U, int otherRow, int otherCol>
SLANG_FORCE_INLINE SLANG_CUDA_CALL Matrix<T, ROWS, COLS> makeMatrix(
    const Matrix<U, otherRow, otherCol>& other)
{
    Matrix<T, ROWS, COLS> result;
    int minRow = ROWS;
    int minCol = COLS;
    if (minRow > otherRow)
        minRow = otherRow;
    if (minCol > otherCol)
        minCol = otherCol;
    for (int i = 0; i < minRow; i++)
        for (int j = 0; j < minCol; j++)
            *_slang_vector_get_element_ptr(result.rows + i, j) =
                (T)_slang_vector_get_element(other.rows[i], j);
    return result;
}

template<typename T, int ROWS, int COLS>
SLANG_FORCE_INLINE SLANG_CUDA_CALL Matrix<T, ROWS, COLS> makeMatrix(T v0, T v1, T v2, T v3)
{
    Matrix<T, ROWS, COLS> rs;
    rs.rows[0].x = v0;
    rs.rows[0].y = v1;
    rs.rows[1].x = v2;
    rs.rows[1].y = v3;
    return rs;
}

template<typename T, int ROWS, int COLS>
SLANG_FORCE_INLINE SLANG_CUDA_CALL Matrix<T, ROWS, COLS> makeMatrix(
    T v0,
    T v1,
    T v2,
    T v3,
    T v4,
    T v5)
{
    Matrix<T, ROWS, COLS> rs;
    if (COLS == 3)
    {
        *_slang_vector_get_element_ptr(&rs.rows[0], 0) = v0;
        *_slang_vector_get_element_ptr(&rs.rows[0], 1) = v1;
        *_slang_vector_get_element_ptr(&rs.rows[0], 2) = v2;
        *_slang_vector_get_element_ptr(&rs.rows[1], 0) = v3;
        *_slang_vector_get_element_ptr(&rs.rows[1], 1) = v4;
        *_slang_vector_get_element_ptr(&rs.rows[1], 2) = v5;
    }
    else
    {
        rs.rows[0].x = v0;
        rs.rows[0].y = v1;
        rs.rows[1].x = v2;
        rs.rows[1].y = v3;
        rs.rows[2].x = v4;
        rs.rows[2].y = v5;
    }
    return rs;
}

template<typename T, int ROWS, int COLS>
SLANG_FORCE_INLINE SLANG_CUDA_CALL Matrix<T, ROWS, COLS> makeMatrix(
    T v0,
    T v1,
    T v2,
    T v3,
    T v4,
    T v5,
    T v6,
    T v7)
{
    Matrix<T, ROWS, COLS> rs;
    if (COLS == 4)
    {
        *_slang_vector_get_element_ptr(&rs.rows[0], 0) = v0;
        *_slang_vector_get_element_ptr(&rs.rows[0], 1) = v1;
        *_slang_vector_get_element_ptr(&rs.rows[0], 2) = v2;
        *_slang_vector_get_element_ptr(&rs.rows[0], 3) = v3;
        *_slang_vector_get_element_ptr(&rs.rows[1], 0) = v4;
        *_slang_vector_get_element_ptr(&rs.rows[1], 1) = v5;
        *_slang_vector_get_element_ptr(&rs.rows[1], 2) = v6;
        *_slang_vector_get_element_ptr(&rs.rows[1], 3) = v7;
    }
    else
    {
        rs.rows[0].x = v0;
        rs.rows[0].y = v1;
        rs.rows[1].x = v2;
        rs.rows[1].y = v3;
        rs.rows[2].x = v4;
        rs.rows[2].y = v5;
        rs.rows[3].x = v6;
        rs.rows[3].y = v7;
    }
    return rs;
}

template<typename T, int ROWS, int COLS>
SLANG_FORCE_INLINE SLANG_CUDA_CALL Matrix<T, ROWS, COLS> makeMatrix(
    T v0,
    T v1,
    T v2,
    T v3,
    T v4,
    T v5,
    T v6,
    T v7,
    T v8)
{
    Matrix<T, ROWS, COLS> rs;
    rs.rows[0].x = v0;
    rs.rows[0].y = v1;
    rs.rows[0].z = v2;
    rs.rows[1].x = v3;
    rs.rows[1].y = v4;
    rs.rows[1].z = v5;
    rs.rows[2].x = v6;
    rs.rows[2].y = v7;
    rs.rows[2].z = v8;
    return rs;
}

template<typename T, int ROWS, int COLS>
SLANG_FORCE_INLINE SLANG_CUDA_CALL Matrix<T, ROWS, COLS> makeMatrix(
    T v0,
    T v1,
    T v2,
    T v3,
    T v4,
    T v5,
    T v6,
    T v7,
    T v8,
    T v9,
    T v10,
    T v11)
{
    Matrix<T, ROWS, COLS> rs;
    if (COLS == 4)
    {
        *_slang_vector_get_element_ptr(&rs.rows[0], 0) = v0;
        *_slang_vector_get_element_ptr(&rs.rows[0], 1) = v1;
        *_slang_vector_get_element_ptr(&rs.rows[0], 2) = v2;
        *_slang_vector_get_element_ptr(&rs.rows[0], 3) = v3;
        *_slang_vector_get_element_ptr(&rs.rows[1], 0) = v4;
        *_slang_vector_get_element_ptr(&rs.rows[1], 1) = v5;
        *_slang_vector_get_element_ptr(&rs.rows[1], 2) = v6;
        *_slang_vector_get_element_ptr(&rs.rows[1], 3) = v7;
        *_slang_vector_get_element_ptr(&rs.rows[2], 0) = v8;
        *_slang_vector_get_element_ptr(&rs.rows[2], 1) = v9;
        *_slang_vector_get_element_ptr(&rs.rows[2], 2) = v10;
        *_slang_vector_get_element_ptr(&rs.rows[2], 3) = v11;
    }
    else
    {
        rs.rows[0].x = v0;
        rs.rows[0].y = v1;
        rs.rows[0].z = v2;
        rs.rows[1].x = v3;
        rs.rows[1].y = v4;
        rs.rows[1].z = v5;
        rs.rows[2].x = v6;
        rs.rows[2].y = v7;
        rs.rows[2].z = v8;
        rs.rows[3].x = v9;
        rs.rows[3].y = v10;
        rs.rows[3].z = v11;
    }
    return rs;
}

template<typename T, int ROWS, int COLS>
SLANG_FORCE_INLINE SLANG_CUDA_CALL Matrix<T, ROWS, COLS> makeMatrix(
    T v0,
    T v1,
    T v2,
    T v3,
    T v4,
    T v5,
    T v6,
    T v7,
    T v8,
    T v9,
    T v10,
    T v11,
    T v12,
    T v13,
    T v14,
    T v15)
{
    Matrix<T, ROWS, COLS> rs;
    rs.rows[0].x = v0;
    rs.rows[0].y = v1;
    rs.rows[0].z = v2;
    rs.rows[0].w = v3;
    rs.rows[1].x = v4;
    rs.rows[1].y = v5;
    rs.rows[1].z = v6;
    rs.rows[1].w = v7;
    rs.rows[2].x = v8;
    rs.rows[2].y = v9;
    rs.rows[2].z = v10;
    rs.rows[2].w = v11;
    rs.rows[3].x = v12;
    rs.rows[3].y = v13;
    rs.rows[3].z = v14;
    rs.rows[3].w = v15;
    return rs;
}

#define SLANG_MATRIX_BINARY_OP(T, op)                                   \
    template<int R, int C>                                              \
    SLANG_FORCE_INLINE SLANG_CUDA_CALL Matrix<T, R, C> operator op(     \
        const Matrix<T, R, C>& thisVal,                                 \
        const Matrix<T, R, C>& other)                                   \
    {                                                                   \
        Matrix<T, R, C> result;                                         \
        for (int i = 0; i < R; i++)                                     \
            for (int j = 0; j < C; j++)                                 \
                *_slang_vector_get_element_ptr(result.rows + i, j) =    \
                    _slang_vector_get_element(thisVal.rows[i], j)       \
                        op _slang_vector_get_element(other.rows[i], j); \
        return result;                                                  \
    }

#define SLANG_MATRIX_UNARY_OP(T, op)                                                               \
    template<int R, int C>                                                                         \
    SLANG_FORCE_INLINE SLANG_CUDA_CALL Matrix<T, R, C> operator op(const Matrix<T, R, C>& thisVal) \
    {                                                                                              \
        Matrix<T, R, C> result;                                                                    \
        for (int i = 0; i < R; i++)                                                                \
            for (int j = 0; j < C; j++)                                                            \
                *_slang_vector_get_element_ptr(result.rows + i, j) =                               \
                    op _slang_vector_get_element(thisVal.rows[i], j);                              \
        return result;                                                                             \
    }
#define SLANG_INT_MATRIX_OPS(T)   \
    SLANG_MATRIX_BINARY_OP(T, +)  \
    SLANG_MATRIX_BINARY_OP(T, -)  \
    SLANG_MATRIX_BINARY_OP(T, *)  \
    SLANG_MATRIX_BINARY_OP(T, /)  \
    SLANG_MATRIX_BINARY_OP(T, &)  \
    SLANG_MATRIX_BINARY_OP(T, |)  \
    SLANG_MATRIX_BINARY_OP(T, &&) \
    SLANG_MATRIX_BINARY_OP(T, ||) \
    SLANG_MATRIX_BINARY_OP(T, ^)  \
    SLANG_MATRIX_BINARY_OP(T, %)  \
    SLANG_MATRIX_UNARY_OP(T, !)   \
    SLANG_MATRIX_UNARY_OP(T, ~)
#define SLANG_FLOAT_MATRIX_OPS(T) \
    SLANG_MATRIX_BINARY_OP(T, +)  \
    SLANG_MATRIX_BINARY_OP(T, -)  \
    SLANG_MATRIX_BINARY_OP(T, *)  \
    SLANG_MATRIX_BINARY_OP(T, /)  \
    SLANG_MATRIX_UNARY_OP(T, -)
SLANG_INT_MATRIX_OPS(int)
SLANG_INT_MATRIX_OPS(uint)
SLANG_INT_MATRIX_OPS(short)
SLANG_INT_MATRIX_OPS(ushort)
SLANG_INT_MATRIX_OPS(char)
SLANG_INT_MATRIX_OPS(uchar)
SLANG_INT_MATRIX_OPS(longlong)
SLANG_INT_MATRIX_OPS(ulonglong)
SLANG_FLOAT_MATRIX_OPS(float)
SLANG_FLOAT_MATRIX_OPS(double)
#if SLANG_CUDA_ENABLE_HALF
SLANG_FLOAT_MATRIX_OPS(__half)
#endif
#define SLANG_MATRIX_INT_NEG_OP(T)                                                        \
    template<int R, int C>                                                                \
    SLANG_FORCE_INLINE SLANG_CUDA_CALL Matrix<T, R, C> operator-(Matrix<T, R, C> thisVal) \
    {                                                                                     \
        Matrix<T, R, C> result;                                                           \
        for (int i = 0; i < R; i++)                                                       \
            for (int j = 0; j < C; j++)                                                   \
                *_slang_vector_get_element_ptr(result.rows + i, j) =                      \
                    0 - _slang_vector_get_element(thisVal.rows[i], j);                    \
        return result;                                                                    \
    }
SLANG_MATRIX_INT_NEG_OP(int)
SLANG_MATRIX_INT_NEG_OP(uint)
SLANG_MATRIX_INT_NEG_OP(short)
SLANG_MATRIX_INT_NEG_OP(ushort)
SLANG_MATRIX_INT_NEG_OP(char)
SLANG_MATRIX_INT_NEG_OP(uchar)
SLANG_MATRIX_INT_NEG_OP(longlong)
SLANG_MATRIX_INT_NEG_OP(ulonglong)

#define SLANG_FLOAT_MATRIX_MOD(T)                                                 \
    template<int R, int C>                                                        \
    SLANG_FORCE_INLINE SLANG_CUDA_CALL Matrix<T, R, C> operator%(                 \
        Matrix<T, R, C> left,                                                     \
        Matrix<T, R, C> right)                                                    \
    {                                                                             \
        Matrix<T, R, C> result;                                                   \
        for (int i = 0; i < R; i++)                                               \
            for (int j = 0; j < C; j++)                                           \
                *_slang_vector_get_element_ptr(result.rows + i, j) = _slang_fmod( \
                    _slang_vector_get_element(left.rows[i], j),                   \
                    _slang_vector_get_element(right.rows[i], j));                 \
        return result;                                                            \
    }

SLANG_FLOAT_MATRIX_MOD(float)
SLANG_FLOAT_MATRIX_MOD(double)
#if SLANG_CUDA_ENABLE_HALF
template<int R, int C>
SLANG_FORCE_INLINE SLANG_CUDA_CALL Matrix<__half, R, C> operator%(
    Matrix<__half, R, C> left,
    Matrix<__half, R, C> right)
{
    Matrix<__half, R, C> result;
    for (int i = 0; i < R; i++)
        for (int j = 0; j < C; j++)
            *_slang_vector_get_element_ptr(result.rows + i, j) = __float2half(_slang_fmod(
                __half2float(_slang_vector_get_element(left.rows[i], j)),
                __half2float(_slang_vector_get_element(right.rows[i], j))));
    return result;
}
#endif
#undef SLANG_FLOAT_MATRIX_MOD
#undef SLANG_MATRIX_BINARY_OP
#undef SLANG_MATRIX_UNARY_OP
#undef SLANG_INT_MATRIX_OPS
#undef SLANG_FLOAT_MATRIX_OPS
#undef SLANG_MATRIX_INT_NEG_OP
#undef SLANG_FLOAT_MATRIX_MOD

#define SLANG_SELECT_IMPL(T, N)                                                                  \
    SLANG_FORCE_INLINE SLANG_CUDA_CALL Vector<T, N> _slang_select(                               \
        bool##N condition,                                                                       \
        Vector<T, N> v0,                                                                         \
        Vector<T, N> v1)                                                                         \
    {                                                                                            \
        Vector<T, N> result;                                                                     \
        for (int i = 0; i < N; i++)                                                              \
        {                                                                                        \
            *_slang_vector_get_element_ptr(&result, i) = _slang_vector_get_element(condition, i) \
                                                             ? _slang_vector_get_element(v0, i)  \
                                                             : _slang_vector_get_element(v1, i); \
        }                                                                                        \
        return result;                                                                           \
    }
#define SLANG_SELECT_T(T)   \
    SLANG_SELECT_IMPL(T, 2) \
    SLANG_SELECT_IMPL(T, 3) \
    SLANG_SELECT_IMPL(T, 4)

SLANG_SELECT_T(int)
SLANG_SELECT_T(bool)
SLANG_SELECT_T(uint)
SLANG_SELECT_T(short)
SLANG_SELECT_T(ushort)
SLANG_SELECT_T(char)
SLANG_SELECT_T(uchar)
SLANG_SELECT_T(float)
SLANG_SELECT_T(double)

template<typename T>
SLANG_FORCE_INLINE SLANG_CUDA_CALL T _slang_select(bool condition, T v0, T v1)
{
    return condition ? v0 : v1;
}

//
// Half support
//

#if SLANG_CUDA_ENABLE_HALF
SLANG_SELECT_T(__half)

// Convenience functions ushort -> half

SLANG_FORCE_INLINE SLANG_CUDA_CALL __half2 __ushort_as_half(const ushort2& i)
{
    return __halves2half2(__ushort_as_half(i.x), __ushort_as_half(i.y));
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL __half3 __ushort_as_half(const ushort3& i)
{
    return __half3{__ushort_as_half(i.x), __ushort_as_half(i.y), __ushort_as_half(i.z)};
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL __half4 __ushort_as_half(const ushort4& i)
{
    return __half4{
        __ushort_as_half(i.x),
        __ushort_as_half(i.y),
        __ushort_as_half(i.z),
        __ushort_as_half(i.w)};
}

// Convenience functions half -> ushort

SLANG_FORCE_INLINE SLANG_CUDA_CALL ushort2 __half_as_ushort(const __half2& i)
{
    return make_ushort2(__half_as_ushort(i.x), __half_as_ushort(i.y));
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL ushort3 __half_as_ushort(const __half3& i)
{
    return make_ushort3(__half_as_ushort(i.x), __half_as_ushort(i.y), __half_as_ushort(i.z));
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL ushort4 __half_as_ushort(const __half4& i)
{
    return make_ushort4(
        __half_as_ushort(i.x),
        __half_as_ushort(i.y),
        __half_as_ushort(i.z),
        __half_as_ushort(i.w));
}

// This is a little bit of a hack. Fortunately CUDA has the definitions of the templated types in
// include/surface_indirect_functions.h
// Here we find the template definition requires a specialization of __nv_isurf_trait to allow
// a specialization of the surface write functions.
// This *isn't* a problem on the read functions as they don't have a return type that uses this
// mechanism

template<>
struct __nv_isurf_trait<__half>
{
    typedef void type;
};
template<>
struct __nv_isurf_trait<__half2>
{
    typedef void type;
};
template<>
struct __nv_isurf_trait<__half4>
{
    typedef void type;
};

#define SLANG_DROP_PARENS(...) __VA_ARGS__

#define SLANG_SURFACE_READ(FUNC_NAME, TYPE_ARGS, ARGS)                                             \
    template<>                                                                                     \
    SLANG_FORCE_INLINE SLANG_CUDA_CALL __half FUNC_NAME<__half>(                                   \
        cudaSurfaceObject_t surfObj,                                                               \
        SLANG_DROP_PARENS TYPE_ARGS,                                                               \
        cudaSurfaceBoundaryMode boundaryMode)                                                      \
    {                                                                                              \
        return __ushort_as_half(FUNC_NAME<ushort>(surfObj, SLANG_DROP_PARENS ARGS, boundaryMode)); \
    }                                                                                              \
                                                                                                   \
    template<>                                                                                     \
    SLANG_FORCE_INLINE SLANG_CUDA_CALL __half2 FUNC_NAME<__half2>(                                 \
        cudaSurfaceObject_t surfObj,                                                               \
        SLANG_DROP_PARENS TYPE_ARGS,                                                               \
        cudaSurfaceBoundaryMode boundaryMode)                                                      \
    {                                                                                              \
        return __ushort_as_half(                                                                   \
            FUNC_NAME<ushort2>(surfObj, SLANG_DROP_PARENS ARGS, boundaryMode));                    \
    }                                                                                              \
                                                                                                   \
    template<>                                                                                     \
    SLANG_FORCE_INLINE SLANG_CUDA_CALL __half4 FUNC_NAME<__half4>(                                 \
        cudaSurfaceObject_t surfObj,                                                               \
        SLANG_DROP_PARENS TYPE_ARGS,                                                               \
        cudaSurfaceBoundaryMode boundaryMode)                                                      \
    {                                                                                              \
        return __ushort_as_half(                                                                   \
            FUNC_NAME<ushort4>(surfObj, SLANG_DROP_PARENS ARGS, boundaryMode));                    \
    }

SLANG_SURFACE_READ(surf1Dread, (int x), (x))
SLANG_SURFACE_READ(surf2Dread, (int x, int y), (x, y))
SLANG_SURFACE_READ(surf3Dread, (int x, int y, int z), (x, y, z))
SLANG_SURFACE_READ(surf1DLayeredread, (int x, int layer), (x, layer))
SLANG_SURFACE_READ(surf2DLayeredread, (int x, int y, int layer), (x, y, layer))
SLANG_SURFACE_READ(surfCubemapread, (int x, int y, int face), (x, y, face))
SLANG_SURFACE_READ(surfCubemapLayeredread, (int x, int y, int layerFace), (x, y, layerFace))

#define SLANG_SURFACE_WRITE(FUNC_NAME, TYPE_ARGS, ARGS)                                            \
    template<>                                                                                     \
    SLANG_FORCE_INLINE SLANG_CUDA_CALL void FUNC_NAME<__half>(                                     \
        __half data,                                                                               \
        cudaSurfaceObject_t surfObj,                                                               \
        SLANG_DROP_PARENS TYPE_ARGS,                                                               \
        cudaSurfaceBoundaryMode boundaryMode)                                                      \
    {                                                                                              \
        FUNC_NAME<ushort>(__half_as_ushort(data), surfObj, SLANG_DROP_PARENS ARGS, boundaryMode);  \
    }                                                                                              \
                                                                                                   \
    template<>                                                                                     \
    SLANG_FORCE_INLINE SLANG_CUDA_CALL void FUNC_NAME<__half2>(                                    \
        __half2 data,                                                                              \
        cudaSurfaceObject_t surfObj,                                                               \
        SLANG_DROP_PARENS TYPE_ARGS,                                                               \
        cudaSurfaceBoundaryMode boundaryMode)                                                      \
    {                                                                                              \
        FUNC_NAME<ushort2>(__half_as_ushort(data), surfObj, SLANG_DROP_PARENS ARGS, boundaryMode); \
    }                                                                                              \
                                                                                                   \
    template<>                                                                                     \
    SLANG_FORCE_INLINE SLANG_CUDA_CALL void FUNC_NAME<__half4>(                                    \
        __half4 data,                                                                              \
        cudaSurfaceObject_t surfObj,                                                               \
        SLANG_DROP_PARENS TYPE_ARGS,                                                               \
        cudaSurfaceBoundaryMode boundaryMode)                                                      \
    {                                                                                              \
        FUNC_NAME<ushort4>(__half_as_ushort(data), surfObj, SLANG_DROP_PARENS ARGS, boundaryMode); \
    }

SLANG_SURFACE_WRITE(surf1Dwrite, (int x), (x))
SLANG_SURFACE_WRITE(surf2Dwrite, (int x, int y), (x, y))
SLANG_SURFACE_WRITE(surf3Dwrite, (int x, int y, int z), (x, y, z))
SLANG_SURFACE_WRITE(surf1DLayeredwrite, (int x, int layer), (x, layer))
SLANG_SURFACE_WRITE(surf2DLayeredwrite, (int x, int y, int layer), (x, y, layer))
SLANG_SURFACE_WRITE(surfCubemapwrite, (int x, int y, int face), (x, y, face))
SLANG_SURFACE_WRITE(surfCubemapLayeredwrite, (int x, int y, int layerFace), (x, y, layerFace))

// ! Hack to test out reading !!!
// Only works converting *from* half

// template <typename T>
// SLANG_FORCE_INLINE SLANG_CUDA_CALL T surf2Dread_convert(cudaSurfaceObject_t surfObj, int x, int
// y, cudaSurfaceBoundaryMode boundaryMode);

#define SLANG_SURFACE_READ_HALF_CONVERT(FUNC_NAME, TYPE_ARGS, ARGS)                              \
                                                                                                 \
    template<typename T>                                                                         \
    SLANG_FORCE_INLINE SLANG_CUDA_CALL T FUNC_NAME##_convert(                                    \
        cudaSurfaceObject_t surfObj,                                                             \
        SLANG_DROP_PARENS TYPE_ARGS,                                                             \
        cudaSurfaceBoundaryMode boundaryMode);                                                   \
                                                                                                 \
    template<>                                                                                   \
    SLANG_FORCE_INLINE SLANG_CUDA_CALL float FUNC_NAME##_convert<float>(                         \
        cudaSurfaceObject_t surfObj,                                                             \
        SLANG_DROP_PARENS TYPE_ARGS,                                                             \
        cudaSurfaceBoundaryMode boundaryMode)                                                    \
    {                                                                                            \
        return __ushort_as_half(                                                                 \
            FUNC_NAME<uint16_t>(surfObj, SLANG_DROP_PARENS ARGS, boundaryMode));                 \
    }                                                                                            \
                                                                                                 \
    template<>                                                                                   \
    SLANG_FORCE_INLINE SLANG_CUDA_CALL float2 FUNC_NAME##_convert<float2>(                       \
        cudaSurfaceObject_t surfObj,                                                             \
        SLANG_DROP_PARENS TYPE_ARGS,                                                             \
        cudaSurfaceBoundaryMode boundaryMode)                                                    \
    {                                                                                            \
        const __half2 v =                                                                        \
            __ushort_as_half(FUNC_NAME<ushort2>(surfObj, SLANG_DROP_PARENS ARGS, boundaryMode)); \
        return float2{v.x, v.y};                                                                 \
    }                                                                                            \
                                                                                                 \
    template<>                                                                                   \
    SLANG_FORCE_INLINE SLANG_CUDA_CALL float4 FUNC_NAME##_convert<float4>(                       \
        cudaSurfaceObject_t surfObj,                                                             \
        SLANG_DROP_PARENS TYPE_ARGS,                                                             \
        cudaSurfaceBoundaryMode boundaryMode)                                                    \
    {                                                                                            \
        const __half4 v =                                                                        \
            __ushort_as_half(FUNC_NAME<ushort4>(surfObj, SLANG_DROP_PARENS ARGS, boundaryMode)); \
        return float4{v.x, v.y, v.z, v.w};                                                       \
    }

SLANG_SURFACE_READ_HALF_CONVERT(surf1Dread, (int x), (x))
SLANG_SURFACE_READ_HALF_CONVERT(surf2Dread, (int x, int y), (x, y))
SLANG_SURFACE_READ_HALF_CONVERT(surf3Dread, (int x, int y, int z), (x, y, z))

#endif

// Support for doing format conversion when writing to a surface/RWTexture

// NOTE! For normal surface access x values are *byte* addressed.
// For the _convert versions they are *not*. They don't need to be because sust.p does not require
// it.

// https://docs.nvidia.com/cuda/inline-ptx-assembly/index.html
// https://docs.nvidia.com/cuda/parallel-thread-execution/index.html#surface-instructions-sust


// surf1Dwrite_convert

template<typename T>
SLANG_FORCE_INLINE SLANG_CUDA_CALL void surf1Dwrite_convert(
    T v,
    cudaSurfaceObject_t surfObj,
    int x,
    cudaSurfaceBoundaryMode boundaryMode);

#define SLANG_SURF1DWRITE_CONVERT_IMPL(T, c)                                                     \
    template<>                                                                                   \
    SLANG_FORCE_INLINE SLANG_CUDA_CALL void surf1Dwrite_convert<T>(                              \
        T v,                                                                                     \
        cudaSurfaceObject_t surfObj,                                                             \
        int x,                                                                                   \
        cudaSurfaceBoundaryMode boundaryMode)                                                    \
    {                                                                                            \
        asm volatile(                                                                            \
            "sust.p.1d.b32." SLANG_PTX_BOUNDARY_MODE " [%0, {%1}], {%2};" ::"l"(surfObj),        \
            "r"(x),                                                                              \
            c(v));                                                                               \
    }                                                                                            \
    template<>                                                                                   \
    SLANG_FORCE_INLINE SLANG_CUDA_CALL void surf1Dwrite_convert<T##2>(                           \
        T##2 v,                                                                                  \
        cudaSurfaceObject_t surfObj,                                                             \
        int x,                                                                                   \
        cudaSurfaceBoundaryMode boundaryMode)                                                    \
    {                                                                                            \
        const T vx = v.x, vy = v.y;                                                              \
        asm volatile(                                                                            \
            "sust.p.1d.v2.b32." SLANG_PTX_BOUNDARY_MODE " [%0, {%1}], {%2, %3};" ::"l"(surfObj), \
            "r"(x),                                                                              \
            c(vx),                                                                               \
            c(vy));                                                                              \
    }                                                                                            \
    template<>                                                                                   \
    SLANG_FORCE_INLINE SLANG_CUDA_CALL void surf1Dwrite_convert<T##4>(                           \
        T##4 v,                                                                                  \
        cudaSurfaceObject_t surfObj,                                                             \
        int x,                                                                                   \
        cudaSurfaceBoundaryMode boundaryMode)                                                    \
    {                                                                                            \
        const T vx = v.x, vy = v.y, vz = v.z, vw = v.w;                                          \
        asm volatile(                                                                            \
            "sust.p.1d.v4.b32." SLANG_PTX_BOUNDARY_MODE                                          \
            " [%0, {%1}], {%2, %3, %4, %5};" ::"l"(surfObj),                                     \
            "r"(x),                                                                              \
            c(vx),                                                                               \
            c(vy),                                                                               \
            c(vz),                                                                               \
            c(vw));                                                                              \
    }

SLANG_SURF1DWRITE_CONVERT_IMPL(float, "f")
SLANG_SURF1DWRITE_CONVERT_IMPL(uint, "r")
SLANG_SURF1DWRITE_CONVERT_IMPL(int, "r")

// surf1DLayeredwrite_convert (not supported)

template<typename T>
SLANG_FORCE_INLINE SLANG_CUDA_CALL void surf1DLayeredwrite_convert(
    T v,
    cudaSurfaceObject_t surfObj,
    int x,
    int layer,
    cudaSurfaceBoundaryMode boundaryMode)
{
    // TODO: static_assert(false) can fail on some compilers, even if template is not instantiated.
    // We should check for this in hlsl.meta.slang instead.
    // static_assert(false, "CUDA doesn't support formatted surface writes on 1D array surfaces");
}

// surf2Dwrite_convert

template<typename T>
SLANG_FORCE_INLINE SLANG_CUDA_CALL void surf2Dwrite_convert(
    T v,
    cudaSurfaceObject_t surfObj,
    int x,
    int y,
    cudaSurfaceBoundaryMode boundaryMode);

#define SLANG_SURF2DWRITE_CONVERT_IMPL(T, c)                                                  \
    template<>                                                                                \
    SLANG_FORCE_INLINE SLANG_CUDA_CALL void surf2Dwrite_convert<T>(                           \
        T v,                                                                                  \
        cudaSurfaceObject_t surfObj,                                                          \
        int x,                                                                                \
        int y,                                                                                \
        cudaSurfaceBoundaryMode boundaryMode)                                                 \
    {                                                                                         \
        asm volatile(                                                                         \
            "sust.p.2d.b32." SLANG_PTX_BOUNDARY_MODE " [%0, {%1, %2}], {%3};" ::"l"(surfObj), \
            "r"(x),                                                                           \
            "r"(y),                                                                           \
            c(v));                                                                            \
    }                                                                                         \
    template<>                                                                                \
    SLANG_FORCE_INLINE SLANG_CUDA_CALL void surf2Dwrite_convert<T##2>(                        \
        T##2 v,                                                                               \
        cudaSurfaceObject_t surfObj,                                                          \
        int x,                                                                                \
        int y,                                                                                \
        cudaSurfaceBoundaryMode boundaryMode)                                                 \
    {                                                                                         \
        const T vx = v.x, vy = v.y;                                                           \
        asm volatile(                                                                         \
            "sust.p.2d.v2.b32." SLANG_PTX_BOUNDARY_MODE                                       \
            " [%0, {%1, %2}], {%3, %4};" ::"l"(surfObj),                                      \
            "r"(x),                                                                           \
            "r"(y),                                                                           \
            c(vx),                                                                            \
            c(vy));                                                                           \
    }                                                                                         \
    template<>                                                                                \
    SLANG_FORCE_INLINE SLANG_CUDA_CALL void surf2Dwrite_convert<T##4>(                        \
        T##4 v,                                                                               \
        cudaSurfaceObject_t surfObj,                                                          \
        int x,                                                                                \
        int y,                                                                                \
        cudaSurfaceBoundaryMode boundaryMode)                                                 \
    {                                                                                         \
        const T vx = v.x, vy = v.y, vz = v.z, vw = v.w;                                       \
        asm volatile(                                                                         \
            "sust.p.2d.v4.b32." SLANG_PTX_BOUNDARY_MODE                                       \
            " [%0, {%1, %2}], {%3, %4, %5, %6};" ::"l"(surfObj),                              \
            "r"(x),                                                                           \
            "r"(y),                                                                           \
            c(vx),                                                                            \
            c(vy),                                                                            \
            c(vz),                                                                            \
            c(vw));                                                                           \
    }

SLANG_SURF2DWRITE_CONVERT_IMPL(float, "f")
SLANG_SURF2DWRITE_CONVERT_IMPL(uint, "r")
SLANG_SURF2DWRITE_CONVERT_IMPL(int, "r")

// surf2DLayeredwrite_convert (not supported)

template<typename T>
SLANG_FORCE_INLINE SLANG_CUDA_CALL void surf2DLayeredwrite_convert(
    T v,
    cudaSurfaceObject_t surfObj,
    int x,
    int y,
    int layer,
    cudaSurfaceBoundaryMode boundaryMode)
{
    // TODO: static_assert(false) can fail on some compilers, even if template is not instantiated.
    // We should check for this in hlsl.meta.slang instead.
    // static_assert(false, "CUDA doesn't support formatted surface writes on 2D array surfaces");
}

// surf3Dwrite_convert

template<typename T>
SLANG_FORCE_INLINE SLANG_CUDA_CALL void surf3Dwrite_convert(
    T v,
    cudaSurfaceObject_t surfObj,
    int x,
    int y,
    int z,
    cudaSurfaceBoundaryMode boundaryMode);

#define SLANG_SURF3DWRITE_CONVERT_IMPL(T, c)                             \
    template<>                                                           \
    SLANG_FORCE_INLINE SLANG_CUDA_CALL void surf3Dwrite_convert<T>(      \
        T v,                                                             \
        cudaSurfaceObject_t surfObj,                                     \
        int x,                                                           \
        int y,                                                           \
        int z,                                                           \
        cudaSurfaceBoundaryMode boundaryMode)                            \
    {                                                                    \
        asm volatile(                                                    \
            "sust.p.3d.b32." SLANG_PTX_BOUNDARY_MODE                     \
            " [%0, {%1, %2, %3, %4}], {%5};" ::"l"(surfObj),             \
            "r"(x),                                                      \
            "r"(y),                                                      \
            "r"(z),                                                      \
            "r"(0),                                                      \
            c(v));                                                       \
    }                                                                    \
    template<>                                                           \
    SLANG_FORCE_INLINE SLANG_CUDA_CALL void surf3Dwrite_convert<T##2>(   \
        T##2 v,                                                          \
        cudaSurfaceObject_t surfObj,                                     \
        int x,                                                           \
        int y,                                                           \
        int z,                                                           \
        cudaSurfaceBoundaryMode boundaryMode)                            \
    {                                                                    \
        const T vx = v.x, vy = v.y;                                      \
        asm volatile(                                                    \
            "sust.p.3d.v2.b32." SLANG_PTX_BOUNDARY_MODE                  \
            " [%0, {%1, %2, %3, %4}], {%5, %6};" ::"l"(surfObj),         \
            "r"(x),                                                      \
            "r"(y),                                                      \
            "r"(z),                                                      \
            "r"(0),                                                      \
            c(vx),                                                       \
            c(vy));                                                      \
    }                                                                    \
    template<>                                                           \
    SLANG_FORCE_INLINE SLANG_CUDA_CALL void surf3Dwrite_convert<T##4>(   \
        T##4 v,                                                          \
        cudaSurfaceObject_t surfObj,                                     \
        int x,                                                           \
        int y,                                                           \
        int z,                                                           \
        cudaSurfaceBoundaryMode boundaryMode)                            \
    {                                                                    \
        const T vx = v.x, vy = v.y, vz = v.z, vw = v.w;                  \
        asm volatile(                                                    \
            "sust.p.3d.v4.b32." SLANG_PTX_BOUNDARY_MODE                  \
            " [%0, {%1, %2, %3, %4}], {%5, %6, %7, %8};" ::"l"(surfObj), \
            "r"(x),                                                      \
            "r"(y),                                                      \
            "r"(z),                                                      \
            "r"(0),                                                      \
            c(vx),                                                       \
            c(vy),                                                       \
            c(vz),                                                       \
            c(vw));                                                      \
    }

SLANG_SURF3DWRITE_CONVERT_IMPL(float, "f")
SLANG_SURF3DWRITE_CONVERT_IMPL(uint, "r")
SLANG_SURF3DWRITE_CONVERT_IMPL(int, "r")

// ----------------------------- F16 -----------------------------------------
#if SLANG_CUDA_ENABLE_HALF
// Unary
SLANG_FORCE_INLINE SLANG_CUDA_CALL __half F16_ceil(__half f)
{
    return ::hceil(f);
}

SLANG_FORCE_INLINE SLANG_CUDA_CALL __half F16_floor(__half f)
{
    return ::hfloor(f);
}

SLANG_FORCE_INLINE SLANG_CUDA_CALL __half F16_round(__half f)
{
    return ::hrint(f);
}

SLANG_FORCE_INLINE SLANG_CUDA_CALL __half F16_sin(__half f)
{
    return ::hsin(f);
}

SLANG_FORCE_INLINE SLANG_CUDA_CALL __half F16_cos(__half f)
{
    return ::hcos(f);
}

SLANG_FORCE_INLINE SLANG_CUDA_CALL void F16_sincos(__half f, __half* s, __half* c)
{
    *s = ::hsin(f);
    *c = ::hcos(f);
}

SLANG_FORCE_INLINE SLANG_CUDA_CALL __half F16_tan(__half f)
{
    return __float2half(::tanf(__half2float(f)));
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL __half F16_asin(__half f)
{
    return __float2half(::asinf(__half2float(f)));
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL __half F16_acos(__half f)
{
    return __float2half(::acosf(__half2float(f)));
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL __half F16_atan(__half f)
{
    return __float2half(::atanf(__half2float(f)));
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL __half F16_sinh(__half f)
{
    return __float2half(::sinhf(__half2float(f)));
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL __half F16_cosh(__half f)
{
    return __float2half(::coshf(__half2float(f)));
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL __half F16_tanh(__half f)
{
    return __float2half(::tanhf(__half2float(f)));
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL __half F16_asinh(__half f)
{
    return __float2half(::asinhf(__half2float(f)));
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL __half F16_acosh(__half f)
{
    return __float2half(::acoshf(__half2float(f)));
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL __half F16_atanh(__half f)
{
    return __float2half(::atanhf(__half2float(f)));
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL __half F16_log2(__half f)
{
    return ::hlog2(f);
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL __half F16_log(__half f)
{
    return ::hlog(f);
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL __half F16_log10(__half f)
{
    return ::hlog10(f);
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL __half F16_exp2(__half f)
{
    return ::hexp2(f);
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL __half F16_exp(__half f)
{
    return ::hexp(f);
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL __half F16_abs(__half f)
{
    return __habs(f);
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL __half F16_trunc(__half f)
{
    return ::htrunc(f);
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL __half F16_sqrt(__half f)
{
    return ::hsqrt(f);
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL __half F16_rsqrt(__half f)
{
    return ::hrsqrt(f);
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL int F16_sign(__half f)
{
    return (f == __half(0.0f)) ? 0 : ((f < __half(0.0f)) ? -1 : 1);
}

SLANG_FORCE_INLINE SLANG_CUDA_CALL __half F16_frac(__half f)
{
    return f - F16_floor(f);
}

SLANG_FORCE_INLINE SLANG_CUDA_CALL bool F16_isnan(__half f)
{
    return __hisnan(f);
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL bool F16_isfinite(__half f)
{
    return !__hisinf(f) && !__hisnan(f);
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL bool F16_isinf(__half f)
{
    return __hisinf(f);
}

// Binary
SLANG_FORCE_INLINE SLANG_CUDA_CALL __half F16_min(__half a, __half b)
{
    return __hmin(a, b);
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL __half F16_max(__half a, __half b)
{
    return __hmax(a, b);
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL __half F16_pow(__half a, __half b)
{
    return __float2half(::powf(__half2float(a), __half2float(b)));
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL __half F16_fmod(__half a, __half b)
{
    return __float2half(::fmodf(__half2float(a), __half2float(b)));
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL __half F16_remainder(__half a, __half b)
{
    return __float2half(::remainderf(__half2float(a), __half2float(b)));
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL __half F16_atan2(__half a, __half b)
{
    return __float2half(::atan2(__half2float(a), __half2float(b)));
}

SLANG_FORCE_INLINE SLANG_CUDA_CALL __half F16_frexp(__half x, int* e)
{
    return __float2half(frexpf(__half2float(x), e));
}

SLANG_FORCE_INLINE SLANG_CUDA_CALL __half F16_modf(__half x, __half* ip)
{
    float ipf;
    float res = ::modff(__half2float(x), &ipf);
    *ip = __float2half(ipf);
    return __float2half(res);
}

SLANG_FORCE_INLINE SLANG_CUDA_CALL uint16_t F16_asuint(__half h)
{
    return __half_as_ushort(h);
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL int16_t F16_asint(__half h)
{
    return __half_as_short(h);
}

// Ternary
SLANG_FORCE_INLINE SLANG_CUDA_CALL __half F16_fma(__half a, __half b, __half c)
{
    return __hfma(a, b, c);
}

#endif

// ----------------------------- F32 -----------------------------------------

// Unary
SLANG_FORCE_INLINE SLANG_CUDA_CALL float F32_ceil(float f)
{
    return ::ceilf(f);
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL float F32_floor(float f)
{
    return ::floorf(f);
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL float F32_round(float f)
{
    return ::roundf(f);
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL float F32_sin(float f)
{
    return ::sinf(f);
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL float F32_cos(float f)
{
    return ::cosf(f);
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL void F32_sincos(float f, float* s, float* c)
{
    ::sincosf(f, s, c);
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL float F32_tan(float f)
{
    return ::tanf(f);
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL float F32_asin(float f)
{
    return ::asinf(f);
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL float F32_acos(float f)
{
    return ::acosf(f);
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL float F32_atan(float f)
{
    return ::atanf(f);
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL float F32_sinh(float f)
{
    return ::sinhf(f);
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL float F32_cosh(float f)
{
    return ::coshf(f);
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL float F32_tanh(float f)
{
    return ::tanhf(f);
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL float F32_asinh(float f)
{
    return ::asinhf(f);
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL float F32_acosh(float f)
{
    return ::acoshf(f);
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL float F32_atanh(float f)
{
    return ::atanhf(f);
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL float F32_log2(float f)
{
    return ::log2f(f);
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL float F32_log(float f)
{
    return ::logf(f);
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL float F32_log10(float f)
{
    return ::log10f(f);
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL float F32_exp2(float f)
{
    return ::exp2f(f);
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL float F32_exp(float f)
{
    return ::expf(f);
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL float F32_abs(float f)
{
    return ::fabsf(f);
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL float F32_trunc(float f)
{
    return ::truncf(f);
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL float F32_sqrt(float f)
{
    return ::sqrtf(f);
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL float F32_rsqrt(float f)
{
    return ::rsqrtf(f);
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL int F32_sign(float f)
{
    return (f == 0.0f) ? 0 : ((f < 0.0f) ? -1 : 1);
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL float F32_frac(float f)
{
    return f - F32_floor(f);
}

SLANG_FORCE_INLINE SLANG_CUDA_CALL bool F32_isnan(float f)
{
    return isnan(f);
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL bool F32_isfinite(float f)
{
    return isfinite(f);
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL bool F32_isinf(float f)
{
    return isinf(f);
}

// Binary
SLANG_FORCE_INLINE SLANG_CUDA_CALL float F32_min(float a, float b)
{
    return ::fminf(a, b);
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL float F32_max(float a, float b)
{
    return ::fmaxf(a, b);
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL float F32_pow(float a, float b)
{
    return ::powf(a, b);
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL float F32_fmod(float a, float b)
{
    return ::fmodf(a, b);
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL float F32_remainder(float a, float b)
{
    return ::remainderf(a, b);
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL float F32_atan2(float a, float b)
{
    return float(::atan2(a, b));
}

SLANG_FORCE_INLINE SLANG_CUDA_CALL float F32_frexp(float x, int* e)
{
    return frexpf(x, e);
}

SLANG_FORCE_INLINE SLANG_CUDA_CALL float F32_modf(float x, float* ip)
{
    return ::modff(x, ip);
}

SLANG_FORCE_INLINE SLANG_CUDA_CALL uint32_t F32_asuint(float f)
{
    Union32 u;
    u.f = f;
    return u.u;
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL int32_t F32_asint(float f)
{
    Union32 u;
    u.f = f;
    return u.i;
}

// Ternary
SLANG_FORCE_INLINE SLANG_CUDA_CALL float F32_fma(float a, float b, float c)
{
    return ::fmaf(a, b, c);
}


// ----------------------------- F64 -----------------------------------------

// Unary
SLANG_FORCE_INLINE SLANG_CUDA_CALL double F64_ceil(double f)
{
    return ::ceil(f);
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL double F64_floor(double f)
{
    return ::floor(f);
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL double F64_round(double f)
{
    return ::round(f);
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL double F64_sin(double f)
{
    return ::sin(f);
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL double F64_cos(double f)
{
    return ::cos(f);
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL void F64_sincos(double f, double* s, double* c)
{
    ::sincos(f, s, c);
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL double F64_tan(double f)
{
    return ::tan(f);
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL double F64_asin(double f)
{
    return ::asin(f);
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL double F64_acos(double f)
{
    return ::acos(f);
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL double F64_atan(double f)
{
    return ::atan(f);
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL double F64_sinh(double f)
{
    return ::sinh(f);
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL double F64_cosh(double f)
{
    return ::cosh(f);
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL double F64_tanh(double f)
{
    return ::tanh(f);
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL double F64_log2(double f)
{
    return ::log2(f);
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL double F64_log(double f)
{
    return ::log(f);
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL double F64_log10(float f)
{
    return ::log10(f);
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL double F64_exp2(double f)
{
    return ::exp2(f);
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL double F64_exp(double f)
{
    return ::exp(f);
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL double F64_abs(double f)
{
    return ::fabs(f);
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL double F64_trunc(double f)
{
    return ::trunc(f);
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL double F64_sqrt(double f)
{
    return ::sqrt(f);
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL double F64_rsqrt(double f)
{
    return ::rsqrt(f);
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL int F64_sign(double f)
{
    return (f == 0.0) ? 0 : ((f < 0.0) ? -1 : 1);
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL double F64_frac(double f)
{
    return f - F64_floor(f);
}

SLANG_FORCE_INLINE SLANG_CUDA_CALL bool F64_isnan(double f)
{
    return isnan(f);
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL bool F64_isfinite(double f)
{
    return isfinite(f);
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL bool F64_isinf(double f)
{
    return isinf(f);
}

// Binary
SLANG_FORCE_INLINE SLANG_CUDA_CALL double F64_min(double a, double b)
{
    return ::fmin(a, b);
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL double F64_max(double a, double b)
{
    return ::fmax(a, b);
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL double F64_pow(double a, double b)
{
    return ::pow(a, b);
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL double F64_fmod(double a, double b)
{
    return ::fmod(a, b);
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL double F64_remainder(double a, double b)
{
    return ::remainder(a, b);
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL double F64_atan2(double a, double b)
{
    return ::atan2(a, b);
}

SLANG_FORCE_INLINE SLANG_CUDA_CALL double F64_frexp(double x, int* e)
{
    return ::frexp(x, e);
}

SLANG_FORCE_INLINE SLANG_CUDA_CALL double F64_modf(double x, double* ip)
{
    return ::modf(x, ip);
}

SLANG_FORCE_INLINE SLANG_CUDA_CALL void F64_asuint(double d, uint32_t* low, uint32_t* hi)
{
    Union64 u;
    u.d = d;
    *low = uint32_t(u.u);
    *hi = uint32_t(u.u >> 32);
}

SLANG_FORCE_INLINE SLANG_CUDA_CALL void F64_asint(double d, int32_t* low, int32_t* hi)
{
    Union64 u;
    u.d = d;
    *low = int32_t(u.u);
    *hi = int32_t(u.u >> 32);
}

// Ternary
SLANG_FORCE_INLINE SLANG_CUDA_CALL double F64_fma(double a, double b, double c)
{
    return ::fma(a, b, c);
}

// ----------------------------- U8 -----------------------------------------

SLANG_FORCE_INLINE SLANG_CUDA_CALL uint32_t U8_countbits(uint8_t v)
{
    // No native 8bit popc yet, just cast and use 32bit variant
    return __popc(uint32_t(v));
}

// ----------------------------- I8 -----------------------------------------

SLANG_FORCE_INLINE SLANG_CUDA_CALL uint32_t I8_countbits(int8_t v)
{
    return U8_countbits(uint8_t(v));
}

// ----------------------------- U16 -----------------------------------------

SLANG_FORCE_INLINE SLANG_CUDA_CALL uint32_t U16_countbits(uint16_t v)
{
    // No native 16bit popc yet, just cast and use 32bit variant
    return __popc(uint32_t(v));
}

// ----------------------------- I16 -----------------------------------------

SLANG_FORCE_INLINE SLANG_CUDA_CALL uint32_t I16_countbits(int16_t v)
{
    return U16_countbits(uint16_t(v));
}

// ----------------------------- U32 -----------------------------------------

// Unary
SLANG_FORCE_INLINE SLANG_CUDA_CALL uint32_t U32_abs(uint32_t f)
{
    return f;
}

// Binary
SLANG_FORCE_INLINE SLANG_CUDA_CALL uint32_t U32_min(uint32_t a, uint32_t b)
{
    return a < b ? a : b;
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL uint32_t U32_max(uint32_t a, uint32_t b)
{
    return a > b ? a : b;
}

SLANG_FORCE_INLINE SLANG_CUDA_CALL float U32_asfloat(uint32_t x)
{
    Union32 u;
    u.u = x;
    return u.f;
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL uint32_t U32_asint(int32_t x)
{
    return uint32_t(x);
}

SLANG_FORCE_INLINE SLANG_CUDA_CALL double U32_asdouble(uint32_t low, uint32_t hi)
{
    Union64 u;
    u.u = (uint64_t(hi) << 32) | low;
    return u.d;
}

SLANG_FORCE_INLINE SLANG_CUDA_CALL uint32_t U32_countbits(uint32_t v)
{
    return __popc(v);
}

SLANG_FORCE_INLINE SLANG_CUDA_CALL uint32_t U32_firstbitlow(uint32_t v)
{
    // __ffs returns 1-based bit position or 0 if no bits set
    // firstbitlow should return 0-based bit position or ~0u if no bits set
    return v == 0 ? ~0u : (__ffs(v) - 1);
}

SLANG_FORCE_INLINE SLANG_CUDA_CALL uint32_t U32_firstbithigh(uint32_t v)
{
    // maps to hlsl firstbithigh
    if ((int32_t)v < 0)
        v = ~v;
    if (v == 0)
        return ~0u;
    return 31 - __clz(v);
}

SLANG_FORCE_INLINE SLANG_CUDA_CALL uint32_t U32_reversebits(uint32_t v)
{
    return __brev(v);
}

// ----------------------------- I32 -----------------------------------------

// Unary
SLANG_FORCE_INLINE SLANG_CUDA_CALL int32_t I32_abs(int32_t f)
{
    return (f < 0) ? -f : f;
}

// Binary
SLANG_FORCE_INLINE SLANG_CUDA_CALL int32_t I32_min(int32_t a, int32_t b)
{
    return a < b ? a : b;
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL int32_t I32_max(int32_t a, int32_t b)
{
    return a > b ? a : b;
}

SLANG_FORCE_INLINE SLANG_CUDA_CALL float I32_asfloat(int32_t x)
{
    Union32 u;
    u.i = x;
    return u.f;
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL uint32_t I32_asuint(int32_t x)
{
    return uint32_t(x);
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL double I32_asdouble(int32_t low, int32_t hi)
{
    Union64 u;
    u.u = (uint64_t(hi) << 32) | uint32_t(low);
    return u.d;
}

SLANG_FORCE_INLINE SLANG_CUDA_CALL uint32_t I32_countbits(int32_t v)
{
    return U32_countbits(uint32_t(v));
}

SLANG_FORCE_INLINE SLANG_CUDA_CALL uint32_t I32_firstbitlow(int32_t v)
{
    return U32_firstbitlow(uint32_t(v));
}

SLANG_FORCE_INLINE SLANG_CUDA_CALL uint32_t I32_firstbithigh(int32_t v)
{
    return U32_firstbithigh(uint32_t(v));
}

SLANG_FORCE_INLINE SLANG_CUDA_CALL int32_t I32_reversebits(int32_t v)
{
    return int32_t(U32_reversebits(uint32_t(v)));
}

// ----------------------------- U64 -----------------------------------------

SLANG_FORCE_INLINE SLANG_CUDA_CALL int64_t U64_abs(uint64_t f)
{
    return f;
}

SLANG_FORCE_INLINE SLANG_CUDA_CALL int64_t U64_min(uint64_t a, uint64_t b)
{
    return a < b ? a : b;
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL int64_t U64_max(uint64_t a, uint64_t b)
{
    return a > b ? a : b;
}

SLANG_FORCE_INLINE SLANG_CUDA_CALL uint32_t U64_countbits(uint64_t v)
{
    return __popcll(v);
}

SLANG_FORCE_INLINE SLANG_CUDA_CALL uint32_t U64_firstbitlow(uint64_t v)
{
    // __ffs returns 1-based bit position or 0 if no bits set
    // firstbitlow should return 0-based bit position or ~0u if no bits set
    return v == 0 ? ~uint32_t(0) : (__ffsll(v) - 1u);
}

SLANG_FORCE_INLINE SLANG_CUDA_CALL uint32_t U64_firstbithigh(uint64_t v)
{
    if (v == 0)
        return ~uint32_t(0);
    return 63 - __clzll(v);
}

SLANG_FORCE_INLINE SLANG_CUDA_CALL uint64_t U64_reversebits(uint64_t v)
{
    return __brevll(v);
}

// ----------------------------- I64 -----------------------------------------

SLANG_FORCE_INLINE SLANG_CUDA_CALL int64_t I64_abs(int64_t f)
{
    return (f < 0) ? -f : f;
}

SLANG_FORCE_INLINE SLANG_CUDA_CALL int64_t I64_min(int64_t a, int64_t b)
{
    return a < b ? a : b;
}
SLANG_FORCE_INLINE SLANG_CUDA_CALL int64_t I64_max(int64_t a, int64_t b)
{
    return a > b ? a : b;
}

SLANG_FORCE_INLINE SLANG_CUDA_CALL uint32_t I64_countbits(int64_t v)
{
    return U64_countbits(uint64_t(v));
}

SLANG_FORCE_INLINE SLANG_CUDA_CALL uint32_t I64_firstbitlow(int64_t v)
{
    return U64_firstbitlow(uint64_t(v));
}

SLANG_FORCE_INLINE SLANG_CUDA_CALL uint32_t I64_firstbithigh(int64_t v)
{
    if (v < 0)
        v = ~v;
    return U64_firstbithigh(uint64_t(v));
}

SLANG_FORCE_INLINE SLANG_CUDA_CALL int64_t I64_reversebits(int64_t v)
{
    return int64_t(U64_reversebits(uint64_t(v)));
}

// ----------------------------- IPTR -----------------------------------------

SLANG_FORCE_INLINE SLANG_CUDA_CALL intptr_t IPTR_abs(intptr_t f)
{
    return (f < 0) ? -f : f;
}

SLANG_FORCE_INLINE SLANG_CUDA_CALL intptr_t IPTR_min(intptr_t a, intptr_t b)
{
    return a < b ? a : b;
}

SLANG_FORCE_INLINE SLANG_CUDA_CALL intptr_t IPTR_max(intptr_t a, intptr_t b)
{
    return a > b ? a : b;
}

// ----------------------------- UPTR -----------------------------------------

SLANG_FORCE_INLINE SLANG_CUDA_CALL uintptr_t UPTR_abs(uintptr_t f)
{
    return f;
}

SLANG_FORCE_INLINE SLANG_CUDA_CALL uintptr_t UPTR_min(uintptr_t a, uintptr_t b)
{
    return a < b ? a : b;
}

SLANG_FORCE_INLINE SLANG_CUDA_CALL uintptr_t UPTR_max(uintptr_t a, uintptr_t b)
{
    return a > b ? a : b;
}

// ----------------------------- ResourceType -----------------------------------------


// https://docs.microsoft.com/en-us/windows/win32/direct3dhlsl/sm5-object-structuredbuffer-getdimensions
// Missing  Load(_In_  int  Location, _Out_ uint Status);

template<typename T>
struct StructuredBuffer
{
    SLANG_CUDA_CALL T& operator[](size_t index) const
    {
#ifndef SLANG_CUDA_STRUCTURED_BUFFER_NO_COUNT
        SLANG_BOUND_CHECK(index, count);
#endif
        return data[index];
    }

    SLANG_CUDA_CALL T& Load(size_t index) const
    {
#ifndef SLANG_CUDA_STRUCTURED_BUFFER_NO_COUNT
        SLANG_BOUND_CHECK(index, count);
#endif
        return data[index];
    }

#ifndef SLANG_CUDA_STRUCTURED_BUFFER_NO_COUNT
    SLANG_CUDA_CALL void GetDimensions(uint32_t* outNumStructs, uint32_t* outStride) const
    {
        *outNumStructs = uint32_t(count);
        *outStride = uint32_t(sizeof(T));
    }
#endif

    T* data;
#ifndef SLANG_CUDA_STRUCTURED_BUFFER_NO_COUNT
    size_t count;
#endif
};

template<typename T>
struct RWStructuredBuffer : StructuredBuffer<T>
{
    SLANG_CUDA_CALL T& operator[](size_t index) const
    {
#ifndef SLANG_CUDA_STRUCTURED_BUFFER_NO_COUNT
        SLANG_BOUND_CHECK(index, this->count);
#endif
        return this->data[index];
    }
};

// Missing  Load(_In_  int  Location, _Out_ uint Status);
struct ByteAddressBuffer
{
    SLANG_CUDA_CALL void GetDimensions(uint32_t* outDim) const { *outDim = uint32_t(sizeInBytes); }
    SLANG_CUDA_CALL uint32_t Load(size_t index) const
    {
        SLANG_BOUND_CHECK_BYTE_ADDRESS(index, 4, sizeInBytes);
        return data[index >> 2];
    }
    SLANG_CUDA_CALL uint2 Load2(size_t index) const
    {
        SLANG_BOUND_CHECK_BYTE_ADDRESS(index, 8, sizeInBytes);
        const size_t dataIdx = index >> 2;
        return uint2{data[dataIdx], data[dataIdx + 1]};
    }
    SLANG_CUDA_CALL uint3 Load3(size_t index) const
    {
        SLANG_BOUND_CHECK_BYTE_ADDRESS(index, 12, sizeInBytes);
        const size_t dataIdx = index >> 2;
        return uint3{data[dataIdx], data[dataIdx + 1], data[dataIdx + 2]};
    }
    SLANG_CUDA_CALL uint4 Load4(size_t index) const
    {
        SLANG_BOUND_CHECK_BYTE_ADDRESS(index, 16, sizeInBytes);
        const size_t dataIdx = index >> 2;
        return uint4{data[dataIdx], data[dataIdx + 1], data[dataIdx + 2], data[dataIdx + 3]};
    }
    template<typename T>
    SLANG_CUDA_CALL T Load(size_t index) const
    {
        SLANG_BOUND_CHECK_BYTE_ADDRESS(index, sizeof(T), sizeInBytes);
        T data;
        memcpy(&data, ((const char*)this->data) + index, sizeof(T));
        return data;
    }
    template<typename T>
    SLANG_CUDA_CALL StructuredBuffer<T> asStructuredBuffer() const
    {
        StructuredBuffer<T> rs;
        rs.data = (T*)data;
        rs.count = sizeInBytes / sizeof(T);
        return rs;
    }
    const uint32_t* data;
    size_t sizeInBytes; //< Must be multiple of 4
};

// https://docs.microsoft.com/en-us/windows/win32/direct3dhlsl/sm5-object-rwbyteaddressbuffer
// Atomic operations support

// Signed 64-bit atomic wrappers
// CUDA only supports unsigned long long atomics, so we cast signed to unsigned
// Use longlong type with explicit unsigned long long casts for platform portability
__device__ __forceinline__ longlong atomicExch(longlong* address, longlong val)
{
    return (longlong)atomicExch((unsigned long long*)address, (unsigned long long)val);
}

__device__ __forceinline__ longlong atomicCAS(longlong* address, longlong compare, longlong val)
{
    return (longlong)atomicCAS(
        (unsigned long long*)address,
        (unsigned long long)compare,
        (unsigned long long)val);
}

__device__ __forceinline__ longlong atomicAdd(longlong* address, longlong val)
{
    return (longlong)atomicAdd((unsigned long long*)address, (unsigned long long)val);
}

// Float bitwise atomic compare-and-swap
// Uses integer atomics to preserve exact float bit patterns
__device__ __forceinline__ float atomicCAS(float* address, float compare, float val)
{
    int* addr_as_int = (int*)address;
    int old = atomicCAS(addr_as_int, __float_as_int(compare), __float_as_int(val));
    return __int_as_float(old);
}

// =====================================================================
// Atomic Reduction Operations (PTX `red` instruction)
// These are in-place atomic operations that don't return the old value.
// They are faster than the corresponding atomic operations that return values
// because they use the PTX `red` instruction with relaxed memory ordering.
//
// Supported operations based on PTX ISA:
// - add: .s32, .u32, .u64, .s64, .f16, .f16x2, .bf16, .bf16x2, .f32, .f64
// - min/max: .s32, .u32, .s64, .u64, .f32, .f64, .f16, .f16x2
// - and/or/xor: .b32, .b64
// - inc/dec: .u32
// =====================================================================

// Atomic reduction ADD operations
__device__ __forceinline__ void __slang_atomic_reduce_add(int32_t* addr, int32_t val, int order)
{
    asm volatile("red.relaxed.gpu.global.add.s32 [%0], %1;" : : "l"(addr), "r"(val) : "memory");
}

__device__ __forceinline__ void __slang_atomic_reduce_add(uint32_t* addr, uint32_t val, int order)
{
    asm volatile("red.relaxed.gpu.global.add.u32 [%0], %1;" : : "l"(addr), "r"(val) : "memory");
}

__device__ __forceinline__ void __slang_atomic_reduce_add(int64_t* addr, int64_t val, int order)
{
    asm volatile("red.relaxed.gpu.global.add.s64 [%0], %1;" : : "l"(addr), "l"(val) : "memory");
}

__device__ __forceinline__ void __slang_atomic_reduce_add(uint64_t* addr, uint64_t val, int order)
{
    asm volatile("red.relaxed.gpu.global.add.u64 [%0], %1;" : : "l"(addr), "l"(val) : "memory");
}

__device__ __forceinline__ void __slang_atomic_reduce_add(float* addr, float val, int order)
{
    asm volatile("red.relaxed.gpu.global.add.f32 [%0], %1;" : : "l"(addr), "f"(val) : "memory");
}

__device__ __forceinline__ void __slang_atomic_reduce_add(double* addr, double val, int order)
{
    asm volatile("red.relaxed.gpu.global.add.f64 [%0], %1;" : : "l"(addr), "d"(val) : "memory");
}

#if SLANG_CUDA_ENABLE_HALF
__device__ __forceinline__ void __slang_atomic_reduce_add(__half* addr, __half val, int order)
{
    unsigned short val_as_ushort = *reinterpret_cast<unsigned short*>(&val);
    asm volatile("red.relaxed.gpu.global.add.noftz.f16 [%0], %1;"
                 :
                 : "l"(addr), "h"(val_as_ushort)
                 : "memory");
}

__device__ __forceinline__ void __slang_atomic_reduce_add(__half2* addr, __half2 val, int order)
{
    unsigned int val_as_uint = *reinterpret_cast<unsigned int*>(&val);
    asm volatile("red.relaxed.gpu.global.add.noftz.f16x2 [%0], %1;"
                 :
                 : "l"(addr), "r"(val_as_uint)
                 : "memory");
}
#endif

#if SLANG_CUDA_ENABLE_BF16
__device__ __forceinline__ void __slang_atomic_reduce_add(
    __nv_bfloat16* addr,
    __nv_bfloat16 val,
    int order)
{
    unsigned short val_as_ushort = *reinterpret_cast<unsigned short*>(&val);
    asm volatile("red.relaxed.gpu.global.add.noftz.bf16 [%0], %1;"
                 :
                 : "l"(addr), "h"(val_as_ushort)
                 : "memory");
}

__device__ __forceinline__ void __slang_atomic_reduce_add(
    __nv_bfloat162* addr,
    __nv_bfloat162 val,
    int order)
{
    unsigned int val_as_uint = *reinterpret_cast<unsigned int*>(&val);
    asm volatile("red.relaxed.gpu.global.add.noftz.bf16x2 [%0], %1;"
                 :
                 : "l"(addr), "r"(val_as_uint)
                 : "memory");
}
#endif

// Atomic reduction MIN operations
__device__ __forceinline__ void __slang_atomic_reduce_min(int32_t* addr, int32_t val, int order)
{
    asm volatile("red.relaxed.gpu.global.min.s32 [%0], %1;" : : "l"(addr), "r"(val) : "memory");
}

__device__ __forceinline__ void __slang_atomic_reduce_min(uint32_t* addr, uint32_t val, int order)
{
    asm volatile("red.relaxed.gpu.global.min.u32 [%0], %1;" : : "l"(addr), "r"(val) : "memory");
}

__device__ __forceinline__ void __slang_atomic_reduce_min(int64_t* addr, int64_t val, int order)
{
    asm volatile("red.relaxed.gpu.global.min.s64 [%0], %1;" : : "l"(addr), "l"(val) : "memory");
}

__device__ __forceinline__ void __slang_atomic_reduce_min(uint64_t* addr, uint64_t val, int order)
{
    asm volatile("red.relaxed.gpu.global.min.u64 [%0], %1;" : : "l"(addr), "l"(val) : "memory");
}

// NOTE: PTX `red` instruction does NOT support min/max for floating-point types.
// Only integer types (.u32, .u64, .s32, .s64) are supported for min/max.
// For floating-point min/max atomics, use the regular `atom` instruction via
// __atomic_min/__atomic_max.

// Atomic reduction MAX operations
__device__ __forceinline__ void __slang_atomic_reduce_max(int32_t* addr, int32_t val, int order)
{
    asm volatile("red.relaxed.gpu.global.max.s32 [%0], %1;" : : "l"(addr), "r"(val) : "memory");
}

__device__ __forceinline__ void __slang_atomic_reduce_max(uint32_t* addr, uint32_t val, int order)
{
    asm volatile("red.relaxed.gpu.global.max.u32 [%0], %1;" : : "l"(addr), "r"(val) : "memory");
}

__device__ __forceinline__ void __slang_atomic_reduce_max(int64_t* addr, int64_t val, int order)
{
    asm volatile("red.relaxed.gpu.global.max.s64 [%0], %1;" : : "l"(addr), "l"(val) : "memory");
}

__device__ __forceinline__ void __slang_atomic_reduce_max(uint64_t* addr, uint64_t val, int order)
{
    asm volatile("red.relaxed.gpu.global.max.u64 [%0], %1;" : : "l"(addr), "l"(val) : "memory");
}

// NOTE: PTX `red` instruction does NOT support min/max for floating-point types.
// Only integer types (.u32, .u64, .s32, .s64) are supported for min/max.
// For floating-point min/max atomics, use the regular `atom` instruction via
// __atomic_min/__atomic_max.

// Atomic reduction AND operations (bitwise, integers only)
__device__ __forceinline__ void __slang_atomic_reduce_and(int32_t* addr, int32_t val, int order)
{
    asm volatile("red.relaxed.gpu.global.and.b32 [%0], %1;" : : "l"(addr), "r"(val) : "memory");
}

__device__ __forceinline__ void __slang_atomic_reduce_and(uint32_t* addr, uint32_t val, int order)
{
    asm volatile("red.relaxed.gpu.global.and.b32 [%0], %1;" : : "l"(addr), "r"(val) : "memory");
}

__device__ __forceinline__ void __slang_atomic_reduce_and(int64_t* addr, int64_t val, int order)
{
    asm volatile("red.relaxed.gpu.global.and.b64 [%0], %1;" : : "l"(addr), "l"(val) : "memory");
}

__device__ __forceinline__ void __slang_atomic_reduce_and(uint64_t* addr, uint64_t val, int order)
{
    asm volatile("red.relaxed.gpu.global.and.b64 [%0], %1;" : : "l"(addr), "l"(val) : "memory");
}

// Atomic reduction OR operations (bitwise, integers only)
__device__ __forceinline__ void __slang_atomic_reduce_or(int32_t* addr, int32_t val, int order)
{
    asm volatile("red.relaxed.gpu.global.or.b32 [%0], %1;" : : "l"(addr), "r"(val) : "memory");
}

__device__ __forceinline__ void __slang_atomic_reduce_or(uint32_t* addr, uint32_t val, int order)
{
    asm volatile("red.relaxed.gpu.global.or.b32 [%0], %1;" : : "l"(addr), "r"(val) : "memory");
}

__device__ __forceinline__ void __slang_atomic_reduce_or(int64_t* addr, int64_t val, int order)
{
    asm volatile("red.relaxed.gpu.global.or.b64 [%0], %1;" : : "l"(addr), "l"(val) : "memory");
}

__device__ __forceinline__ void __slang_atomic_reduce_or(uint64_t* addr, uint64_t val, int order)
{
    asm volatile("red.relaxed.gpu.global.or.b64 [%0], %1;" : : "l"(addr), "l"(val) : "memory");
}

// Atomic reduction XOR operations (bitwise, integers only)
__device__ __forceinline__ void __slang_atomic_reduce_xor(int32_t* addr, int32_t val, int order)
{
    asm volatile("red.relaxed.gpu.global.xor.b32 [%0], %1;" : : "l"(addr), "r"(val) : "memory");
}

__device__ __forceinline__ void __slang_atomic_reduce_xor(uint32_t* addr, uint32_t val, int order)
{
    asm volatile("red.relaxed.gpu.global.xor.b32 [%0], %1;" : : "l"(addr), "r"(val) : "memory");
}

__device__ __forceinline__ void __slang_atomic_reduce_xor(int64_t* addr, int64_t val, int order)
{
    asm volatile("red.relaxed.gpu.global.xor.b64 [%0], %1;" : : "l"(addr), "l"(val) : "memory");
}

__device__ __forceinline__ void __slang_atomic_reduce_xor(uint64_t* addr, uint64_t val, int order)
{
    asm volatile("red.relaxed.gpu.global.xor.b64 [%0], %1;" : : "l"(addr), "l"(val) : "memory");
}

// Atomic reduction INC/DEC operations (unsigned 32-bit only in PTX)
// Note: PTX inc/dec have specific semantics:
//   inc: d = (old >= b) ? 0 : old + 1
//   dec: d = ((old == 0) || (old > b)) ? b : old - 1
// For simple increment by 1, we use add instead
__device__ __forceinline__ void __slang_atomic_reduce_inc(uint32_t* addr, int order)
{
    asm volatile("red.relaxed.gpu.global.add.u32 [%0], 1;" : : "l"(addr) : "memory");
}

__device__ __forceinline__ void __slang_atomic_reduce_inc(int32_t* addr, int order)
{
    asm volatile("red.relaxed.gpu.global.add.s32 [%0], 1;" : : "l"(addr) : "memory");
}

__device__ __forceinline__ void __slang_atomic_reduce_dec(uint32_t* addr, int order)
{
    asm volatile("red.relaxed.gpu.global.add.u32 [%0], -1;" : : "l"(addr) : "memory");
}

__device__ __forceinline__ void __slang_atomic_reduce_dec(int32_t* addr, int order)
{
    asm volatile("red.relaxed.gpu.global.add.s32 [%0], -1;" : : "l"(addr) : "memory");
}

// =====================================================================
// End of Atomic Reduction Operations
// =====================================================================

// Missing support for Load with status
struct RWByteAddressBuffer
{
    SLANG_CUDA_CALL void GetDimensions(uint32_t* outDim) const { *outDim = uint32_t(sizeInBytes); }

    SLANG_CUDA_CALL uint32_t Load(size_t index) const
    {
        SLANG_BOUND_CHECK_BYTE_ADDRESS(index, 4, sizeInBytes);
        return data[index >> 2];
    }
    SLANG_CUDA_CALL uint2 Load2(size_t index) const
    {
        SLANG_BOUND_CHECK_BYTE_ADDRESS(index, 8, sizeInBytes);
        const size_t dataIdx = index >> 2;
        return uint2{data[dataIdx], data[dataIdx + 1]};
    }
    SLANG_CUDA_CALL uint3 Load3(size_t index) const
    {
        SLANG_BOUND_CHECK_BYTE_ADDRESS(index, 12, sizeInBytes);
        const size_t dataIdx = index >> 2;
        return uint3{data[dataIdx], data[dataIdx + 1], data[dataIdx + 2]};
    }
    SLANG_CUDA_CALL uint4 Load4(size_t index) const
    {
        SLANG_BOUND_CHECK_BYTE_ADDRESS(index, 16, sizeInBytes);
        const size_t dataIdx = index >> 2;
        return uint4{data[dataIdx], data[dataIdx + 1], data[dataIdx + 2], data[dataIdx + 3]};
    }
    template<typename T>
    SLANG_CUDA_CALL T Load(size_t index) const
    {
        SLANG_BOUND_CHECK_BYTE_ADDRESS(index, sizeof(T), sizeInBytes);
        T data;
        memcpy(&data, ((const char*)this->data) + index, sizeof(T));
        return data;
    }

    SLANG_CUDA_CALL void Store(size_t index, uint32_t v) const
    {
        SLANG_BOUND_CHECK_BYTE_ADDRESS(index, 4, sizeInBytes);
        data[index >> 2] = v;
    }
    SLANG_CUDA_CALL void Store2(size_t index, uint2 v) const
    {
        SLANG_BOUND_CHECK_BYTE_ADDRESS(index, 8, sizeInBytes);
        const size_t dataIdx = index >> 2;
        data[dataIdx + 0] = v.x;
        data[dataIdx + 1] = v.y;
    }
    SLANG_CUDA_CALL void Store3(size_t index, uint3 v) const
    {
        SLANG_BOUND_CHECK_BYTE_ADDRESS(index, 12, sizeInBytes);
        const size_t dataIdx = index >> 2;
        data[dataIdx + 0] = v.x;
        data[dataIdx + 1] = v.y;
        data[dataIdx + 2] = v.z;
    }
    SLANG_CUDA_CALL void Store4(size_t index, uint4 v) const
    {
        SLANG_BOUND_CHECK_BYTE_ADDRESS(index, 16, sizeInBytes);
        const size_t dataIdx = index >> 2;
        data[dataIdx + 0] = v.x;
        data[dataIdx + 1] = v.y;
        data[dataIdx + 2] = v.z;
        data[dataIdx + 3] = v.w;
    }
    template<typename T>
    SLANG_CUDA_CALL void Store(size_t index, T const& value) const
    {
        SLANG_BOUND_CHECK_BYTE_ADDRESS(index, sizeof(T), sizeInBytes);
        memcpy((char*)data + index, &value, sizeof(T));
    }

    /// Can be used in the core module to gain access
    template<typename T>
    SLANG_CUDA_CALL T* _getPtrAt(size_t index)
    {
        SLANG_BOUND_CHECK_BYTE_ADDRESS(index, sizeof(T), sizeInBytes);
        return (T*)(((char*)data) + index);
    }
    template<typename T>
    SLANG_CUDA_CALL RWStructuredBuffer<T> asStructuredBuffer() const
    {
        RWStructuredBuffer<T> rs;
        rs.data = (T*)data;
        rs.count = sizeInBytes / sizeof(T);
        return rs;
    }
    uint32_t* data;
    size_t sizeInBytes; //< Must be multiple of 4
};


// ---------------------- Wave --------------------------------------

// TODO(JS): It appears that cuda does not have a simple way to get a lane index.
//
// Another approach could be...
// laneId = ((threadIdx.z * blockDim.y + threadIdx.y) * blockDim.x + threadIdx.x) &
// SLANG_CUDA_WARP_MASK If that is really true another way to do this, would be for code generator
// to add this function with the [numthreads] baked in.
//
// For now I'll just assume you have a launch that makes the following correct if the kernel uses
// WaveGetLaneIndex()
#ifndef SLANG_USE_ASM_LANE_ID
__forceinline__ __device__ uint32_t _getLaneId()
{
    return ((threadIdx.z * blockDim.y + threadIdx.y) * blockDim.x + threadIdx.x) &
           SLANG_CUDA_WARP_MASK;
}
#else
__forceinline__ __device__ uint32_t _getLaneId()
{
    // https://stackoverflow.com/questions/44337309/whats-the-most-efficient-way-to-calculate-the-warp-id-lane-id-in-a-1-d-grid#
    // This mechanism is not the fastest way to do it, and that is why the other mechanism
    // is the default. But the other mechanism relies on a launch that makes the assumption
    // true.
    unsigned ret;
    asm volatile("mov.u32 %0, %laneid;" : "=r"(ret));
    return ret;
}
#endif

typedef int WarpMask;

// It appears that the __activemask() cannot always be used because
// threads need to be converged.
//
// For CUDA the article claims mask has to be used carefully
// https://devblogs.nvidia.com/using-cuda-warp-level-primitives/
// With the Warp intrinsics there is no mask, and it's just the 'active lanes'.
// __activemask() though does not require there is convergence, so that doesn't work.
//
// '__ballot_sync' produces a convergance.
//
// From the CUDA docs:
// ```For __all_sync, __any_sync, and __ballot_sync, a mask must be passed that specifies the
// threads participating in the call. A bit, representing the thread's lane ID, must be set for each
// participating thread to ensure they are properly converged before the intrinsic is executed by
// the hardware. All active threads named in mask must execute the same intrinsic with the same
// mask, or the result is undefined.```
//
// Currently there isn't a mechanism to correctly get the mask without it being passed through.
// Doing so will most likely require some changes to slang code generation to track masks, for now
// then we use _getActiveMask.

// Return mask of all the lanes less than the current lane
__forceinline__ __device__ WarpMask _getLaneLtMask()
{
    return (int(1) << _getLaneId()) - 1;
}

// TODO(JS):
// THIS IS NOT CORRECT! That determining the appropriate active mask requires appropriate
// mask tracking.
__forceinline__ __device__ WarpMask _getActiveMask()
{
    return __ballot_sync(__activemask(), true);
}

// Return a mask suitable for the 'MultiPrefix' style functions
__forceinline__ __device__ WarpMask _getMultiPrefixMask(int mask)
{
    return mask;
}

// Note! Note will return true if mask is 0, but thats okay, because there must be one
// lane active to execute anything
__inline__ __device__ bool _waveIsSingleLane(WarpMask mask)
{
    return (mask & (mask - 1)) == 0;
}

// Returns the power of 2 size of run of set bits. Returns 0 if not a suitable run.
// Examples:
// 0b00000000'00000000'00000000'11111111 -> 8
// 0b11111111'11111111'11111111'11111111 -> 32
// 0b00000000'00000000'00000000'00011111 -> 0 (since 5 is not a power of 2)
// 0b00000000'00000000'00000000'11110000 -> 0 (since the run of bits does not start at the LSB)
// 0b00000000'00000000'00000000'00100111 -> 0 (since it is not a single contiguous run)
__inline__ __device__ int _waveCalcPow2Offset(WarpMask mask)
{
    // This should be the most common case, so fast path it
    if (mask == SLANG_CUDA_WARP_BITMASK)
    {
        return SLANG_CUDA_WARP_SIZE;
    }
    // Is it a contiguous run of bits?
    if ((mask & (mask + 1)) == 0)
    {
        // const int offsetSize = __ffs(mask + 1) - 1;
        const int offset = 32 - __clz(mask);
        // Is it a power of 2 size
        if ((offset & (offset - 1)) == 0)
        {
            return offset;
        }
    }
    return 0;
}

__inline__ __device__ bool _waveIsFirstLane()
{
    const WarpMask mask = __activemask();
    // We special case bit 0, as that most warps are expected to be fully active.

    // mask & -mask, isolates the lowest set bit.
    // return (mask & 1 ) || ((mask & -mask) == (1 << _getLaneId()));

    // This mechanism is most similar to what was in an nVidia post, so assume it is prefered.
    return (mask & 1) || ((__ffs(mask) - 1) == _getLaneId());
}

template<typename T>
struct WaveOpOr
{
    __inline__ __device__ static T getInitial(T a) { return 0; }
    __inline__ __device__ static T doOp(T a, T b) { return a | b; }
};

template<typename T>
struct WaveOpAnd
{
    __inline__ __device__ static T getInitial(T a) { return ~T(0); }
    __inline__ __device__ static T doOp(T a, T b) { return a & b; }
};

template<typename T>
struct WaveOpXor
{
    __inline__ __device__ static T getInitial(T a) { return 0; }
    __inline__ __device__ static T doOp(T a, T b) { return a ^ b; }
    __inline__ __device__ static T doInverse(T a, T b) { return a ^ b; }
};

template<typename T>
struct WaveOpAdd
{
    __inline__ __device__ static T getInitial(T a) { return 0; }
    __inline__ __device__ static T doOp(T a, T b) { return a + b; }
    __inline__ __device__ static T doInverse(T a, T b) { return a - b; }
};

template<typename T>
struct WaveOpMul
{
    __inline__ __device__ static T getInitial(T a) { return T(1); }
    __inline__ __device__ static T doOp(T a, T b) { return a * b; }
    // Using this inverse for int is probably undesirable - because in general it requires T to have
    // more precision There is also a performance aspect to it, where divides are generally
    // significantly slower
    __inline__ __device__ static T doInverse(T a, T b) { return a / b; }
};

template<typename T>
struct WaveOpMax
{
    __inline__ __device__ static T getInitial(T a, bool exclusive = false);
    __inline__ __device__ static T doOp(T a, T b) { return a > b ? a : b; }
};

template<typename T>
struct WaveOpMin
{
    __inline__ __device__ static T getInitial(T a, bool exclusive = false);
    __inline__ __device__ static T doOp(T a, T b) { return a < b ? a : b; }
};

// Compact specializations using macro for getInitial
#define SLANG_WAVE_MIN_SPEC(T, EXCL_VAL)                                  \
    template<>                                                            \
    __inline__ __device__ T WaveOpMin<T>::getInitial(T a, bool exclusive) \
    {                                                                     \
        return exclusive ? (EXCL_VAL) : a;                                \
    }

#define SLANG_WAVE_MAX_SPEC(T, EXCL_VAL)                                  \
    template<>                                                            \
    __inline__ __device__ T WaveOpMax<T>::getInitial(T a, bool exclusive) \
    {                                                                     \
        return exclusive ? (EXCL_VAL) : a;                                \
    }

// Min specializations (exclusive identity = max value)
SLANG_WAVE_MIN_SPEC(float, SLANG_INFINITY)
SLANG_WAVE_MIN_SPEC(double, SLANG_INFINITY)
SLANG_WAVE_MIN_SPEC(int, 0x7FFFFFFF)
SLANG_WAVE_MIN_SPEC(uint, 0xFFFFFFFF)
SLANG_WAVE_MIN_SPEC(char, (char)0x7F)
SLANG_WAVE_MIN_SPEC(int8_t, (int8_t)0x7F)
SLANG_WAVE_MIN_SPEC(uint8_t, (uint8_t)0xFF)
SLANG_WAVE_MIN_SPEC(int16_t, (int16_t)0x7FFF)
SLANG_WAVE_MIN_SPEC(uint16_t, (uint16_t)0xFFFF)
SLANG_WAVE_MIN_SPEC(int64_t, 0x7FFFFFFFFFFFFFFFLL)
SLANG_WAVE_MIN_SPEC(uint64_t, 0xFFFFFFFFFFFFFFFFULL)
#if SLANG_CUDA_ENABLE_HALF
SLANG_WAVE_MIN_SPEC(__half, __ushort_as_half(0x7BFF))
#endif

// Max specializations (exclusive identity = min value)
SLANG_WAVE_MAX_SPEC(float, -SLANG_INFINITY)
SLANG_WAVE_MAX_SPEC(double, -SLANG_INFINITY)
SLANG_WAVE_MAX_SPEC(int, (int)0x80000000)
SLANG_WAVE_MAX_SPEC(uint, 0)
SLANG_WAVE_MAX_SPEC(char, (char)0x80)
SLANG_WAVE_MAX_SPEC(int8_t, (int8_t)0x80)
SLANG_WAVE_MAX_SPEC(uint8_t, 0)
SLANG_WAVE_MAX_SPEC(int16_t, (int16_t)0x8000)
SLANG_WAVE_MAX_SPEC(uint16_t, 0)
SLANG_WAVE_MAX_SPEC(int64_t, (int64_t)0x8000000000000000LL)
SLANG_WAVE_MAX_SPEC(uint64_t, 0)
#if SLANG_CUDA_ENABLE_HALF
SLANG_WAVE_MAX_SPEC(__half, __ushort_as_half(0xFBFF))
#endif

#undef SLANG_WAVE_MIN_SPEC
#undef SLANG_WAVE_MAX_SPEC

template<typename T>
struct ElementTypeTrait;

// Scalar
template<>
struct ElementTypeTrait<int>
{
    typedef int Type;
};
template<>
struct ElementTypeTrait<uint>
{
    typedef uint Type;
};
template<>
struct ElementTypeTrait<float>
{
    typedef float Type;
};
template<>
struct ElementTypeTrait<double>
{
    typedef double Type;
};
template<>
struct ElementTypeTrait<uint64_t>
{
    typedef uint64_t Type;
};
template<>
struct ElementTypeTrait<int64_t>
{
    typedef int64_t Type;
};
template<>
struct ElementTypeTrait<char>
{
    typedef char Type;
};
template<>
struct ElementTypeTrait<uchar>
{
    typedef uchar Type;
};
template<>
struct ElementTypeTrait<short>
{
    typedef short Type;
};
template<>
struct ElementTypeTrait<ushort>
{
    typedef ushort Type;
};
#if SLANG_CUDA_ENABLE_HALF
template<>
struct ElementTypeTrait<__half>
{
    typedef __half Type;
};
#endif

// Vector
template<>
struct ElementTypeTrait<int1>
{
    typedef int Type;
};
template<>
struct ElementTypeTrait<int2>
{
    typedef int Type;
};
template<>
struct ElementTypeTrait<int3>
{
    typedef int Type;
};
template<>
struct ElementTypeTrait<int4>
{
    typedef int Type;
};

template<>
struct ElementTypeTrait<uint1>
{
    typedef uint Type;
};
template<>
struct ElementTypeTrait<uint2>
{
    typedef uint Type;
};
template<>
struct ElementTypeTrait<uint3>
{
    typedef uint Type;
};
template<>
struct ElementTypeTrait<uint4>
{
    typedef uint Type;
};

template<>
struct ElementTypeTrait<float1>
{
    typedef float Type;
};
template<>
struct ElementTypeTrait<float2>
{
    typedef float Type;
};
template<>
struct ElementTypeTrait<float3>
{
    typedef float Type;
};
template<>
struct ElementTypeTrait<float4>
{
    typedef float Type;
};

template<>
struct ElementTypeTrait<double1>
{
    typedef double Type;
};
template<>
struct ElementTypeTrait<double2>
{
    typedef double Type;
};
template<>
struct ElementTypeTrait<double3>
{
    typedef double Type;
};
template<>
struct ElementTypeTrait<double4>
{
    typedef double Type;
};

// Additional vector types
template<>
struct ElementTypeTrait<char2>
{
    typedef char Type;
};
template<>
struct ElementTypeTrait<char3>
{
    typedef char Type;
};
template<>
struct ElementTypeTrait<char4>
{
    typedef char Type;
};
template<>
struct ElementTypeTrait<uchar2>
{
    typedef uchar Type;
};
template<>
struct ElementTypeTrait<uchar3>
{
    typedef uchar Type;
};
template<>
struct ElementTypeTrait<uchar4>
{
    typedef uchar Type;
};
template<>
struct ElementTypeTrait<short2>
{
    typedef short Type;
};
template<>
struct ElementTypeTrait<short3>
{
    typedef short Type;
};
template<>
struct ElementTypeTrait<short4>
{
    typedef short Type;
};
template<>
struct ElementTypeTrait<ushort2>
{
    typedef ushort Type;
};
template<>
struct ElementTypeTrait<ushort3>
{
    typedef ushort Type;
};
template<>
struct ElementTypeTrait<ushort4>
{
    typedef ushort Type;
};
template<>
struct ElementTypeTrait<longlong2>
{
    typedef int64_t Type;
};
template<>
struct ElementTypeTrait<longlong3>
{
    typedef int64_t Type;
};
template<>
struct ElementTypeTrait<longlong4>
{
    typedef int64_t Type;
};
template<>
struct ElementTypeTrait<ulonglong2>
{
    typedef uint64_t Type;
};
template<>
struct ElementTypeTrait<ulonglong3>
{
    typedef uint64_t Type;
};
template<>
struct ElementTypeTrait<ulonglong4>
{
    typedef uint64_t Type;
};
#if SLANG_CUDA_ENABLE_HALF
template<>
struct ElementTypeTrait<__half2>
{
    typedef __half Type;
};
template<>
struct ElementTypeTrait<__half3>
{
    typedef __half Type;
};
template<>
struct ElementTypeTrait<__half4>
{
    typedef __half Type;
};
#endif

// Matrix
template<typename T, int ROWS, int COLS>
struct ElementTypeTrait<Matrix<T, ROWS, COLS>>
{
    typedef T Type;
};

// Scalar
template<typename INTF, typename T>
__device__ T _waveReduceScalar(WarpMask mask, T val)
{
    const int offsetSize = _waveCalcPow2Offset(mask);
    if (offsetSize > 0)
    {
        // Fast path O(log2(activeLanes))
        for (int offset = offsetSize >> 1; offset > 0; offset >>= 1)
        {
            val = INTF::doOp(val, __shfl_xor_sync(mask, val, offset));
        }
    }
    else if (!_waveIsSingleLane(mask))
    {
        T result = INTF::getInitial(val);
        int remaining = mask;
        while (remaining)
        {
            const int laneBit = remaining & -remaining;
            // Get the sourceLane
            const int srcLane = __ffs(laneBit) - 1;
            // Broadcast (can also broadcast to self)
            result = INTF::doOp(result, __shfl_sync(mask, val, srcLane));
            remaining &= ~laneBit;
        }
        return result;
    }
    return val;
}


// Multiple values
template<typename INTF, typename T, size_t COUNT>
__device__ void _waveReduceMultiple(WarpMask mask, T* val)
{
    const int offsetSize = _waveCalcPow2Offset(mask);
    if (offsetSize > 0)
    {
        // Fast path O(log2(activeLanes))
        for (int offset = offsetSize >> 1; offset > 0; offset >>= 1)
        {
            for (size_t i = 0; i < COUNT; ++i)
            {
                val[i] = INTF::doOp(val[i], __shfl_xor_sync(mask, val[i], offset));
            }
        }
    }
    else if (!_waveIsSingleLane(mask))
    {
        // Copy the original
        T originalVal[COUNT];
        for (size_t i = 0; i < COUNT; ++i)
        {
            const T v = val[i];
            originalVal[i] = v;
            val[i] = INTF::getInitial(v);
        }

        int remaining = mask;
        while (remaining)
        {
            const int laneBit = remaining & -remaining;
            // Get the sourceLane
            const int srcLane = __ffs(laneBit) - 1;
            // Broadcast (can also broadcast to self)
            for (size_t i = 0; i < COUNT; ++i)
            {
                val[i] = INTF::doOp(val[i], __shfl_sync(mask, originalVal[i], srcLane));
            }
            remaining &= ~laneBit;
        }
    }
}

template<typename INTF, typename T>
__device__ void _waveReduceMultiple(WarpMask mask, T* val)
{
    typedef typename ElementTypeTrait<T>::Type ElemType;
    _waveReduceMultiple<INTF, ElemType, sizeof(T) / sizeof(ElemType)>(mask, (ElemType*)val);
}

template<typename T>
__inline__ __device__ T _waveOr(WarpMask mask, T val)
{
    return _waveReduceScalar<WaveOpOr<T>, T>(mask, val);
}

template<typename T>
__inline__ __device__ T _waveAnd(WarpMask mask, T val)
{
    return _waveReduceScalar<WaveOpAnd<T>, T>(mask, val);
}

template<typename T>
__inline__ __device__ T _waveXor(WarpMask mask, T val)
{
    return _waveReduceScalar<WaveOpXor<T>, T>(mask, val);
}

template<typename T>
__inline__ __device__ T _waveProduct(WarpMask mask, T val)
{
    return _waveReduceScalar<WaveOpMul<T>, T>(mask, val);
}

template<typename T>
__inline__ __device__ T _waveSum(WarpMask mask, T val)
{
    return _waveReduceScalar<WaveOpAdd<T>, T>(mask, val);
}

template<typename T>
__inline__ __device__ T _waveMin(WarpMask mask, T val)
{
    return _waveReduceScalar<WaveOpMin<T>, T>(mask, val);
}

template<typename T>
__inline__ __device__ T _waveMax(WarpMask mask, T val)
{
    return _waveReduceScalar<WaveOpMax<T>, T>(mask, val);
}

// Fast-path specializations when CUDA warp reduce operators are available
#if __CUDA_ARCH__ >= 800 // 8.x or higher
template<>
__inline__ __device__ unsigned _waveOr<unsigned>(WarpMask mask, unsigned val)
{
    return __reduce_or_sync(mask, val);
}

template<>
__inline__ __device__ unsigned _waveAnd<unsigned>(WarpMask mask, unsigned val)
{
    return __reduce_and_sync(mask, val);
}

template<>
__inline__ __device__ unsigned _waveXor<unsigned>(WarpMask mask, unsigned val)
{
    return __reduce_xor_sync(mask, val);
}

template<>
__inline__ __device__ unsigned _waveSum<unsigned>(WarpMask mask, unsigned val)
{
    return __reduce_add_sync(mask, val);
}

template<>
__inline__ __device__ int _waveSum<int>(WarpMask mask, int val)
{
    return __reduce_add_sync(mask, val);
}

template<>
__inline__ __device__ unsigned _waveMin<unsigned>(WarpMask mask, unsigned val)
{
    return __reduce_min_sync(mask, val);
}

template<>
__inline__ __device__ int _waveMin<int>(WarpMask mask, int val)
{
    return __reduce_min_sync(mask, val);
}

template<>
__inline__ __device__ unsigned _waveMax<unsigned>(WarpMask mask, unsigned val)
{
    return __reduce_max_sync(mask, val);
}

template<>
__inline__ __device__ int _waveMax<int>(WarpMask mask, int val)
{
    return __reduce_max_sync(mask, val);
}
#endif

// Multiple

template<typename T>
__inline__ __device__ T _waveOrMultiple(WarpMask mask, T val)
{
    typedef typename ElementTypeTrait<T>::Type ElemType;
    _waveReduceMultiple<WaveOpOr<ElemType>>(mask, &val);
    return val;
}

template<typename T>
__inline__ __device__ T _waveAndMultiple(WarpMask mask, T val)
{
    typedef typename ElementTypeTrait<T>::Type ElemType;
    _waveReduceMultiple<WaveOpAnd<ElemType>>(mask, &val);
    return val;
}

template<typename T>
__inline__ __device__ T _waveXorMultiple(WarpMask mask, T val)
{
    typedef typename ElementTypeTrait<T>::Type ElemType;
    _waveReduceMultiple<WaveOpXor<ElemType>>(mask, &val);
    return val;
}

template<typename T>
__inline__ __device__ T _waveProductMultiple(WarpMask mask, T val)
{
    typedef typename ElementTypeTrait<T>::Type ElemType;
    _waveReduceMultiple<WaveOpMul<ElemType>>(mask, &val);
    return val;
}

template<typename T>
__inline__ __device__ T _waveSumMultiple(WarpMask mask, T val)
{
    typedef typename ElementTypeTrait<T>::Type ElemType;
    _waveReduceMultiple<WaveOpAdd<ElemType>>(mask, &val);
    return val;
}

template<typename T>
__inline__ __device__ T _waveMinMultiple(WarpMask mask, T val)
{
    typedef typename ElementTypeTrait<T>::Type ElemType;
    _waveReduceMultiple<WaveOpMin<ElemType>>(mask, &val);
    return val;
}

template<typename T>
__inline__ __device__ T _waveMaxMultiple(WarpMask mask, T val)
{
    typedef typename ElementTypeTrait<T>::Type ElemType;
    _waveReduceMultiple<WaveOpMax<ElemType>>(mask, &val);
    return val;
}


template<typename T>
__inline__ __device__ bool _waveAllEqual(WarpMask mask, T val)
{
    int pred;
    __match_all_sync(mask, val, &pred);
    return pred != 0;
}

template<typename T>
__inline__ __device__ bool _waveAllEqualMultiple(WarpMask mask, T inVal)
{
    typedef typename ElementTypeTrait<T>::Type ElemType;
    const size_t count = sizeof(T) / sizeof(ElemType);
    int pred;
    const ElemType* src = (const ElemType*)&inVal;
    for (size_t i = 0; i < count; ++i)
    {
        __match_all_sync(mask, src[i], &pred);
        if (pred == 0)
        {
            return false;
        }
    }
    return true;
}

template<typename T>
__inline__ __device__ T _waveReadFirst(WarpMask mask, T val)
{
    const int lowestLaneId = __ffs(mask) - 1;
    return __shfl_sync(mask, val, lowestLaneId);
}

template<typename T>
__inline__ __device__ T _waveReadFirstMultiple(WarpMask mask, T inVal)
{
    typedef typename ElementTypeTrait<T>::Type ElemType;
    const size_t count = sizeof(T) / sizeof(ElemType);
    T outVal;
    const ElemType* src = (const ElemType*)&inVal;
    ElemType* dst = (ElemType*)&outVal;
    const int lowestLaneId = __ffs(mask) - 1;
    for (size_t i = 0; i < count; ++i)
    {
        dst[i] = __shfl_sync(mask, src[i], lowestLaneId);
    }
    return outVal;
}

template<typename T>
__inline__ __device__ T _waveShuffleMultiple(WarpMask mask, T inVal, int lane)
{
    typedef typename ElementTypeTrait<T>::Type ElemType;
    const size_t count = sizeof(T) / sizeof(ElemType);
    T outVal;
    const ElemType* src = (const ElemType*)&inVal;
    ElemType* dst = (ElemType*)&outVal;
    for (size_t i = 0; i < count; ++i)
    {
        dst[i] = __shfl_sync(mask, src[i], lane);
    }
    return outVal;
}

// Scalar

// Invertable means that when we get to the end of the reduce, we can remove val (to make
// exclusive), using the inverse of the op.
template<typename INTF, typename T>
__device__ T _wavePrefixInvertableScalar(WarpMask mask, T val)
{
    const int offsetSize = _waveCalcPow2Offset(mask);

    const int laneId = _getLaneId();
    T result;
    if (offsetSize > 0)
    {
        // Sum is calculated inclusive of this lanes value
        result = val;
        for (int i = 1; i < offsetSize; i += i)
        {
            const T readVal = __shfl_up_sync(mask, result, i, offsetSize);
            if (laneId >= i)
            {
                result = INTF::doOp(result, readVal);
            }
        }
        // Remove val from the result, by applyin inverse
        result = INTF::doInverse(result, val);
    }
    else
    {
        result = INTF::getInitial(val);
        if (!_waveIsSingleLane(mask))
        {
            int remaining = mask;
            while (remaining)
            {
                const int laneBit = remaining & -remaining;
                // Get the sourceLane
                const int srcLane = __ffs(laneBit) - 1;
                // Broadcast (can also broadcast to self)
                const T readValue = __shfl_sync(mask, val, srcLane);
                // Only accumulate if srcLane is less than this lane
                if (srcLane < laneId)
                {
                    result = INTF::doOp(result, readValue);
                }
                remaining &= ~laneBit;
            }
        }
    }
    return result;
}


// This implementation separately tracks the value to be propogated, and the value
// that is the final result
template<typename INTF, typename T>
__device__ T _wavePrefixScalar(WarpMask mask, T val)
{
    const int offsetSize = _waveCalcPow2Offset(mask);

    const int laneId = _getLaneId();
    T result = INTF::getInitial(val);
    if (offsetSize > 0)
    {
        // For transmitted value we will do it inclusively with this lanes value
        // For the result we do not include the lanes value. This means an extra multiply for each
        // iteration but means we don't need to have a divide at the end and also removes overflow
        // issues in that scenario.
        for (int i = 1; i < offsetSize; i += i)
        {
            const T readVal = __shfl_up_sync(mask, val, i, offsetSize);
            if (laneId >= i)
            {
                result = INTF::doOp(result, readVal);
                val = INTF::doOp(val, readVal);
            }
        }
    }
    else
    {
        if (!_waveIsSingleLane(mask))
        {
            int remaining = mask;
            while (remaining)
            {
                const int laneBit = remaining & -remaining;
                // Get the sourceLane
                const int srcLane = __ffs(laneBit) - 1;
                // Broadcast (can also broadcast to self)
                const T readValue = __shfl_sync(mask, val, srcLane);
                // Only accumulate if srcLane is less than this lane
                if (srcLane < laneId)
                {
                    result = INTF::doOp(result, readValue);
                }
                remaining &= ~laneBit;
            }
        }
    }
    return result;
}


template<typename INTF, typename T, size_t COUNT>
__device__ T _waveOpCopy(T* dst, const T* src)
{
    for (size_t j = 0; j < COUNT; ++j)
    {
        dst[j] = src[j];
    }
}


template<typename INTF, typename T, size_t COUNT>
__device__ T _waveOpDoInverse(T* inOut, const T* val)
{
    for (size_t j = 0; j < COUNT; ++j)
    {
        inOut[j] = INTF::doInverse(inOut[j], val[j]);
    }
}

template<typename INTF, typename T, size_t COUNT>
__device__ T _waveOpSetInitial(T* out, const T* val)
{
    for (size_t j = 0; j < COUNT; ++j)
    {
        out[j] = INTF::getInitial(val[j]);
    }
}

template<typename INTF, typename T, size_t COUNT>
__device__ T _wavePrefixInvertableMultiple(WarpMask mask, T* val)
{
    const int offsetSize = _waveCalcPow2Offset(mask);

    const int laneId = _getLaneId();
    T originalVal[COUNT];
    _waveOpCopy<INTF, T, COUNT>(originalVal, val);

    if (offsetSize > 0)
    {
        // Sum is calculated inclusive of this lanes value
        for (int i = 1; i < offsetSize; i += i)
        {
            // TODO(JS): Note that here I don't split the laneId outside so it's only tested once.
            // This may be better but it would also mean that there would be shfl between lanes
            // that are on different (albeit identical) instructions. So this seems more likely to
            // work as expected with everything in lock step.
            for (size_t j = 0; j < COUNT; ++j)
            {
                const T readVal = __shfl_up_sync(mask, val[j], i, offsetSize);
                if (laneId >= i)
                {
                    val[j] = INTF::doOp(val[j], readVal);
                }
            }
        }
        // Remove originalVal from the result, by applyin inverse
        _waveOpDoInverse<INTF, T, COUNT>(val, originalVal);
    }
    else
    {
        _waveOpSetInitial<INTF, T, COUNT>(val, val);
        if (!_waveIsSingleLane(mask))
        {
            int remaining = mask;
            while (remaining)
            {
                const int laneBit = remaining & -remaining;
                // Get the sourceLane
                const int srcLane = __ffs(laneBit) - 1;

                for (size_t j = 0; j < COUNT; ++j)
                {
                    // Broadcast (can also broadcast to self)
                    const T readValue = __shfl_sync(mask, originalVal[j], srcLane);
                    // Only accumulate if srcLane is less than this lane
                    if (srcLane < laneId)
                    {
                        val[j] = INTF::doOp(val[j], readValue);
                    }
                    remaining &= ~laneBit;
                }
            }
        }
    }
}

template<typename INTF, typename T, size_t COUNT>
__device__ T _wavePrefixMultiple(WarpMask mask, T* val)
{
    const int offsetSize = _waveCalcPow2Offset(mask);

    const int laneId = _getLaneId();

    T work[COUNT];
    _waveOpCopy<INTF, T, COUNT>(work, val);
    _waveOpSetInitial<INTF, T, COUNT>(val, val);

    if (offsetSize > 0)
    {
        // For transmitted value we will do it inclusively with this lanes value
        // For the result we do not include the lanes value. This means an extra op for each
        // iteration but means we don't need to have a divide at the end and also removes overflow
        // issues in that scenario.
        for (int i = 1; i < offsetSize; i += i)
        {
            for (size_t j = 0; j < COUNT; ++j)
            {
                const T readVal = __shfl_up_sync(mask, work[j], i, offsetSize);
                if (laneId >= i)
                {
                    work[j] = INTF::doOp(work[j], readVal);
                    val[j] = INTF::doOp(val[j], readVal);
                }
            }
        }
    }
    else
    {
        if (!_waveIsSingleLane(mask))
        {
            int remaining = mask;
            while (remaining)
            {
                const int laneBit = remaining & -remaining;
                // Get the sourceLane
                const int srcLane = __ffs(laneBit) - 1;

                for (size_t j = 0; j < COUNT; ++j)
                {
                    // Broadcast (can also broadcast to self)
                    const T readValue = __shfl_sync(mask, work[j], srcLane);
                    // Only accumulate if srcLane is less than this lane
                    if (srcLane < laneId)
                    {
                        val[j] = INTF::doOp(val[j], readValue);
                    }
                }
                remaining &= ~laneBit;
            }
        }
    }
}

template<typename T>
__inline__ __device__ T _wavePrefixProduct(WarpMask mask, T val)
{
    return _wavePrefixScalar<WaveOpMul<T>, T>(mask, val);
}

template<typename T>
__inline__ __device__ T _wavePrefixSum(WarpMask mask, T val)
{
    return _wavePrefixInvertableScalar<WaveOpAdd<T>, T>(mask, val);
}

template<typename T>
__inline__ __device__ T _wavePrefixXor(WarpMask mask, T val)
{
    return _wavePrefixInvertableScalar<WaveOpXor<T>, T>(mask, val);
}

template<typename T>
__inline__ __device__ T _wavePrefixOr(WarpMask mask, T val)
{
    return _wavePrefixScalar<WaveOpOr<T>, T>(mask, val);
}

template<typename T>
__inline__ __device__ T _wavePrefixAnd(WarpMask mask, T val)
{
    return _wavePrefixScalar<WaveOpAnd<T>, T>(mask, val);
}


template<typename T>
__inline__ __device__ T _wavePrefixProductMultiple(WarpMask mask, T val)
{
    typedef typename ElementTypeTrait<T>::Type ElemType;
    _wavePrefixInvertableMultiple<WaveOpMul<ElemType>, ElemType, sizeof(T) / sizeof(ElemType)>(
        mask,
        (ElemType*)&val);
    return val;
}

template<typename T>
__inline__ __device__ T _wavePrefixSumMultiple(WarpMask mask, T val)
{
    typedef typename ElementTypeTrait<T>::Type ElemType;
    _wavePrefixInvertableMultiple<WaveOpAdd<ElemType>, ElemType, sizeof(T) / sizeof(ElemType)>(
        mask,
        (ElemType*)&val);
    return val;
}

template<typename T>
__inline__ __device__ T _wavePrefixXorMultiple(WarpMask mask, T val)
{
    typedef typename ElementTypeTrait<T>::Type ElemType;
    _wavePrefixInvertableMultiple<WaveOpXor<ElemType>, ElemType, sizeof(T) / sizeof(ElemType)>(
        mask,
        (ElemType*)&val);
    return val;
}

template<typename T>
__inline__ __device__ T _wavePrefixOrMultiple(WarpMask mask, T val)
{
    typedef typename ElementTypeTrait<T>::Type ElemType;
    _wavePrefixMultiple<WaveOpOr<ElemType>, ElemType, sizeof(T) / sizeof(ElemType)>(
        mask,
        (ElemType*)&val);
    return val;
}

template<typename T>
__inline__ __device__ T _wavePrefixAndMultiple(WarpMask mask, T val)
{
    typedef typename ElementTypeTrait<T>::Type ElemType;
    _wavePrefixMultiple<WaveOpAnd<ElemType>, ElemType, sizeof(T) / sizeof(ElemType)>(
        mask,
        (ElemType*)&val);
    return val;
}

template<typename T>
__inline__ __device__ T _wavePrefixMin(WarpMask mask, T val)
{
    return _wavePrefixScalar<WaveOpMin<T>, T>(mask, val);
}

template<typename T>
__inline__ __device__ T _wavePrefixMax(WarpMask mask, T val)
{
    return _wavePrefixScalar<WaveOpMax<T>, T>(mask, val);
}

template<typename T>
__inline__ __device__ T _wavePrefixMinMultiple(WarpMask mask, T val)
{
    typedef typename ElementTypeTrait<T>::Type ElemType;
    _wavePrefixMultiple<WaveOpMin<ElemType>, ElemType, sizeof(T) / sizeof(ElemType)>(
        mask,
        (ElemType*)&val);
    return val;
}

template<typename T>
__inline__ __device__ T _wavePrefixMaxMultiple(WarpMask mask, T val)
{
    typedef typename ElementTypeTrait<T>::Type ElemType;
    _wavePrefixMultiple<WaveOpMax<ElemType>, ElemType, sizeof(T) / sizeof(ElemType)>(
        mask,
        (ElemType*)&val);
    return val;
}

// Wrapper structures for exclusive operations that use the overloaded getInitial method
template<typename T>
struct WaveOpExclusiveMin
{
    __inline__ __device__ static T getInitial(T a) { return WaveOpMin<T>::getInitial(a, true); }
    __inline__ __device__ static T doOp(T a, T b) { return WaveOpMin<T>::doOp(a, b); }
};

template<typename T>
struct WaveOpExclusiveMax
{
    __inline__ __device__ static T getInitial(T a) { return WaveOpMax<T>::getInitial(a, true); }
    __inline__ __device__ static T doOp(T a, T b) { return WaveOpMax<T>::doOp(a, b); }
};

// Inclusive prefix min/max functions (for WaveMultiPrefixInclusive*)
template<typename T>
__inline__ __device__ T _wavePrefixInclusiveMin(WarpMask mask, T val)
{
    return _wavePrefixMin(mask, val);
}

template<typename T>
__inline__ __device__ T _wavePrefixInclusiveMax(WarpMask mask, T val)
{
    return _wavePrefixMax(mask, val);
}

template<typename T>
__inline__ __device__ T _wavePrefixInclusiveMinMultiple(WarpMask mask, T val)
{
    return _wavePrefixMinMultiple(mask, val);
}

template<typename T>
__inline__ __device__ T _wavePrefixInclusiveMaxMultiple(WarpMask mask, T val)
{
    return _wavePrefixMaxMultiple(mask, val);
}

// Explicit exclusive prefix min/max functions (for WaveMultiPrefixExclusive*)
template<typename T>
__inline__ __device__ T _wavePrefixExclusiveMin(WarpMask mask, T val)
{
    return _wavePrefixScalar<WaveOpExclusiveMin<T>, T>(mask, val);
}

template<typename T>
__inline__ __device__ T _wavePrefixExclusiveMax(WarpMask mask, T val)
{
    return _wavePrefixScalar<WaveOpExclusiveMax<T>, T>(mask, val);
}

template<typename T>
__inline__ __device__ T _wavePrefixExclusiveMinMultiple(WarpMask mask, T val)
{
    typedef typename ElementTypeTrait<T>::Type ElemType;
    _wavePrefixMultiple<WaveOpExclusiveMin<ElemType>, ElemType, sizeof(T) / sizeof(ElemType)>(
        mask,
        (ElemType*)&val);
    return val;
}

template<typename T>
__inline__ __device__ T _wavePrefixExclusiveMaxMultiple(WarpMask mask, T val)
{
    typedef typename ElementTypeTrait<T>::Type ElemType;
    _wavePrefixMultiple<WaveOpExclusiveMax<ElemType>, ElemType, sizeof(T) / sizeof(ElemType)>(
        mask,
        (ElemType*)&val);
    return val;
}

template<typename T>
__inline__ __device__ uint4 _waveMatchScalar(WarpMask mask, T val)
{
    int pred;
    return make_uint4(__match_all_sync(mask, val, &pred), 0, 0, 0);
}

template<typename T>
__inline__ __device__ uint4 _waveMatchMultiple(WarpMask mask, const T& inVal)
{
    typedef typename ElementTypeTrait<T>::Type ElemType;
    const size_t count = sizeof(T) / sizeof(ElemType);
    int pred;
    const ElemType* src = (const ElemType*)&inVal;
    uint matchBits = 0xffffffff;
    for (size_t i = 0; i < count && matchBits; ++i)
    {
        matchBits = matchBits & __match_all_sync(mask, src[i], &pred);
    }
    return make_uint4(matchBits, 0, 0, 0);
}

__inline__ __device__ uint getAt(dim3 a, int b)
{
    SLANG_PRELUDE_ASSERT(b >= 0 && b < 3);
    return (&a.x)[b];
}
__inline__ __device__ uint3 operator*(uint3 a, dim3 b)
{
    uint3 r;
    r.x = a.x * b.x;
    r.y = a.y * b.y;
    r.z = a.z * b.z;
    return r;
}

template<typename TResult, typename TInput>
__inline__ __device__ TResult slang_bit_cast(TInput val)
{
    return *(TResult*)(&val);
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */


/* Type that defines the uniform entry point params. The actual content of this type is dependent on
the entry point parameters, and can be found via reflection or defined such that it matches the
shader appropriately.
*/
struct UniformEntryPointParams;
struct UniformState;

// ---------------------- OptiX Ray Payload --------------------------------------
#ifdef SLANG_CUDA_ENABLE_OPTIX

struct RayDesc
{
    float3 Origin;
    float TMin;
    float3 Direction;
    float TMax;
};

static __forceinline__ __device__ void* unpackOptiXRayPayloadPointer(uint32_t i0, uint32_t i1)
{
    const uint64_t uptr = static_cast<uint64_t>(i0) << 32 | i1;
    void* ptr = reinterpret_cast<void*>(uptr);
    return ptr;
}

static __forceinline__ __device__ void packOptiXRayPayloadPointer(
    void* ptr,
    uint32_t& i0,
    uint32_t& i1)
{
    const uint64_t uptr = reinterpret_cast<uint64_t>(ptr);
    i0 = uptr >> 32;
    i1 = uptr & 0x00000000ffffffff;
}

static __forceinline__ __device__ void* getOptiXRayPayloadPtr()
{
    const uint32_t u0 = optixGetPayload_0();
    const uint32_t u1 = optixGetPayload_1();
    return unpackOptiXRayPayloadPointer(u0, u1);
}

// Maximum number of 32-bit registers for OptiX payload (32 registers = 128 bytes)
static constexpr size_t kMaxOptiXPayloadRegisters = 32;

// Helper to pack/unpack payload to/from registers for small payloads (<= 128 bytes)
template<typename T, size_t N = (sizeof(T) + 3) / 4>
struct PayloadRegisters
{
    uint32_t regs[N > 0 ? N : 1];

    __forceinline__ __device__ void pack(const T& payload) { memcpy(regs, &payload, sizeof(T)); }

    __forceinline__ __device__ void unpack(T& payload) { memcpy(&payload, regs, sizeof(T)); }
};

// Internal helper to call optixTrace with the right number of register arguments
template<typename T, size_t N = (sizeof(T) + 3) / 4>
__forceinline__ __device__ void optixTraceWithRegs(
    OptixTraversableHandle AccelerationStructure,
    float3 Origin,
    float3 Direction,
    float TMin,
    float TMax,
    float Time,
    uint32_t InstanceInclusionMask,
    uint32_t RayFlags,
    uint32_t RayContributionToHitGroupIndex,
    uint32_t MultiplierForGeometryContributionToHitGroupIndex,
    uint32_t MissShaderIndex,
    PayloadRegisters<T, N>& pr)
{
    // Call optixTrace with the appropriate number of payload registers
    if constexpr (N == 0)
    {
        optixTrace(
            AccelerationStructure,
            Origin,
            Direction,
            TMin,
            TMax,
            Time,
            InstanceInclusionMask,
            RayFlags,
            RayContributionToHitGroupIndex,
            MultiplierForGeometryContributionToHitGroupIndex,
            MissShaderIndex);
    }
    else if constexpr (N == 1)
    {
        optixTrace(
            AccelerationStructure,
            Origin,
            Direction,
            TMin,
            TMax,
            Time,
            InstanceInclusionMask,
            RayFlags,
            RayContributionToHitGroupIndex,
            MultiplierForGeometryContributionToHitGroupIndex,
            MissShaderIndex,
            pr.regs[0]);
    }
    else if constexpr (N == 2)
    {
        optixTrace(
            AccelerationStructure,
            Origin,
            Direction,
            TMin,
            TMax,
            Time,
            InstanceInclusionMask,
            RayFlags,
            RayContributionToHitGroupIndex,
            MultiplierForGeometryContributionToHitGroupIndex,
            MissShaderIndex,
            pr.regs[0],
            pr.regs[1]);
    }
    else if constexpr (N == 3)
    {
        optixTrace(
            AccelerationStructure,
            Origin,
            Direction,
            TMin,
            TMax,
            Time,
            InstanceInclusionMask,
            RayFlags,
            RayContributionToHitGroupIndex,
            MultiplierForGeometryContributionToHitGroupIndex,
            MissShaderIndex,
            pr.regs[0],
            pr.regs[1],
            pr.regs[2]);
    }
    else if constexpr (N == 4)
    {
        optixTrace(
            AccelerationStructure,
            Origin,
            Direction,
            TMin,
            TMax,
            Time,
            InstanceInclusionMask,
            RayFlags,
            RayContributionToHitGroupIndex,
            MultiplierForGeometryContributionToHitGroupIndex,
            MissShaderIndex,
            pr.regs[0],
            pr.regs[1],
            pr.regs[2],
            pr.regs[3]);
    }
    else if constexpr (N == 5)
    {
        optixTrace(
            AccelerationStructure,
            Origin,
            Direction,
            TMin,
            TMax,
            Time,
            InstanceInclusionMask,
            RayFlags,
            RayContributionToHitGroupIndex,
            MultiplierForGeometryContributionToHitGroupIndex,
            MissShaderIndex,
            pr.regs[0],
            pr.regs[1],
            pr.regs[2],
            pr.regs[3],
            pr.regs[4]);
    }
    else if constexpr (N == 6)
    {
        optixTrace(
            AccelerationStructure,
            Origin,
            Direction,
            TMin,
            TMax,
            Time,
            InstanceInclusionMask,
            RayFlags,
            RayContributionToHitGroupIndex,
            MultiplierForGeometryContributionToHitGroupIndex,
            MissShaderIndex,
            pr.regs[0],
            pr.regs[1],
            pr.regs[2],
            pr.regs[3],
            pr.regs[4],
            pr.regs[5]);
    }
    else if constexpr (N == 7)
    {
        optixTrace(
            AccelerationStructure,
            Origin,
            Direction,
            TMin,
            TMax,
            Time,
            InstanceInclusionMask,
            RayFlags,
            RayContributionToHitGroupIndex,
            MultiplierForGeometryContributionToHitGroupIndex,
            MissShaderIndex,
            pr.regs[0],
            pr.regs[1],
            pr.regs[2],
            pr.regs[3],
            pr.regs[4],
            pr.regs[5],
            pr.regs[6]);
    }
    else if constexpr (N == 8)
    {
        optixTrace(
            AccelerationStructure,
            Origin,
            Direction,
            TMin,
            TMax,
            Time,
            InstanceInclusionMask,
            RayFlags,
            RayContributionToHitGroupIndex,
            MultiplierForGeometryContributionToHitGroupIndex,
            MissShaderIndex,
            pr.regs[0],
            pr.regs[1],
            pr.regs[2],
            pr.regs[3],
            pr.regs[4],
            pr.regs[5],
            pr.regs[6],
            pr.regs[7]);
    }
    else if constexpr (N <= 16)
    {
        optixTrace(
            AccelerationStructure,
            Origin,
            Direction,
            TMin,
            TMax,
            Time,
            InstanceInclusionMask,
            RayFlags,
            RayContributionToHitGroupIndex,
            MultiplierForGeometryContributionToHitGroupIndex,
            MissShaderIndex,
            pr.regs[0],
            pr.regs[1],
            pr.regs[2],
            pr.regs[3],
            pr.regs[4],
            pr.regs[5],
            pr.regs[6],
            pr.regs[7],
            pr.regs[8],
            pr.regs[9],
            pr.regs[10],
            pr.regs[11],
            pr.regs[12],
            pr.regs[13],
            pr.regs[14],
            pr.regs[15]);
    }
    else if constexpr (N <= kMaxOptiXPayloadRegisters)
    {
        optixTrace(
            AccelerationStructure,
            Origin,
            Direction,
            TMin,
            TMax,
            Time,
            InstanceInclusionMask,
            RayFlags,
            RayContributionToHitGroupIndex,
            MultiplierForGeometryContributionToHitGroupIndex,
            MissShaderIndex,
            pr.regs[0],
            pr.regs[1],
            pr.regs[2],
            pr.regs[3],
            pr.regs[4],
            pr.regs[5],
            pr.regs[6],
            pr.regs[7],
            pr.regs[8],
            pr.regs[9],
            pr.regs[10],
            pr.regs[11],
            pr.regs[12],
            pr.regs[13],
            pr.regs[14],
            pr.regs[15],
            pr.regs[16],
            pr.regs[17],
            pr.regs[18],
            pr.regs[19],
            pr.regs[20],
            pr.regs[21],
            pr.regs[22],
            pr.regs[23],
            pr.regs[24],
            pr.regs[25],
            pr.regs[26],
            pr.regs[27],
            pr.regs[28],
            pr.regs[29],
            pr.regs[30],
            pr.regs[31]);
    }
}

template<typename T>
__forceinline__ __device__ void optixTrace(
    OptixTraversableHandle AccelerationStructure,
    uint32_t RayFlags,
    uint32_t InstanceInclusionMask,
    uint32_t RayContributionToHitGroupIndex,
    uint32_t MultiplierForGeometryContributionToHitGroupIndex,
    uint32_t MissShaderIndex,
    RayDesc Ray,
    T* Payload)
{
    constexpr size_t numRegs = (sizeof(T) + 3) / 4;

    if constexpr (numRegs <= kMaxOptiXPayloadRegisters)
    {
        // Register-based approach for small payloads
        PayloadRegisters<T> pr;
        pr.pack(*Payload);

        optixTraceWithRegs<T>(
            AccelerationStructure,
            Ray.Origin,
            Ray.Direction,
            Ray.TMin,
            Ray.TMax,
            0.f, /* Time for motion blur */
            InstanceInclusionMask,
            RayFlags,
            RayContributionToHitGroupIndex,
            MultiplierForGeometryContributionToHitGroupIndex,
            MissShaderIndex,
            pr);

        // Read back updated payload registers
        // Native optixTrace updates regs in place
        pr.unpack(*Payload);
    }
    else
    {
        // Pointer-based fallback for large payloads
        uint32_t r0, r1;
        packOptiXRayPayloadPointer((void*)Payload, r0, r1);
        optixTrace(
            AccelerationStructure,
            Ray.Origin,
            Ray.Direction,
            Ray.TMin,
            Ray.TMax,
            0.f,
            InstanceInclusionMask,
            RayFlags,
            RayContributionToHitGroupIndex,
            MultiplierForGeometryContributionToHitGroupIndex,
            MissShaderIndex,
            r0,
            r1);
    }
}

// Non-template overload for empty payload case.
// When Slang's type legalization eliminates an empty payload struct,
// the generated code calls optixTrace without a payload argument.
__forceinline__ __device__ void optixTrace(
    OptixTraversableHandle AccelerationStructure,
    uint32_t RayFlags,
    uint32_t InstanceInclusionMask,
    uint32_t RayContributionToHitGroupIndex,
    uint32_t MultiplierForGeometryContributionToHitGroupIndex,
    uint32_t MissShaderIndex,
    RayDesc Ray)
{
    optixTrace(
        AccelerationStructure,
        Ray.Origin,
        Ray.Direction,
        Ray.TMin,
        Ray.TMax,
        0.f,
        InstanceInclusionMask,
        RayFlags,
        RayContributionToHitGroupIndex,
        MultiplierForGeometryContributionToHitGroupIndex,
        MissShaderIndex);
}

#if (OPTIX_VERSION >= 90000)
__forceinline__ __device__ float4 optixGetSpherePositionAndRadius()
{
    float4 data[1];
    optixGetSphereData(data);
    return data[0];
}
#endif

#if (OPTIX_VERSION >= 90000)
__forceinline__ __device__ float4
optixHitObjectGetSpherePositionAndRadius(OptixTraversableHandle* Obj)
{
    float4 data[1];
    optixHitObjectGetSphereData(data);
    return data[0];
}
#endif

#if (OPTIX_VERSION >= 90000)
__forceinline__ __device__ Matrix<float, 2, 4> optixGetLssPositionsAndRadii()
{
    float4 data[2];
    optixGetLinearCurveVertexData(data);
    return makeMatrix<float, 2, 4>(data[0], data[1]);
}
#endif

#if (OPTIX_VERSION >= 90000)
__forceinline__ __device__ Matrix<float, 2, 4> optixHitObjectGetLssPositionsAndRadii(
    OptixTraversableHandle* Obj)
{
    float4 data[2];
    optixHitObjectGetLinearCurveVertexData(data);
    return makeMatrix<float, 2, 4>(data[0], data[1]);
}
#endif

#if (OPTIX_VERSION >= 90000)
__forceinline__ __device__ bool optixIsSphereHit()
{
    return optixGetPrimitiveType() == OPTIX_PRIMITIVE_TYPE_SPHERE;
}
#endif

#if (OPTIX_VERSION >= 90000)
__forceinline__ __device__ bool optixHitObjectIsSphereHit(OptixTraversableHandle* Obj)
{
    return optixGetPrimitiveType(optixHitObjectGetHitKind()) == OPTIX_PRIMITIVE_TYPE_SPHERE;
}
#endif

#if (OPTIX_VERSION >= 90000)
__forceinline__ __device__ bool optixIsLSSHit()
{
    return optixGetPrimitiveType() == OPTIX_PRIMITIVE_TYPE_ROUND_LINEAR;
}
#endif

#if (OPTIX_VERSION >= 90000)
__forceinline__ __device__ bool optixHitObjectIsLSSHit(OptixTraversableHandle* Obj)
{
    return optixGetPrimitiveType(optixHitObjectGetHitKind()) == OPTIX_PRIMITIVE_TYPE_ROUND_LINEAR;
}
#endif

// Internal helper to call optixTraverse with the right number of register arguments
template<typename T, size_t N = (sizeof(T) + 3) / 4>
__forceinline__ __device__ void optixTraverseWithRegs(
    OptixTraversableHandle AccelerationStructure,
    float3 Origin,
    float3 Direction,
    float TMin,
    float TMax,
    float Time,
    uint32_t InstanceInclusionMask,
    uint32_t RayFlags,
    uint32_t RayContributionToHitGroupIndex,
    uint32_t MultiplierForGeometryContributionToHitGroupIndex,
    uint32_t MissShaderIndex,
    PayloadRegisters<T, N>& pr)
{
    // Call optixTraverse with the appropriate number of payload registers
    if constexpr (N == 0)
    {
        optixTraverse(
            AccelerationStructure,
            Origin,
            Direction,
            TMin,
            TMax,
            Time,
            InstanceInclusionMask,
            RayFlags,
            RayContributionToHitGroupIndex,
            MultiplierForGeometryContributionToHitGroupIndex,
            MissShaderIndex);
    }
    else if constexpr (N == 1)
    {
        optixTraverse(
            AccelerationStructure,
            Origin,
            Direction,
            TMin,
            TMax,
            Time,
            InstanceInclusionMask,
            RayFlags,
            RayContributionToHitGroupIndex,
            MultiplierForGeometryContributionToHitGroupIndex,
            MissShaderIndex,
            pr.regs[0]);
    }
    else if constexpr (N == 2)
    {
        optixTraverse(
            AccelerationStructure,
            Origin,
            Direction,
            TMin,
            TMax,
            Time,
            InstanceInclusionMask,
            RayFlags,
            RayContributionToHitGroupIndex,
            MultiplierForGeometryContributionToHitGroupIndex,
            MissShaderIndex,
            pr.regs[0],
            pr.regs[1]);
    }
    else if constexpr (N == 3)
    {
        optixTraverse(
            AccelerationStructure,
            Origin,
            Direction,
            TMin,
            TMax,
            Time,
            InstanceInclusionMask,
            RayFlags,
            RayContributionToHitGroupIndex,
            MultiplierForGeometryContributionToHitGroupIndex,
            MissShaderIndex,
            pr.regs[0],
            pr.regs[1],
            pr.regs[2]);
    }
    else if constexpr (N == 4)
    {
        optixTraverse(
            AccelerationStructure,
            Origin,
            Direction,
            TMin,
            TMax,
            Time,
            InstanceInclusionMask,
            RayFlags,
            RayContributionToHitGroupIndex,
            MultiplierForGeometryContributionToHitGroupIndex,
            MissShaderIndex,
            pr.regs[0],
            pr.regs[1],
            pr.regs[2],
            pr.regs[3]);
    }
    else if constexpr (N == 5)
    {
        optixTraverse(
            AccelerationStructure,
            Origin,
            Direction,
            TMin,
            TMax,
            Time,
            InstanceInclusionMask,
            RayFlags,
            RayContributionToHitGroupIndex,
            MultiplierForGeometryContributionToHitGroupIndex,
            MissShaderIndex,
            pr.regs[0],
            pr.regs[1],
            pr.regs[2],
            pr.regs[3],
            pr.regs[4]);
    }
    else if constexpr (N == 6)
    {
        optixTraverse(
            AccelerationStructure,
            Origin,
            Direction,
            TMin,
            TMax,
            Time,
            InstanceInclusionMask,
            RayFlags,
            RayContributionToHitGroupIndex,
            MultiplierForGeometryContributionToHitGroupIndex,
            MissShaderIndex,
            pr.regs[0],
            pr.regs[1],
            pr.regs[2],
            pr.regs[3],
            pr.regs[4],
            pr.regs[5]);
    }
    else if constexpr (N == 7)
    {
        optixTraverse(
            AccelerationStructure,
            Origin,
            Direction,
            TMin,
            TMax,
            Time,
            InstanceInclusionMask,
            RayFlags,
            RayContributionToHitGroupIndex,
            MultiplierForGeometryContributionToHitGroupIndex,
            MissShaderIndex,
            pr.regs[0],
            pr.regs[1],
            pr.regs[2],
            pr.regs[3],
            pr.regs[4],
            pr.regs[5],
            pr.regs[6]);
    }
    else if constexpr (N == 8)
    {
        optixTraverse(
            AccelerationStructure,
            Origin,
            Direction,
            TMin,
            TMax,
            Time,
            InstanceInclusionMask,
            RayFlags,
            RayContributionToHitGroupIndex,
            MultiplierForGeometryContributionToHitGroupIndex,
            MissShaderIndex,
            pr.regs[0],
            pr.regs[1],
            pr.regs[2],
            pr.regs[3],
            pr.regs[4],
            pr.regs[5],
            pr.regs[6],
            pr.regs[7]);
    }
    else if constexpr (N <= 16)
    {
        optixTraverse(
            AccelerationStructure,
            Origin,
            Direction,
            TMin,
            TMax,
            Time,
            InstanceInclusionMask,
            RayFlags,
            RayContributionToHitGroupIndex,
            MultiplierForGeometryContributionToHitGroupIndex,
            MissShaderIndex,
            pr.regs[0],
            pr.regs[1],
            pr.regs[2],
            pr.regs[3],
            pr.regs[4],
            pr.regs[5],
            pr.regs[6],
            pr.regs[7],
            pr.regs[8],
            pr.regs[9],
            pr.regs[10],
            pr.regs[11],
            pr.regs[12],
            pr.regs[13],
            pr.regs[14],
            pr.regs[15]);
    }
    else if constexpr (N <= kMaxOptiXPayloadRegisters)
    {
        optixTraverse(
            AccelerationStructure,
            Origin,
            Direction,
            TMin,
            TMax,
            Time,
            InstanceInclusionMask,
            RayFlags,
            RayContributionToHitGroupIndex,
            MultiplierForGeometryContributionToHitGroupIndex,
            MissShaderIndex,
            pr.regs[0],
            pr.regs[1],
            pr.regs[2],
            pr.regs[3],
            pr.regs[4],
            pr.regs[5],
            pr.regs[6],
            pr.regs[7],
            pr.regs[8],
            pr.regs[9],
            pr.regs[10],
            pr.regs[11],
            pr.regs[12],
            pr.regs[13],
            pr.regs[14],
            pr.regs[15],
            pr.regs[16],
            pr.regs[17],
            pr.regs[18],
            pr.regs[19],
            pr.regs[20],
            pr.regs[21],
            pr.regs[22],
            pr.regs[23],
            pr.regs[24],
            pr.regs[25],
            pr.regs[26],
            pr.regs[27],
            pr.regs[28],
            pr.regs[29],
            pr.regs[30],
            pr.regs[31]);
    }
}

template<typename T>
__forceinline__ __device__ void optixTraverse(
    OptixTraversableHandle AccelerationStructure,
    uint32_t RayFlags,
    uint32_t InstanceInclusionMask,
    uint32_t RayContributionToHitGroupIndex,
    uint32_t MultiplierForGeometryContributionToHitGroupIndex,
    uint32_t MissShaderIndex,
    RayDesc Ray,
    T* Payload,
    OptixTraversableHandle* hitObj)
{
    constexpr size_t numRegs = (sizeof(T) + 3) / 4;

    if constexpr (numRegs <= kMaxOptiXPayloadRegisters)
    {
        // Register-based approach for small payloads
        PayloadRegisters<T> pr;
        pr.pack(*Payload);

        optixTraverseWithRegs<T>(
            AccelerationStructure,
            Ray.Origin,
            Ray.Direction,
            Ray.TMin,
            Ray.TMax,
            0.f, /* Time for motion blur */
            InstanceInclusionMask,
            RayFlags,
            RayContributionToHitGroupIndex,
            MultiplierForGeometryContributionToHitGroupIndex,
            MissShaderIndex,
            pr);

        // Read back updated payload registers
        // Native optixTrace updates regs in place
        pr.unpack(*Payload);
    }
    else
    {
        // Pointer-based fallback for large payloads
        uint32_t r0, r1;
        packOptiXRayPayloadPointer((void*)Payload, r0, r1);
        optixTraverse(
            AccelerationStructure,
            Ray.Origin,
            Ray.Direction,
            Ray.TMin,
            Ray.TMax,
            0.f,
            InstanceInclusionMask,
            RayFlags,
            RayContributionToHitGroupIndex,
            MultiplierForGeometryContributionToHitGroupIndex,
            MissShaderIndex,
            r0,
            r1);
    }
}

template<typename T>
__forceinline__ __device__ void optixTraverse(
    OptixTraversableHandle AccelerationStructure,
    uint32_t RayFlags,
    uint32_t InstanceInclusionMask,
    uint32_t RayContributionToHitGroupIndex,
    uint32_t MultiplierForGeometryContributionToHitGroupIndex,
    uint32_t MissShaderIndex,
    RayDesc Ray,
    float RayTime,
    T* Payload,
    OptixTraversableHandle* hitObj)
{
    constexpr size_t numRegs = (sizeof(T) + 3) / 4;

    if constexpr (numRegs <= kMaxOptiXPayloadRegisters)
    {
        // Register-based approach for small payloads
        PayloadRegisters<T> pr;
        pr.pack(*Payload);

        optixTraverseWithRegs<T>(
            AccelerationStructure,
            Ray.Origin,
            Ray.Direction,
            Ray.TMin,
            Ray.TMax,
            RayTime,
            InstanceInclusionMask,
            RayFlags,
            RayContributionToHitGroupIndex,
            MultiplierForGeometryContributionToHitGroupIndex,
            MissShaderIndex,
            pr);

        // Read back updated payload registers
        // Native optixTrace updates regs in place
        pr.unpack(*Payload);
    }
    else
    {
        // Pointer-based fallback for large payloads
        uint32_t r0, r1;
        packOptiXRayPayloadPointer((void*)Payload, r0, r1);
        optixTraverse(
            AccelerationStructure,
            Ray.Origin,
            Ray.Direction,
            Ray.TMin,
            Ray.TMax,
            RayTime,
            InstanceInclusionMask,
            RayFlags,
            RayContributionToHitGroupIndex,
            MultiplierForGeometryContributionToHitGroupIndex,
            MissShaderIndex,
            r0,
            r1);
    }
}

// Non-template overload for empty payload case.
// When Slang's type legalization eliminates an empty payload struct,
// the generated code calls optixTraverse without a payload argument.
__forceinline__ __device__ void optixTraverse(
    OptixTraversableHandle AccelerationStructure,
    uint32_t RayFlags,
    uint32_t InstanceInclusionMask,
    uint32_t RayContributionToHitGroupIndex,
    uint32_t MultiplierForGeometryContributionToHitGroupIndex,
    uint32_t MissShaderIndex,
    RayDesc Ray,
    OptixTraversableHandle* hitObj)
{
    optixTraverse(
        AccelerationStructure,
        Ray.Origin,
        Ray.Direction,
        Ray.TMin,
        Ray.TMax,
        0.f,
        InstanceInclusionMask,
        RayFlags,
        RayContributionToHitGroupIndex,
        MultiplierForGeometryContributionToHitGroupIndex,
        MissShaderIndex);
}

#if (OPTIX_VERSION >= 80100)
static __forceinline__ __device__ bool slangOptixHitObjectIsHit(OptixTraversableHandle* hitObj)
{
    return optixHitObjectIsHit();
}
#endif

#if (OPTIX_VERSION >= 80100)
static __forceinline__ __device__ bool slangOptixHitObjectIsMiss(OptixTraversableHandle* hitObj)
{
    return optixHitObjectIsMiss();
}
#endif

#if (OPTIX_VERSION >= 80100)
static __forceinline__ __device__ bool slangOptixHitObjectIsNop(OptixTraversableHandle* hitObj)
{
    return optixHitObjectIsNop();
}
#endif

#if (OPTIX_VERSION >= 90000)
static __forceinline__ __device__ uint
slangOptixHitObjectGetClusterId(OptixTraversableHandle* hitObj)
{
    return optixHitObjectGetClusterId();
}
#endif

#if (OPTIX_VERSION >= 80100)
static __forceinline__ __device__ void optixMakeMissHitObject(
    uint MissShaderIndex,
    RayDesc Ray,
    OptixTraversableHandle* missObj)
{
    optixMakeMissHitObject(
        MissShaderIndex,
        Ray.Origin,
        Ray.Direction,
        Ray.TMin,
        Ray.TMax,
        0.f /* rayTime */
#if (OPTIX_VERSION >= 90000)
        ,
        OPTIX_RAY_FLAG_NONE /* rayFlags*/
#endif
    );
}
#endif

#if (OPTIX_VERSION >= 80100)
static __forceinline__ __device__ void optixMakeMissHitObject(
    uint MissShaderIndex,
    RayDesc Ray,
    float CurrentTime,
    OptixTraversableHandle* missObj)
{
    optixMakeMissHitObject(
        MissShaderIndex,
        Ray.Origin,
        Ray.Direction,
        Ray.TMin,
        Ray.TMax,
        CurrentTime
#if (OPTIX_VERSION >= 90000)
        ,
        OPTIX_RAY_FLAG_NONE /* rayFlags*/
#endif
    );
}
#endif

#if (OPTIX_VERSION >= 90000)
template<typename T>
static __forceinline__ __device__ void optixMakeHitObject(
    OptixTraversableHandle AccelerationStructure,
    uint InstanceIndex,
    uint GeometryIndex,
    uint PrimitiveIndex,
    uint HitKind,
    uint RayContributionToHitGroupIndex,
    uint MultiplierForGeometryContributionToHitGroupIndex,
    RayDesc Ray,
    T attr,
    OptixTraversableHandle* handle)
{
    OptixTraverseData data{};
    optixHitObjectGetTraverseData(&data);
    optixMakeHitObject(
        AccelerationStructure,
        Ray.Origin,
        Ray.Direction,
        Ray.TMin,
        0.f,
        OPTIX_RAY_FLAG_NONE, /* rayFlags*/
        data,
        nullptr, /*OptixTraversableHandle* transforms*/
        0 /*numTransforms */);
}
#elif (OPTIX_VERSION >= 80100)
template<typename T>
static __forceinline__ __device__ void optixMakeHitObject(
    OptixTraversableHandle AccelerationStructure,
    uint InstanceIndex,
    uint GeometryIndex,
    uint PrimitiveIndex,
    uint HitKind,
    uint RayContributionToHitGroupIndex,
    uint MultiplierForGeometryContributionToHitGroupIndex,
    RayDesc Ray,
    T attr,
    OptixTraversableHandle* handle)
{
    // OptiX 8.1 version: call native optixMakeHitObject directly
    optixMakeHitObject(
        AccelerationStructure,                            // handle
        Ray.Origin,                                       // rayOrigin
        Ray.Direction,                                    // rayDirection
        Ray.TMin,                                         // tmin
        Ray.TMax,                                         // tmax
        0.f,                                              // rayTime
        RayContributionToHitGroupIndex,                   // sbtOffset
        MultiplierForGeometryContributionToHitGroupIndex, // sbtStride
        InstanceIndex,                                    // instIdx
        nullptr,                                          // transforms
        0,                                                // numTransforms
        GeometryIndex,                                    // sbtGASIdx
        PrimitiveIndex,                                   // primIdx
        HitKind                                           // hitKind
        /* no attributes passed - empty variadic pack */
    );
}
#endif

#if (OPTIX_VERSION >= 90000)
template<typename T>
static __forceinline__ __device__ void optixMakeHitObject(
    uint HitGroupRecordIndex,
    OptixTraversableHandle AccelerationStructure,
    uint InstanceIndex,
    uint GeometryIndex,
    uint PrimitiveIndex,
    uint HitKind,
    RayDesc Ray,
    T attr,
    OptixTraversableHandle* handle)
{
    OptixTraverseData data{};
    optixHitObjectGetTraverseData(&data);
    optixMakeHitObject(
        AccelerationStructure,
        Ray.Origin,
        Ray.Direction,
        Ray.TMin,
        0.f,
        OPTIX_RAY_FLAG_NONE, /* rayFlags*/
        data,
        nullptr, /*OptixTraversableHandle* transforms*/
        0 /*numTransforms */);
}
#elif (OPTIX_VERSION >= 80100)
template<typename T>
static __forceinline__ __device__ void optixMakeHitObject(
    uint HitGroupRecordIndex,
    OptixTraversableHandle AccelerationStructure,
    uint InstanceIndex,
    uint GeometryIndex,
    uint PrimitiveIndex,
    uint HitKind,
    RayDesc Ray,
    T attr,
    OptixTraversableHandle* handle)
{
    // OptiX 8.1 version: call optixMakeHitObjectWithRecord directly
    optixMakeHitObjectWithRecord(
        AccelerationStructure, // handle
        Ray.Origin,            // rayOrigin
        Ray.Direction,         // rayDirection
        Ray.TMin,              // tmin
        Ray.TMax,              // tmax
        0.f,                   // rayTime
        HitGroupRecordIndex,   // sbtRecordIndex
        InstanceIndex,         // instIdx
        nullptr,               // transforms
        0,                     // numTransforms
        GeometryIndex,         // sbtGASIdx
        PrimitiveIndex,        // primIdx
        HitKind                // hitKind
        /* no attributes passed - empty variadic pack */
    );
}
#endif

#if (OPTIX_VERSION >= 90000)
template<typename T>
static __forceinline__ __device__ void optixMakeHitObject(
    OptixTraversableHandle AccelerationStructure,
    uint InstanceIndex,
    uint GeometryIndex,
    uint PrimitiveIndex,
    uint HitKind,
    uint RayContributionToHitGroupIndex,
    uint MultiplierForGeometryContributionToHitGroupIndex,
    RayDesc Ray,
    float CurrentTime,
    T attr,
    OptixTraversableHandle* handle)
{
    OptixTraverseData data{};
    optixHitObjectGetTraverseData(&data);
    optixMakeHitObject(
        AccelerationStructure,
        Ray.Origin,
        Ray.Direction,
        Ray.TMin,
        CurrentTime,
        OPTIX_RAY_FLAG_NONE, /* rayFlags*/
        data,
        nullptr, /*OptixTraversableHandle* transforms*/
        0 /*numTransforms */);
}
#elif (OPTIX_VERSION >= 80100)
template<typename T>
static __forceinline__ __device__ void optixMakeHitObject(
    OptixTraversableHandle AccelerationStructure,
    uint InstanceIndex,
    uint GeometryIndex,
    uint PrimitiveIndex,
    uint HitKind,
    uint RayContributionToHitGroupIndex,
    uint MultiplierForGeometryContributionToHitGroupIndex,
    RayDesc Ray,
    float CurrentTime,
    T attr,
    OptixTraversableHandle* handle)
{
    // OptiX 8.1 version: call native optixMakeHitObject directly
    optixMakeHitObject(
        AccelerationStructure,                            // handle
        Ray.Origin,                                       // rayOrigin
        Ray.Direction,                                    // rayDirection
        Ray.TMin,                                         // tmin
        Ray.TMax,                                         // tmax
        CurrentTime,                                      // rayTime
        RayContributionToHitGroupIndex,                   // sbtOffset
        MultiplierForGeometryContributionToHitGroupIndex, // sbtStride
        InstanceIndex,                                    // instIdx
        nullptr,                                          // transforms
        0,                                                // numTransforms
        GeometryIndex,                                    // sbtGASIdx
        PrimitiveIndex,                                   // primIdx
        HitKind                                           // hitKind
        /* no attributes passed - empty variadic pack */
    );
}
#endif

#if (OPTIX_VERSION >= 90000)
template<typename T>
static __forceinline__ __device__ void optixMakeHitObject(
    uint HitGroupRecordIndex,
    OptixTraversableHandle AccelerationStructure,
    uint InstanceIndex,
    uint GeometryIndex,
    uint PrimitiveIndex,
    uint HitKind,
    RayDesc Ray,
    float CurrentTime,
    T attr,
    OptixTraversableHandle* handle)
{
    OptixTraverseData data{};
    optixHitObjectGetTraverseData(&data);
    optixMakeHitObject(
        AccelerationStructure,
        Ray.Origin,
        Ray.Direction,
        Ray.TMin,
        CurrentTime,
        OPTIX_RAY_FLAG_NONE, /* rayFlags*/
        data,
        nullptr, /*OptixTraversableHandle* transforms*/
        0 /*numTransforms */);
}
#elif (OPTIX_VERSION >= 80100)
template<typename T>
static __forceinline__ __device__ void optixMakeHitObject(
    uint HitGroupRecordIndex,
    OptixTraversableHandle AccelerationStructure,
    uint InstanceIndex,
    uint GeometryIndex,
    uint PrimitiveIndex,
    uint HitKind,
    RayDesc Ray,
    float CurrentTime,
    T attr,
    OptixTraversableHandle* handle)
{
    // OptiX 8.1 version: call optixMakeHitObjectWithRecord directly
    optixMakeHitObjectWithRecord(
        AccelerationStructure, // handle
        Ray.Origin,            // rayOrigin
        Ray.Direction,         // rayDirection
        Ray.TMin,              // tmin
        Ray.TMax,              // tmax
        CurrentTime,           // rayTime
        HitGroupRecordIndex,   // sbtRecordIndex
        InstanceIndex,         // instIdx
        nullptr,               // transforms
        0,                     // numTransforms
        GeometryIndex,         // sbtGASIdx
        PrimitiveIndex,        // primIdx
        HitKind                // hitKind
        /* no attributes passed - empty variadic pack */
    );
}
#endif

#if (OPTIX_VERSION >= 80100)
static __forceinline__ __device__ void slangOptixMakeNopHitObject(OptixTraversableHandle* Obj)
{
    optixMakeNopHitObject();
}
#endif

#if (OPTIX_VERSION >= 80100)
// Internal helper to call optixInvoke with the right number of register arguments
template<typename T, size_t N = (sizeof(T) + 3) / 4>
__forceinline__ __device__ void optixInvokeWithRegs(PayloadRegisters<T, N>& pr)
{
    if constexpr (N == 0)
    {
        optixInvoke();
    }
    else if constexpr (N == 1)
    {
        optixInvoke(pr.regs[0]);
    }
    else if constexpr (N == 2)
    {
        optixInvoke(pr.regs[0], pr.regs[1]);
    }
    else if constexpr (N == 3)
    {
        optixInvoke(pr.regs[0], pr.regs[1], pr.regs[2]);
    }
    else if constexpr (N == 4)
    {
        optixInvoke(pr.regs[0], pr.regs[1], pr.regs[2], pr.regs[3]);
    }
    else if constexpr (N == 5)
    {
        optixInvoke(pr.regs[0], pr.regs[1], pr.regs[2], pr.regs[3], pr.regs[4]);
    }
    else if constexpr (N == 6)
    {
        optixInvoke(pr.regs[0], pr.regs[1], pr.regs[2], pr.regs[3], pr.regs[4], pr.regs[5]);
    }
    else if constexpr (N == 7)
    {
        optixInvoke(
            pr.regs[0],
            pr.regs[1],
            pr.regs[2],
            pr.regs[3],
            pr.regs[4],
            pr.regs[5],
            pr.regs[6]);
    }
    else if constexpr (N == 8)
    {
        optixInvoke(
            pr.regs[0],
            pr.regs[1],
            pr.regs[2],
            pr.regs[3],
            pr.regs[4],
            pr.regs[5],
            pr.regs[6],
            pr.regs[7]);
    }
    else if constexpr (N <= 16)
    {
        optixInvoke(
            pr.regs[0],
            pr.regs[1],
            pr.regs[2],
            pr.regs[3],
            pr.regs[4],
            pr.regs[5],
            pr.regs[6],
            pr.regs[7],
            pr.regs[8],
            pr.regs[9],
            pr.regs[10],
            pr.regs[11],
            pr.regs[12],
            pr.regs[13],
            pr.regs[14],
            pr.regs[15]);
    }
    else if constexpr (N <= kMaxOptiXPayloadRegisters)
    {
        optixInvoke(
            pr.regs[0],
            pr.regs[1],
            pr.regs[2],
            pr.regs[3],
            pr.regs[4],
            pr.regs[5],
            pr.regs[6],
            pr.regs[7],
            pr.regs[8],
            pr.regs[9],
            pr.regs[10],
            pr.regs[11],
            pr.regs[12],
            pr.regs[13],
            pr.regs[14],
            pr.regs[15],
            pr.regs[16],
            pr.regs[17],
            pr.regs[18],
            pr.regs[19],
            pr.regs[20],
            pr.regs[21],
            pr.regs[22],
            pr.regs[23],
            pr.regs[24],
            pr.regs[25],
            pr.regs[26],
            pr.regs[27],
            pr.regs[28],
            pr.regs[29],
            pr.regs[30],
            pr.regs[31]);
    }
}

template<typename T>
static __forceinline__ __device__ void optixInvoke(
    OptixTraversableHandle AccelerationStructure,
    OptixTraversableHandle* HitOrMiss,
    T* Payload)
{
    constexpr size_t numRegs = (sizeof(T) + 3) / 4;

    if constexpr (numRegs <= kMaxOptiXPayloadRegisters)
    {
        // Register-based approach for small payloads
        PayloadRegisters<T> pr;
        pr.pack(*Payload);
        optixInvokeWithRegs<T>(pr);
        // Read back updated payload registers
        pr.unpack(*Payload);
    }
    else
    {
        // Pointer-based fallback for large payloads
        uint32_t r0, r1;
        packOptiXRayPayloadPointer((void*)Payload, r0, r1);
        optixInvoke(r0, r1);
    }
}

// Overload for empty payloads (when payload is eliminated by type legalization)
static __forceinline__ __device__ void optixInvoke(
    OptixTraversableHandle AccelerationStructure,
    OptixTraversableHandle* HitOrMiss)
{
    // Call OptiX invoke with no payload for empty payload case
    optixInvoke();
}
#endif

#if (OPTIX_VERSION >= 80100)
static __forceinline__ __device__ RayDesc optixHitObjectGetRayDesc(OptixTraversableHandle* obj)
{
    RayDesc ray = {
        optixHitObjectGetWorldRayOrigin(),
        optixHitObjectGetRayTmin(),
        optixHitObjectGetWorldRayDirection(),
        optixHitObjectGetRayTmax()};
    return ray;
}
#endif

#if (OPTIX_VERSION >= 80100)
static __forceinline__ __device__ uint
slangOptixHitObjectGetInstanceIndex(OptixTraversableHandle* Obj)
{
    return optixHitObjectGetInstanceIndex();
}
#endif

#if (OPTIX_VERSION >= 80100)
static __forceinline__ __device__ uint slangOptixHitObjectGetInstanceId(OptixTraversableHandle* Obj)
{
    return optixHitObjectGetInstanceId();
}
#endif

#if (OPTIX_VERSION >= 80000)
static __forceinline__ __device__ float slangOptixHitObjectGetRayTime(OptixTraversableHandle* Obj)
{
    return optixHitObjectGetRayTime();
}
#endif

#if (OPTIX_VERSION >= 80100)
static __forceinline__ __device__ float slangOptixHitObjectGetRayTmax(OptixTraversableHandle* Obj)
{
    return optixHitObjectGetRayTmax();
}
#endif

#if (OPTIX_VERSION >= 80100)
static __forceinline__ __device__ uint
slangOptixHitObjectGetSbtGASIndex(OptixTraversableHandle* Obj)
{
    return optixHitObjectGetSbtGASIndex();
}
#endif

#if (OPTIX_VERSION >= 80100)
static __forceinline__ __device__ uint
slangOptixHitObjectGetPrimitiveIndex(OptixTraversableHandle* Obj)
{
    return optixHitObjectGetPrimitiveIndex();
}
#endif

#if (OPTIX_VERSION >= 80100)
template<typename T>
static __forceinline__ __device__ T optixHitObjectGetAttribute(OptixTraversableHandle* Obj)
{
    constexpr size_t numInts = (sizeof(T) + sizeof(uint32_t) - 1) /
                               sizeof(uint32_t); // Number of 32-bit values, rounded up
    static_assert(numInts <= 8, "Attribute type is too large");

    // Create an array to hold the attribute values
    uint32_t values[numInts == 0 ? 1 : numInts] = {0}; // Ensure we have at least one element

    // Read the appropriate number of attribute registers
    if constexpr (numInts > 0)
        values[0] = optixHitObjectGetAttribute_0();
    if constexpr (numInts > 1)
        values[1] = optixHitObjectGetAttribute_1();
    if constexpr (numInts > 2)
        values[2] = optixHitObjectGetAttribute_2();
    if constexpr (numInts > 3)
        values[3] = optixHitObjectGetAttribute_3();
    if constexpr (numInts > 4)
        values[4] = optixHitObjectGetAttribute_4();
    if constexpr (numInts > 5)
        values[5] = optixHitObjectGetAttribute_5();
    if constexpr (numInts > 6)
        values[6] = optixHitObjectGetAttribute_6();
    if constexpr (numInts > 7)
        values[7] = optixHitObjectGetAttribute_7();

    // Reinterpret the array as the desired type
    T result;
    memcpy(&result, values, sizeof(T));
    return result;
}
#endif

#if (OPTIX_VERSION >= 80100)
static __forceinline__ __device__ uint
slangOptixHitObjectGetSbtRecordIndex(OptixTraversableHandle* Obj)
{
    return optixHitObjectGetSbtRecordIndex();
}
#endif

#if (OPTIX_VERSION >= 90000)
static __forceinline__ __device__ void slangOptixHitObjectSetSbtRecordIndex(
    OptixTraversableHandle* Obj,
    uint sbtRecordIndex)
{
    optixHitObjectSetSbtRecordIndex(sbtRecordIndex);
}
#endif

// HitObject transform matrix wrappers for SER (Shader Execution Reordering)
// These wrappers convert OptiX's float[12] matrix format to Slang's Matrix type
// Available in RG, CH, MS, CC, DC stages per OptiX documentation
// Note: optixHitObjectGetWorldToObjectTransformMatrix/optixHitObjectGetObjectToWorldTransformMatrix
// were added in OptiX 9.0 (not available in 8.0 or 8.1)
#if (OPTIX_VERSION >= 90000)
static __forceinline__ __device__ Matrix<float, 4, 3> slangOptixHitObjectGetWorldToObject(
    OptixTraversableHandle* hitObj)
{
    float m[12];
    optixHitObjectGetWorldToObjectTransformMatrix(m);
    // OptiX stores matrix as 3 rows of float4, we need to transpose to 4 rows of float3
    return makeMatrix<float, 4, 3>(
        make_float3(m[0], m[4], m[8]),
        make_float3(m[1], m[5], m[9]),
        make_float3(m[2], m[6], m[10]),
        make_float3(m[3], m[7], m[11]));
}
#endif

#if (OPTIX_VERSION >= 90000)
static __forceinline__ __device__ Matrix<float, 4, 3> slangOptixHitObjectGetObjectToWorld(
    OptixTraversableHandle* hitObj)
{
    float m[12];
    optixHitObjectGetObjectToWorldTransformMatrix(m);
    // OptiX stores matrix as 3 rows of float4, we need to transpose to 4 rows of float3
    return makeMatrix<float, 4, 3>(
        make_float3(m[0], m[4], m[8]),
        make_float3(m[1], m[5], m[9]),
        make_float3(m[2], m[6], m[10]),
        make_float3(m[3], m[7], m[11]));
}
#endif

// OptiX multi-level traversal wrappers
// These wrappers convert OptiX's float[12] matrix pointer returns to Slang's Matrix type
__device__ __forceinline__ Matrix<float, 3, 4> _slang_optixGetInstanceTransformFromHandle(
    ulonglong handle)
{
    const float4* m = optixGetInstanceTransformFromHandle(handle);
    // OptiX stores matrix as 3 rows of float4 in the array
    return makeMatrix<float, 3, 4>(m[0], m[1], m[2]);
}

__device__ __forceinline__ Matrix<float, 3, 4> _slang_optixGetInstanceInverseTransformFromHandle(
    ulonglong handle)
{
    const float4* m = optixGetInstanceInverseTransformFromHandle(handle);
    // OptiX stores matrix as 3 rows of float4 in the array
    return makeMatrix<float, 3, 4>(m[0], m[1], m[2]);
}

// OptiX transformation matrix wrappers
// These wrappers convert OptiX's float[12] matrix format to Slang's Matrix type
__device__ __forceinline__ Matrix<float, 3, 4> slangOptixGetObjectToWorldTransformMatrix()
{
    float m[12];
    optixGetObjectToWorldTransformMatrix(m);
    // OptiX stores matrix as 3 rows of float4 in the array
    return makeMatrix<float, 3, 4>(
        make_float4(m[0], m[1], m[2], m[3]),
        make_float4(m[4], m[5], m[6], m[7]),
        make_float4(m[8], m[9], m[10], m[11]));
}

__device__ __forceinline__ Matrix<float, 3, 4> slangOptixGetWorldToObjectTransformMatrix()
{
    float m[12];
    optixGetWorldToObjectTransformMatrix(m);
    // OptiX stores matrix as 3 rows of float4 in the array
    return makeMatrix<float, 3, 4>(
        make_float4(m[0], m[1], m[2], m[3]),
        make_float4(m[4], m[5], m[6], m[7]),
        make_float4(m[8], m[9], m[10], m[11]));
}

__device__ __forceinline__ Matrix<float, 4, 3> slangOptixGetObjectToWorldTransformMatrix4x3()
{
    float m[12];
    optixGetObjectToWorldTransformMatrix(m);
    // OptiX stores matrix as 3 rows of float4, we need to transpose to 4 rows of float3
    return makeMatrix<float, 4, 3>(
        make_float3(m[0], m[4], m[8]),
        make_float3(m[1], m[5], m[9]),
        make_float3(m[2], m[6], m[10]),
        make_float3(m[3], m[7], m[11]));
}

__device__ __forceinline__ Matrix<float, 4, 3> slangOptixGetWorldToObjectTransformMatrix4x3()
{
    float m[12];
    optixGetWorldToObjectTransformMatrix(m);
    // OptiX stores matrix as 3 rows of float4, we need to transpose to 4 rows of float3
    return makeMatrix<float, 4, 3>(
        make_float3(m[0], m[4], m[8]),
        make_float3(m[1], m[5], m[9]),
        make_float3(m[2], m[6], m[10]),
        make_float3(m[3], m[7], m[11]));
}

#else
// Define OptixTraversableHandle even if OptiX is not enabled.
// This allows RaytracingAccelerationStructure to be properly reflected in non-OptiX code.
typedef unsigned long long OptixTraversableHandle;
#endif
static const int kSlangTorchTensorMaxDim = 5;

// TensorView
// NOTE: If you change this struct's layout, also update the hard-coded size/alignment
// in _createTypeLayout() in slang-type-layout.cpp.
struct TensorView
{
    uint8_t* data;
    uint32_t strides[kSlangTorchTensorMaxDim];
    uint32_t sizes[kSlangTorchTensorMaxDim];
    uint32_t dimensionCount;

    template<typename T>
    __device__ T* data_ptr()
    {
        return reinterpret_cast<T*>(data);
    }

    template<typename T>
    __device__ T* data_ptr_at(uint32_t index)
    {
        uint64_t offset = strides[0] * index;
        return reinterpret_cast<T*>(data + offset);
    }

    template<typename T>
    __device__ T* data_ptr_at(uint2 index)
    {
        uint64_t offset = strides[0] * index.x + strides[1] * index.y;
        return reinterpret_cast<T*>(data + offset);
    }

    template<typename T>
    __device__ T* data_ptr_at(uint3 index)
    {
        uint64_t offset = strides[0] * index.x + strides[1] * index.y + strides[2] * index.z;
        return reinterpret_cast<T*>(data + offset);
    }

    template<typename T>
    __device__ T* data_ptr_at(uint4 index)
    {
        uint64_t offset = strides[0] * index.x + strides[1] * index.y + strides[2] * index.z +
                          strides[3] * index.w;
        return reinterpret_cast<T*>(data + offset);
    }

    template<typename T, unsigned int N>
    __device__ T* data_ptr_at(uint index[N])
    {
        uint64_t offset = 0;
        for (unsigned int i = 0; i < N; ++i)
        {
            offset += strides[i] * index[i];
        }
        return reinterpret_cast<T*>(data + offset);
    }

    template<typename T>
    __device__ T& load(uint32_t x)
    {
        return *reinterpret_cast<T*>(data + strides[0] * x);
    }
    template<typename T>
    __device__ T& load(uint32_t x, uint32_t y)
    {
        return *reinterpret_cast<T*>(data + strides[0] * x + strides[1] * y);
    }
    template<typename T>
    __device__ T& load(uint2 index)
    {
        return *reinterpret_cast<T*>(data + strides[0] * index.x + strides[1] * index.y);
    }
    template<typename T>
    __device__ T& load(uint32_t x, uint32_t y, uint32_t z)
    {
        return *reinterpret_cast<T*>(data + strides[0] * x + strides[1] * y + strides[2] * z);
    }
    template<typename T>
    __device__ T& load(uint3 index)
    {
        return *reinterpret_cast<T*>(
            data + strides[0] * index.x + strides[1] * index.y + strides[2] * index.z);
    }
    template<typename T>
    __device__ T& load(uint32_t x, uint32_t y, uint32_t z, uint32_t w)
    {
        return *reinterpret_cast<T*>(
            data + strides[0] * x + strides[1] * y + strides[2] * z + strides[3] * w);
    }
    template<typename T>
    __device__ T& load(uint4 index)
    {
        return *reinterpret_cast<T*>(
            data + strides[0] * index.x + strides[1] * index.y + strides[2] * index.z +
            strides[3] * index.w);
    }
    template<typename T>
    __device__ T& load(uint32_t i0, uint32_t i1, uint32_t i2, uint32_t i3, uint32_t i4)
    {
        return *reinterpret_cast<T*>(
            data + strides[0] * i0 + strides[1] * i1 + strides[2] * i2 + strides[3] * i3 +
            strides[4] * i4);
    }

    // Generic version of load
    template<typename T, unsigned int N>
    __device__ T& load(uint index[N])
    {
        uint64_t offset = 0;
        for (unsigned int i = 0; i < N; ++i)
        {
            offset += strides[i] * index[i];
        }
        return *reinterpret_cast<T*>(data + offset);
    }

    template<typename T>
    __device__ void store(uint32_t x, T val)
    {
        *reinterpret_cast<T*>(data + strides[0] * x) = val;
    }
    template<typename T>
    __device__ void store(uint32_t x, uint32_t y, T val)
    {
        *reinterpret_cast<T*>(data + strides[0] * x + strides[1] * y) = val;
    }
    template<typename T>
    __device__ void store(uint2 index, T val)
    {
        *reinterpret_cast<T*>(data + strides[0] * index.x + strides[1] * index.y) = val;
    }
    template<typename T>
    __device__ void store(uint32_t x, uint32_t y, uint32_t z, T val)
    {
        *reinterpret_cast<T*>(data + strides[0] * x + strides[1] * y + strides[2] * z) = val;
    }
    template<typename T>
    __device__ void store(uint3 index, T val)
    {
        *reinterpret_cast<T*>(
            data + strides[0] * index.x + strides[1] * index.y + strides[2] * index.z) = val;
    }
    template<typename T>
    __device__ void store(uint32_t x, uint32_t y, uint32_t z, uint32_t w, T val)
    {
        *reinterpret_cast<T*>(
            data + strides[0] * x + strides[1] * y + strides[2] * z + strides[3] * w) = val;
    }
    template<typename T>
    __device__ void store(uint4 index, T val)
    {
        *reinterpret_cast<T*>(
            data + strides[0] * index.x + strides[1] * index.y + strides[2] * index.z +
            strides[3] * index.w) = val;
    }
    template<typename T>
    __device__ void store(uint32_t i0, uint32_t i1, uint32_t i2, uint32_t i3, uint32_t i4, T val)
    {
        *reinterpret_cast<T*>(
            data + strides[0] * i0 + strides[1] * i1 + strides[2] * i2 + strides[3] * i3 +
            strides[4] * i4) = val;
    }

    // Generic version
    template<typename T, unsigned int N>
    __device__ void store(uint index[N], T val)
    {
        uint64_t offset = 0;
        for (unsigned int i = 0; i < N; ++i)
        {
            offset += strides[i] * index[i];
        }
        *reinterpret_cast<T*>(data + offset) = val;
    }
};

// Implementations for texture fetch/load functions using tex PTX intrinsics
// These are used for read-only texture access with integer coordinates.

// 1D is not supported via PTX. Keeping the implementation below in case it ever gets supported.
template<typename T>
SLANG_FORCE_INLINE SLANG_CUDA_CALL T tex1Dfetch_int(CUtexObject texObj, int x, int mip)
{
    // TODO: static_assert(false) can fail on some compilers, even if template is not instantiated.
    // We should check for this in hlsl.meta.slang instead.
    // static_assert(false, "CUDA does not support fetching from 1D textures");
}

#if 0
#define SLANG_TEX1DFETCH_INT_IMPL(T, dtype, c)                                                 \
    template<>                                                                                 \
    SLANG_FORCE_INLINE SLANG_CUDA_CALL T tex1Dfetch_int(CUtexObject texObj, int x, int mip)    \
    {                                                                                          \
        T result;                                                                              \
        [[maybe_unused]] T stub;                                                               \
        asm("tex.level.1d.v4." dtype ".s32 {%0, %1, %2, %3}, [%4, {%5}], %6;"                  \
            : c(result), c(stub), c(stub), c(stub)                                             \
            : "l"(texObj), "r"(x), "r"(mip));                                                  \
        return result;                                                                         \
    }                                                                                          \
    template<>                                                                                 \
    SLANG_FORCE_INLINE SLANG_CUDA_CALL T##2 tex1Dfetch_int(CUtexObject texObj, int x, int mip) \
    {                                                                                          \
        T result_x, result_y;                                                                  \
        [[maybe_unused]] T stub;                                                               \
        asm("tex.level.1d.v4." dtype ".s32 {%0, %1, %2, %3}, [%4, {%5}], %6;"                  \
            : c(result_x), c(result_y), c(stub), c(stub)                                       \
            : "l"(texObj), "r"(x), "r"(mip));                                                  \
        return make_##T##2(result_x, result_y);                                                \
    }                                                                                          \
    template<>                                                                                 \
    SLANG_FORCE_INLINE SLANG_CUDA_CALL T##4 tex1Dfetch_int(CUtexObject texObj, int x, int mip) \
    {                                                                                          \
        T result_x, result_y, result_z, result_w;                                              \
        asm("tex.level.1d.v4." dtype ".s32 {%0, %1, %2, %3}, [%4, {%5}], %6;"                  \
            : c(result_x), c(result_y), c(result_z), c(result_w)                               \
            : "l"(texObj), "r"(x), "r"(mip));                                                  \
        return make_##T##4(result_x, result_y, result_z, result_w);                            \
    }

SLANG_TEX1DFETCH_INT_IMPL(float, "f32", "=f")
SLANG_TEX1DFETCH_INT_IMPL(uint, "u32", "=r")
SLANG_TEX1DFETCH_INT_IMPL(int, "s32", "=r")
#endif

template<typename T>
SLANG_FORCE_INLINE SLANG_CUDA_CALL T tex2Dfetch_int(CUtexObject texObj, int x, int y, int mip);

#define SLANG_TEX2DFETCH_INT_IMPL(T, dtype, c)                                                     \
    template<>                                                                                     \
    SLANG_FORCE_INLINE SLANG_CUDA_CALL T tex2Dfetch_int(CUtexObject texObj, int x, int y, int mip) \
    {                                                                                              \
        T result;                                                                                  \
        [[maybe_unused]] T stub;                                                                   \
        asm("tex.level.2d.v4." dtype ".s32 {%0, %1, %2, %3}, [%4, {%5, %6}], %7;"                  \
            : c(result), c(stub), c(stub), c(stub)                                                 \
            : "l"(texObj), "r"(x), "r"(y), "r"(mip));                                              \
        return result;                                                                             \
    }                                                                                              \
    template<>                                                                                     \
    SLANG_FORCE_INLINE SLANG_CUDA_CALL                                                             \
        T##2 tex2Dfetch_int(CUtexObject texObj, int x, int y, int mip)                             \
    {                                                                                              \
        T result_x, result_y;                                                                      \
        [[maybe_unused]] T stub;                                                                   \
        asm("tex.level.2d.v4." dtype ".s32 {%0, %1, %2, %3}, [%4, {%5, %6}], %7;"                  \
            : c(result_x), c(result_y), c(stub), c(stub)                                           \
            : "l"(texObj), "r"(x), "r"(y), "r"(mip));                                              \
        return make_##T##2(result_x, result_y);                                                    \
    }                                                                                              \
    template<>                                                                                     \
    SLANG_FORCE_INLINE SLANG_CUDA_CALL                                                             \
        T##4 tex2Dfetch_int(CUtexObject texObj, int x, int y, int mip)                             \
    {                                                                                              \
        T result_x, result_y, result_z, result_w;                                                  \
        asm("tex.level.2d.v4." dtype ".s32 {%0, %1, %2, %3}, [%4, {%5, %6}], %7;"                  \
            : c(result_x), c(result_y), c(result_z), c(result_w)                                   \
            : "l"(texObj), "r"(x), "r"(y), "r"(mip));                                              \
        return make_##T##4(result_x, result_y, result_z, result_w);                                \
    }

SLANG_TEX2DFETCH_INT_IMPL(float, "f32", "=f")
SLANG_TEX2DFETCH_INT_IMPL(uint, "u32", "=r")
SLANG_TEX2DFETCH_INT_IMPL(int, "s32", "=r")


template<typename T>
SLANG_FORCE_INLINE SLANG_CUDA_CALL T
tex3Dfetch_int(CUtexObject texObj, int x, int y, int z, int mip);

#define SLANG_TEX3DFETCH_INT_IMPL(T, dtype, c)                                            \
    template<>                                                                            \
    SLANG_FORCE_INLINE SLANG_CUDA_CALL T                                                  \
    tex3Dfetch_int(CUtexObject texObj, int x, int y, int z, int mip)                      \
    {                                                                                     \
        T result;                                                                         \
        [[maybe_unused]] T stub;                                                          \
        asm("tex.level.3d.v4." dtype ".s32 {%0, %1, %2, %3}, [%4, {%5, %6, %7, %8}], %9;" \
            : c(result), c(stub), c(stub), c(stub)                                        \
            : "l"(texObj), "r"(x), "r"(y), "r"(z), "r"(z) /* ignored */, "r"(mip));       \
        return result;                                                                    \
    }                                                                                     \
    template<>                                                                            \
    SLANG_FORCE_INLINE SLANG_CUDA_CALL                                                    \
        T##2 tex3Dfetch_int(CUtexObject texObj, int x, int y, int z, int mip)             \
    {                                                                                     \
        T result_x, result_y;                                                             \
        [[maybe_unused]] T stub;                                                          \
        asm("tex.level.3d.v4." dtype ".s32 {%0, %1, %2, %3}, [%4, {%5, %6, %7, %8}], %9;" \
            : c(result_x), c(result_y), c(stub), c(stub)                                  \
            : "l"(texObj), "r"(x), "r"(y), "r"(z), "r"(z) /* ignored */, "r"(mip));       \
        return make_##T##2(result_x, result_y);                                           \
    }                                                                                     \
    template<>                                                                            \
    SLANG_FORCE_INLINE SLANG_CUDA_CALL                                                    \
        T##4 tex3Dfetch_int(CUtexObject texObj, int x, int y, int z, int mip)             \
    {                                                                                     \
        T result_x, result_y, result_z, result_w;                                         \
        asm("tex.level.3d.v4." dtype ".s32 {%0, %1, %2, %3}, [%4, {%5, %6, %7, %8}], %9;" \
            : c(result_x), c(result_y), c(result_z), c(result_w)                          \
            : "l"(texObj), "r"(x), "r"(y), "r"(z), "r"(z) /* ignored */, "r"(mip));       \
        return make_##T##4(result_x, result_y, result_z, result_w);                       \
    }

SLANG_TEX3DFETCH_INT_IMPL(float, "f32", "=f")
SLANG_TEX3DFETCH_INT_IMPL(uint, "u32", "=r")
SLANG_TEX3DFETCH_INT_IMPL(int, "s32", "=r")

template<typename T>
SLANG_FORCE_INLINE SLANG_CUDA_CALL T
tex1DArrayfetch_int(CUtexObject texObj, int x, int layer, int mip);

#define SLANG_TEX1DARRAYFETCH_INT_IMPL(T, dtype, c)                                \
    template<>                                                                     \
    SLANG_FORCE_INLINE SLANG_CUDA_CALL T                                           \
    tex1DArrayfetch_int(CUtexObject texObj, int x, int layer, int mip)             \
    {                                                                              \
        T result;                                                                  \
        [[maybe_unused]] T stub;                                                   \
        asm("tex.level.a1d.v4." dtype ".s32 {%0, %1, %2, %3}, [%4, {%5, %6}], %7;" \
            : c(result), c(stub), c(stub), c(stub)                                 \
            : "l"(texObj), "r"(layer), "r"(x), "r"(mip));                          \
        return result;                                                             \
    }                                                                              \
    template<>                                                                     \
    SLANG_FORCE_INLINE SLANG_CUDA_CALL                                             \
        T##2 tex1DArrayfetch_int(CUtexObject texObj, int x, int layer, int mip)    \
    {                                                                              \
        T result_x, result_y;                                                      \
        [[maybe_unused]] T stub;                                                   \
        asm("tex.level.a1d.v4." dtype ".s32 {%0, %1, %2, %3}, [%4, {%5, %6}], %7;" \
            : c(result_x), c(result_y), c(stub), c(stub)                           \
            : "l"(texObj), "r"(layer), "r"(x), "r"(mip));                          \
        return make_##T##2(result_x, result_y);                                    \
    }                                                                              \
    template<>                                                                     \
    SLANG_FORCE_INLINE SLANG_CUDA_CALL                                             \
        T##4 tex1DArrayfetch_int(CUtexObject texObj, int x, int layer, int mip)    \
    {                                                                              \
        T result_x, result_y, result_z, result_w;                                  \
        asm("tex.level.a1d.v4." dtype ".s32 {%0, %1, %2, %3}, [%4, {%5, %6}], %7;" \
            : c(result_x), c(result_y), c(result_z), c(result_w)                   \
            : "l"(texObj), "r"(layer), "r"(x), "r"(mip));                          \
        return make_##T##4(result_x, result_y, result_z, result_w);                \
    }

SLANG_TEX1DARRAYFETCH_INT_IMPL(float, "f32", "=f")
SLANG_TEX1DARRAYFETCH_INT_IMPL(uint, "u32", "=r")
SLANG_TEX1DARRAYFETCH_INT_IMPL(int, "s32", "=r")

template<typename T>
SLANG_FORCE_INLINE SLANG_CUDA_CALL T
tex2DArrayfetch_int(CUtexObject texObj, int x, int y, int layer, int mip);

#define SLANG_TEX2DARRAYFETCH_INT_IMPL(T, dtype, c)                                         \
    template<>                                                                              \
    SLANG_FORCE_INLINE SLANG_CUDA_CALL T                                                    \
    tex2DArrayfetch_int(CUtexObject texObj, int x, int y, int layer, int mip)               \
    {                                                                                       \
        T result;                                                                           \
        [[maybe_unused]] T stub;                                                            \
        asm("tex.level.a2d.v4." dtype ".s32 {%0, %1, %2, %3}, [%4, {%5, %6, %7, %8}], %9;"  \
            : c(result), c(stub), c(stub), c(stub)                                          \
            : "l"(texObj), "r"(layer), "r"(x), "r"(y), "r"(layer) /* ignored */, "r"(mip)); \
        return result;                                                                      \
    }                                                                                       \
    template<>                                                                              \
    SLANG_FORCE_INLINE SLANG_CUDA_CALL                                                      \
        T##2 tex2DArrayfetch_int(CUtexObject texObj, int x, int y, int layer, int mip)      \
    {                                                                                       \
        T result_x, result_y;                                                               \
        [[maybe_unused]] T stub;                                                            \
        asm("tex.level.a2d.v4." dtype ".s32 {%0, %1, %2, %3}, [%4, {%5, %6, %7, %8}], %9;"  \
            : c(result_x), c(result_y), c(stub), c(stub)                                    \
            : "l"(texObj), "r"(layer), "r"(x), "r"(y), "r"(layer) /* ignored */, "r"(mip)); \
        return make_##T##2(result_x, result_y);                                             \
    }                                                                                       \
    template<>                                                                              \
    SLANG_FORCE_INLINE SLANG_CUDA_CALL                                                      \
        T##4 tex2DArrayfetch_int(CUtexObject texObj, int x, int y, int layer, int mip)      \
    {                                                                                       \
        T result_x, result_y, result_z, result_w;                                           \
        asm("tex.level.a2d.v4." dtype ".s32 {%0, %1, %2, %3}, [%4, {%5, %6, %7, %8}], %9;"  \
            : c(result_x), c(result_y), c(result_z), c(result_w)                            \
            : "l"(texObj), "r"(layer), "r"(x), "r"(y), "r"(layer) /* ignored */, "r"(mip)); \
        return make_##T##4(result_x, result_y, result_z, result_w);                         \
    }

SLANG_TEX2DARRAYFETCH_INT_IMPL(float, "f32", "=f")
SLANG_TEX2DARRAYFETCH_INT_IMPL(uint, "u32", "=r")
SLANG_TEX2DARRAYFETCH_INT_IMPL(int, "s32", "=r")

// Wave rotate helper functions - templated approach
#define SLANG_WARP_FULL_MASK 0xFFFFFFFF

// Macro-based wave rotate implementation following codebase patterns
#define SLANG_WAVE_ROTATE_IMPL(T)                                                     \
    __device__ __forceinline__ T##2 _slang_waveRotate(T##2 value, unsigned int delta) \
    {                                                                                 \
        return make_##T##2(                                                           \
            (T)__shfl_sync(                                                           \
                SLANG_WARP_FULL_MASK,                                                 \
                value.x,                                                              \
                (_getLaneId() + delta) % SLANG_CUDA_WARP_SIZE),                       \
            (T)__shfl_sync(                                                           \
                SLANG_WARP_FULL_MASK,                                                 \
                value.y,                                                              \
                (_getLaneId() + delta) % SLANG_CUDA_WARP_SIZE));                      \
    }                                                                                 \
    __device__ __forceinline__ T##3 _slang_waveRotate(T##3 value, unsigned int delta) \
    {                                                                                 \
        return make_##T##3(                                                           \
            (T)__shfl_sync(                                                           \
                SLANG_WARP_FULL_MASK,                                                 \
                value.x,                                                              \
                (_getLaneId() + delta) % SLANG_CUDA_WARP_SIZE),                       \
            (T)__shfl_sync(                                                           \
                SLANG_WARP_FULL_MASK,                                                 \
                value.y,                                                              \
                (_getLaneId() + delta) % SLANG_CUDA_WARP_SIZE),                       \
            (T)__shfl_sync(                                                           \
                SLANG_WARP_FULL_MASK,                                                 \
                value.z,                                                              \
                (_getLaneId() + delta) % SLANG_CUDA_WARP_SIZE));                      \
    }                                                                                 \
    __device__ __forceinline__ T##4 _slang_waveRotate(T##4 value, unsigned int delta) \
    {                                                                                 \
        return make_##T##4(                                                           \
            (T)__shfl_sync(                                                           \
                SLANG_WARP_FULL_MASK,                                                 \
                value.x,                                                              \
                (_getLaneId() + delta) % SLANG_CUDA_WARP_SIZE),                       \
            (T)__shfl_sync(                                                           \
                SLANG_WARP_FULL_MASK,                                                 \
                value.y,                                                              \
                (_getLaneId() + delta) % SLANG_CUDA_WARP_SIZE),                       \
            (T)__shfl_sync(                                                           \
                SLANG_WARP_FULL_MASK,                                                 \
                value.z,                                                              \
                (_getLaneId() + delta) % SLANG_CUDA_WARP_SIZE),                       \
            (T)__shfl_sync(                                                           \
                SLANG_WARP_FULL_MASK,                                                 \
                value.w,                                                              \
                (_getLaneId() + delta) % SLANG_CUDA_WARP_SIZE));                      \
    }

// Generate wave rotate functions for all standard vector types
SLANG_WAVE_ROTATE_IMPL(uint)
SLANG_WAVE_ROTATE_IMPL(int)
SLANG_WAVE_ROTATE_IMPL(float)
SLANG_WAVE_ROTATE_IMPL(short)
SLANG_WAVE_ROTATE_IMPL(ushort)
SLANG_WAVE_ROTATE_IMPL(char)
SLANG_WAVE_ROTATE_IMPL(uchar)
SLANG_WAVE_ROTATE_IMPL(longlong)
SLANG_WAVE_ROTATE_IMPL(ulonglong)

#ifdef SLANG_CUDA_ENABLE_HALF
SLANG_WAVE_ROTATE_IMPL(__half)
#endif

// Special handling for boolean vectors (requires int conversion)
__device__ __forceinline__ bool2 _slang_waveRotate(bool2 value, unsigned int delta)
{
    int2 intValue = make_int2((int)value.x, (int)value.y);
    int2 result = _slang_waveRotate(intValue, delta);
    return make_bool2((bool)result.x, (bool)result.y);
}

__device__ __forceinline__ bool3 _slang_waveRotate(bool3 value, unsigned int delta)
{
    int3 intValue = make_int3((int)value.x, (int)value.y, (int)value.z);
    int3 result = _slang_waveRotate(intValue, delta);
    return make_bool3((bool)result.x, (bool)result.y, (bool)result.z);
}

__device__ __forceinline__ bool4 _slang_waveRotate(bool4 value, unsigned int delta)
{
    int4 intValue = make_int4((int)value.x, (int)value.y, (int)value.z, (int)value.w);
    int4 result = _slang_waveRotate(intValue, delta);
    return make_bool4((bool)result.x, (bool)result.y, (bool)result.z, (bool)result.w);
}

#undef SLANG_WAVE_ROTATE_IMPL

// Quad control operations for CUDA
__device__ __forceinline__ bool _slang_quadAny(bool expr)
{
    // Get values from all 4 lanes in the quad
    bool v0 = __shfl_sync(0xFFFFFFFF, expr, (_getLaneId() & 0xFFFFFFFC) | 0);
    bool v1 = __shfl_sync(0xFFFFFFFF, expr, (_getLaneId() & 0xFFFFFFFC) | 1);
    bool v2 = __shfl_sync(0xFFFFFFFF, expr, (_getLaneId() & 0xFFFFFFFC) | 2);
    bool v3 = __shfl_sync(0xFFFFFFFF, expr, (_getLaneId() & 0xFFFFFFFC) | 3);
    return v0 || v1 || v2 || v3;
}

__device__ __forceinline__ bool _slang_quadAll(bool expr)
{
    // Get values from all 4 lanes in the quad
    bool v0 = __shfl_sync(0xFFFFFFFF, expr, (_getLaneId() & 0xFFFFFFFC) | 0);
    bool v1 = __shfl_sync(0xFFFFFFFF, expr, (_getLaneId() & 0xFFFFFFFC) | 1);
    bool v2 = __shfl_sync(0xFFFFFFFF, expr, (_getLaneId() & 0xFFFFFFFC) | 2);
    bool v3 = __shfl_sync(0xFFFFFFFF, expr, (_getLaneId() & 0xFFFFFFFC) | 3);
    return v0 && v1 && v2 && v3;
}

// Clustered wave rotate operations for CUDA
// Clustered rotate rotates values within clusters of specified size
#define SLANG_WAVE_CLUSTERED_ROTATE_IMPL(T)                                                       \
    __device__ __forceinline__ T                                                                  \
    _slang_waveClusteredRotate(T value, unsigned int delta, unsigned int clusterSize)             \
    {                                                                                             \
        unsigned int laneId = _getLaneId();                                                       \
        unsigned int clusterStart = (laneId / clusterSize) * clusterSize;                         \
        unsigned int targetLane = clusterStart + ((laneId - clusterStart + delta) % clusterSize); \
        return __shfl_sync(SLANG_WARP_FULL_MASK, value, targetLane);                              \
    }                                                                                             \
    __device__ __forceinline__                                                                    \
        T##2 _slang_waveClusteredRotate(T##2 value, unsigned int delta, unsigned int clusterSize) \
    {                                                                                             \
        unsigned int laneId = _getLaneId();                                                       \
        unsigned int clusterStart = (laneId / clusterSize) * clusterSize;                         \
        unsigned int targetLane = clusterStart + ((laneId - clusterStart + delta) % clusterSize); \
        return make_##T##2(                                                                       \
            (T)__shfl_sync(SLANG_WARP_FULL_MASK, value.x, targetLane),                            \
            (T)__shfl_sync(SLANG_WARP_FULL_MASK, value.y, targetLane));                           \
    }                                                                                             \
    __device__ __forceinline__                                                                    \
        T##3 _slang_waveClusteredRotate(T##3 value, unsigned int delta, unsigned int clusterSize) \
    {                                                                                             \
        unsigned int laneId = _getLaneId();                                                       \
        unsigned int clusterStart = (laneId / clusterSize) * clusterSize;                         \
        unsigned int targetLane = clusterStart + ((laneId - clusterStart + delta) % clusterSize); \
        return make_##T##3(                                                                       \
            (T)__shfl_sync(SLANG_WARP_FULL_MASK, value.x, targetLane),                            \
            (T)__shfl_sync(SLANG_WARP_FULL_MASK, value.y, targetLane),                            \
            (T)__shfl_sync(SLANG_WARP_FULL_MASK, value.z, targetLane));                           \
    }                                                                                             \
    __device__ __forceinline__                                                                    \
        T##4 _slang_waveClusteredRotate(T##4 value, unsigned int delta, unsigned int clusterSize) \
    {                                                                                             \
        unsigned int laneId = _getLaneId();                                                       \
        unsigned int clusterStart = (laneId / clusterSize) * clusterSize;                         \
        unsigned int targetLane = clusterStart + ((laneId - clusterStart + delta) % clusterSize); \
        return make_##T##4(                                                                       \
            (T)__shfl_sync(SLANG_WARP_FULL_MASK, value.x, targetLane),                            \
            (T)__shfl_sync(SLANG_WARP_FULL_MASK, value.y, targetLane),                            \
            (T)__shfl_sync(SLANG_WARP_FULL_MASK, value.z, targetLane),                            \
            (T)__shfl_sync(SLANG_WARP_FULL_MASK, value.w, targetLane));                           \
    }

// Generate clustered wave rotate functions for all standard types
SLANG_WAVE_CLUSTERED_ROTATE_IMPL(uint)
SLANG_WAVE_CLUSTERED_ROTATE_IMPL(int)
SLANG_WAVE_CLUSTERED_ROTATE_IMPL(float)
SLANG_WAVE_CLUSTERED_ROTATE_IMPL(short)
SLANG_WAVE_CLUSTERED_ROTATE_IMPL(ushort)
SLANG_WAVE_CLUSTERED_ROTATE_IMPL(char)
SLANG_WAVE_CLUSTERED_ROTATE_IMPL(uchar)
SLANG_WAVE_CLUSTERED_ROTATE_IMPL(longlong)
SLANG_WAVE_CLUSTERED_ROTATE_IMPL(ulonglong)

#ifdef SLANG_CUDA_ENABLE_HALF
SLANG_WAVE_CLUSTERED_ROTATE_IMPL(__half)
#endif

// Special handling for boolean clustered rotate
__device__ __forceinline__ bool _slang_waveClusteredRotate(
    bool value,
    unsigned int delta,
    unsigned int clusterSize)
{
    int intValue = (int)value;
    int result = _slang_waveClusteredRotate(intValue, delta, clusterSize);
    return (bool)result;
}

__device__ __forceinline__ bool2
_slang_waveClusteredRotate(bool2 value, unsigned int delta, unsigned int clusterSize)
{
    int2 intValue = make_int2((int)value.x, (int)value.y);
    int2 result = _slang_waveClusteredRotate(intValue, delta, clusterSize);
    return make_bool2((bool)result.x, (bool)result.y);
}

__device__ __forceinline__ bool3
_slang_waveClusteredRotate(bool3 value, unsigned int delta, unsigned int clusterSize)
{
    int3 intValue = make_int3((int)value.x, (int)value.y, (int)value.z);
    int3 result = _slang_waveClusteredRotate(intValue, delta, clusterSize);
    return make_bool3((bool)result.x, (bool)result.y, (bool)result.z);
}

__device__ __forceinline__ bool4
_slang_waveClusteredRotate(bool4 value, unsigned int delta, unsigned int clusterSize)
{
    int4 intValue = make_int4((int)value.x, (int)value.y, (int)value.z, (int)value.w);
    int4 result = _slang_waveClusteredRotate(intValue, delta, clusterSize);
    return make_bool4((bool)result.x, (bool)result.y, (bool)result.z, (bool)result.w);
}

#undef SLANG_WAVE_CLUSTERED_ROTATE_IMPL

// ---------------------- OptiX Cooperative Vector Wrappers --------------------------------------
#ifdef SLANG_CUDA_ENABLE_OPTIX

#if (OPTIX_VERSION >= 90000)

// Template trait to extract vector size from OptixCoopVec<T, N>
// Conditional compilation for NVRTC compatibility
template<typename T>
struct OptixCoopVecTraits;

// Template specialization for OptiX's OptixCoopVec - only enabled when cooperative vectors are
// available NVRTC explicitly disables cooperative vectors by setting
// OPTIX_INCLUDE_COOPERATIVE_VECTOR to 0
#if defined(OPTIX_VERSION) && OPTIX_VERSION > 90000
template<typename T, unsigned int N>
struct OptixCoopVecTraits<OptixCoopVec<T, N>>
{
    static constexpr unsigned int size = N;
};
#endif

template<
    typename VecTOut,
    typename VecTIn,
    OptixCoopVecElemType inputInterpretation,
    OptixCoopVecElemType matrixInterpretation,
    OptixCoopVecMatrixLayout matrixLayout>
__forceinline__ __device__ VecTOut slangOptixCoopVecMatMul(
    const VecTIn& inputVector,
    CUdeviceptr matrix,
    unsigned matrixOffset,
    bool transpose,
    unsigned matrixStride)
{
    constexpr unsigned N = OptixCoopVecTraits<VecTOut>::size; // Output vector size
    constexpr unsigned K = OptixCoopVecTraits<VecTIn>::size;  // Input vector size

    return optixCoopVecMatMul<
        VecTOut,
        VecTIn,
        inputInterpretation,
        matrixLayout,
        false,
        N,
        K,
        matrixInterpretation>(inputVector, matrix, matrixOffset, matrixStride);
}

// OptiX cooperative vector matrix multiplication wrapper (WITH bias - 6 runtime params)
template<
    typename VecTOut,
    typename VecTIn,
    OptixCoopVecElemType inputInterpretation,
    OptixCoopVecElemType matrixInterpretation,
    OptixCoopVecMatrixLayout matrixLayout,
    OptixCoopVecElemType biasInterpretation>
__forceinline__ __device__ VecTOut slangOptixCoopVecMatMul(
    const VecTIn& inputVector,
    CUdeviceptr matrix,
    unsigned matrixOffset,
    CUdeviceptr bias,
    unsigned biasOffset,
    unsigned matrixStride)
{
    constexpr unsigned N = OptixCoopVecTraits<VecTOut>::size; // Output vector size
    constexpr unsigned K = OptixCoopVecTraits<VecTIn>::size;  // Input vector size

    // Call OptiX SDK with bias (6 runtime parameters)
    return optixCoopVecMatMul<
        VecTOut,
        VecTIn,
        inputInterpretation,
        matrixLayout,
        false,
        N,
        K,
        matrixInterpretation,
        biasInterpretation>(inputVector, matrix, matrixOffset, bias, biasOffset, matrixStride);
}

// OptiX cooperative vector matrix multiplication wrapper (WITHOUT bias, 4 runtime params -
// StructuredBuffer variant)
template<
    typename VecTOut,
    typename VecTIn,
    OptixCoopVecElemType inputInterpretation,
    OptixCoopVecElemType matrixInterpretation,
    OptixCoopVecMatrixLayout matrixLayout>
__forceinline__ __device__ VecTOut slangOptixCoopVecMatMul(
    const VecTIn& inputVector,
    CUdeviceptr matrix,
    unsigned matrixOffset,
    unsigned matrixStride)
{
    constexpr unsigned N = OptixCoopVecTraits<VecTOut>::size; // Output vector size
    constexpr unsigned K = OptixCoopVecTraits<VecTIn>::size;  // Input vector size

    // Call OptiX SDK without bias and without transpose (4 runtime parameters)
    return optixCoopVecMatMul<
        VecTOut,
        VecTIn,
        inputInterpretation,
        matrixLayout,
        false,
        N,
        K,
        matrixInterpretation>(inputVector, matrix, matrixOffset, matrixStride);
}

#endif // (OPTIX_VERSION >= 90000)

#endif // SLANG_CUDA_ENABLE_OPTIX


// This implementation can only be enabled on CUDA Toolkit 12.5+
#if ((__CUDACC_VER_MAJOR__ > 12) || (__CUDACC_VER_MAJOR__ == 12 && __CUDACC_VER_MINOR__ >= 5)) || \
    (CUDA_VERSION >= 12050)
// The reason we have to implement our own wmma operation on CUDA is the interface
// design of cooperative_matrix on Vulkan is quite different from CUDA WMMA API, where
// SPIRV spec doesn't require the matrix layout during declaration of the cooperative_matrix,
// instead it is only required during load/store operations. However, in CUDA WMMA API, the layout
// has to be specified during the declaration of the fragment itself. Slang's interface desgin
// is more similar to SPIRV's cooperative_matrix. So to bridge this gap, we have to implement our
// wmma operation by using PTX wmma instructions directly, because PTX wmma instructions is quite
// similar to SPIRV's cooperative_matrix spec.
namespace Slang_CUDA_WMMA
{

// Enums for template specialization
enum MatrixUse : int
{
    MatrixA = 0,
    MatrixB = 1,
    MatrixC = 2,
    MatrixD = 3,
};

enum Layout : int
{
    RowMajor = 0,
    ColMajor = 1
};

enum ShapeCombination : int
{
    m16n16k16 = 0,
    m8n32k16 = 1,
    m32n8k16 = 2,
    m16n8k16 = 3
};

// ====================================================================================
// PTX Name Helpers
// ====================================================================================

// Shape names
template<int M, int N, int K>
struct PtxShapeName;
template<>
struct PtxShapeName<16, 16, 16>
{
    static constexpr const char name[] = "m16n16k16";
};
template<>
struct PtxShapeName<8, 32, 16>
{
    static constexpr const char name[] = "m8n32k16";
};
template<>
struct PtxShapeName<32, 8, 16>
{
    static constexpr const char name[] = "m32n8k16";
};
template<>
struct PtxShapeName<16, 8, 16>
{
    static constexpr const char name[] = "m16n8k16";
};

// Matrix role names
template<MatrixUse use>
struct PtxMatrixRoleName;
template<>
struct PtxMatrixRoleName<MatrixUse::MatrixA>
{
    static constexpr const char name[] = "a";
};
template<>
struct PtxMatrixRoleName<MatrixUse::MatrixB>
{
    static constexpr const char name[] = "b";
};
template<>
struct PtxMatrixRoleName<MatrixUse::MatrixC>
{
    static constexpr const char name[] = "c";
};
template<>
struct PtxMatrixRoleName<MatrixUse::MatrixD>
{
    static constexpr const char name[] = "d";
};

// Layout names
template<Layout layout>
struct PtxLayoutName;
template<>
struct PtxLayoutName<Layout::RowMajor>
{
    static constexpr const char name[] = "row";
};
template<>
struct PtxLayoutName<Layout::ColMajor>
{
    static constexpr const char name[] = "col";
};

// Type names
template<typename T>
struct PtxTypeName;

#if SLANG_CUDA_ENABLE_HALF
template<>
struct PtxTypeName<half>
{
    static constexpr const char name[] = "f16";
};
#endif // #if SLANG_CUDA_ENABLE_HALF

#if SLANG_CUDA_ENABLE_FP8
template<>
struct PtxTypeName<__nv_fp8_e4m3>
{
    static constexpr const char name[] = "f8e4m3";
};
template<>
struct PtxTypeName<__nv_fp8_e5m2>
{
    static constexpr const char name[] = "f8e5m2";
};
#endif // #if SLANG_CUDA_ENABLE_FP8

#if SLANG_CUDA_ENABLE_BF16
template<>
struct PtxTypeName<__nv_bfloat16>
{
    static constexpr const char name[] = "bf16";
};
#endif

template<>
struct PtxTypeName<float>
{
    static constexpr const char name[] = "f32";
};
template<>
struct PtxTypeName<char>
{
    static constexpr const char name[] = "s8";
};
template<>
struct PtxTypeName<unsigned char>
{
    static constexpr const char name[] = "u8";
};
template<>
struct PtxTypeName<int32_t>
{
    static constexpr const char name[] = "s32";
};

// ====================================================================================
// Register Counts for different matrices
// ====================================================================================
template<typename ElemT, int M, int N, int K, MatrixUse use>
struct RegisterCount;

#if SLANG_CUDA_ENABLE_HALF
// Half (f16) - 8 regs for A, 4 regs for B/C/D
template<int M, int N, int K>
struct RegisterCount<half, M, N, K, MatrixUse::MatrixA>
{
    static constexpr int value = 4;
};
template<int M, int N, int K>
struct RegisterCount<half, M, N, K, MatrixUse::MatrixB>
{
    static constexpr int value = 4;
};
template<int M, int N, int K>
struct RegisterCount<half, M, N, K, MatrixUse::MatrixC>
{
    static constexpr int value = 4;
};
template<int M, int N, int K>
struct RegisterCount<half, M, N, K, MatrixUse::MatrixD>
{
    static constexpr int value = 4;
};
#endif // #if SLANG_CUDA_ENABLE_HALF

#if SLANG_CUDA_ENABLE_BF16
// bfloat16 - 8 regs for A/B, 4 regs for C/D
template<int M, int N, int K>
struct RegisterCount<__nv_bfloat16, M, N, K, MatrixUse::MatrixA>
{
    static constexpr int value = 4;
};
template<int M, int N, int K>
struct RegisterCount<__nv_bfloat16, M, N, K, MatrixUse::MatrixB>
{
    static constexpr int value = 8;
};
template<int M, int N, int K>
struct RegisterCount<__nv_bfloat16, M, N, K, MatrixUse::MatrixC>
{
    static constexpr int value = 4;
};
template<int M, int N, int K>
struct RegisterCount<__nv_bfloat16, M, N, K, MatrixUse::MatrixD>
{
    static constexpr int value = 4;
};
#endif // #if SLANG_CUDA_ENABLE_BF16

// Float (f32) - 8 regs for C/D only
template<int M, int N, int K>
struct RegisterCount<float, M, N, K, MatrixUse::MatrixC>
{
    static constexpr int value = 8;
};
template<int M, int N, int K>
struct RegisterCount<float, M, N, K, MatrixUse::MatrixD>
{
    static constexpr int value = 8;
};

// Int32 (s32) - 8 regs for C/D (accumulator for int8 operations)
template<int M, int N, int K>
struct RegisterCount<int32_t, M, N, K, MatrixUse::MatrixC>
{
    static constexpr int value = 8;
};
template<int M, int N, int K>
struct RegisterCount<int32_t, M, N, K, MatrixUse::MatrixD>
{
    static constexpr int value = 8;
};

// Uint8 (u8) - varies by shape
template<>
struct RegisterCount<unsigned char, 16, 16, 16, MatrixUse::MatrixA>
{
    static constexpr int value = 2;
};
template<>
struct RegisterCount<unsigned char, 16, 16, 16, MatrixUse::MatrixB>
{
    static constexpr int value = 2;
};
template<>
struct RegisterCount<unsigned char, 8, 32, 16, MatrixUse::MatrixA>
{
    static constexpr int value = 1;
};
template<>
struct RegisterCount<unsigned char, 8, 32, 16, MatrixUse::MatrixB>
{
    static constexpr int value = 4;
};
template<>
struct RegisterCount<unsigned char, 32, 8, 16, MatrixUse::MatrixA>
{
    static constexpr int value = 4;
};
template<>
struct RegisterCount<unsigned char, 32, 8, 16, MatrixUse::MatrixB>
{
    static constexpr int value = 1;
};

// Int8 (s8) - same as u8
template<>
struct RegisterCount<char, 16, 16, 16, MatrixUse::MatrixA>
{
    static constexpr int value = 2;
};
template<>
struct RegisterCount<char, 16, 16, 16, MatrixUse::MatrixB>
{
    static constexpr int value = 2;
};
template<>
struct RegisterCount<char, 8, 32, 16, MatrixUse::MatrixA>
{
    static constexpr int value = 1;
};
template<>
struct RegisterCount<char, 8, 32, 16, MatrixUse::MatrixB>
{
    static constexpr int value = 4;
};
template<>
struct RegisterCount<char, 32, 8, 16, MatrixUse::MatrixA>
{
    static constexpr int value = 4;
};
template<>
struct RegisterCount<char, 32, 8, 16, MatrixUse::MatrixB>
{
    static constexpr int value = 1;
};

#if SLANG_CUDA_ENABLE_FP8
// fp8 - same as u8
template<>
struct RegisterCount<__nv_fp8_e4m3, 16, 16, 16, MatrixUse::MatrixA>
{
    static constexpr int value = 2;
};
template<>
struct RegisterCount<__nv_fp8_e4m3, 16, 16, 16, MatrixUse::MatrixB>
{
    static constexpr int value = 2;
};
template<>
struct RegisterCount<__nv_fp8_e4m3, 8, 32, 16, MatrixUse::MatrixA>
{
    static constexpr int value = 1;
};
template<>
struct RegisterCount<__nv_fp8_e4m3, 8, 32, 16, MatrixUse::MatrixB>
{
    static constexpr int value = 4;
};
template<>
struct RegisterCount<__nv_fp8_e4m3, 32, 8, 16, MatrixUse::MatrixA>
{
    static constexpr int value = 4;
};
template<>
struct RegisterCount<__nv_fp8_e4m3, 32, 8, 16, MatrixUse::MatrixB>
{
    static constexpr int value = 1;
};

template<>
struct RegisterCount<__nv_fp8_e5m2, 16, 16, 16, MatrixUse::MatrixA>
{
    static constexpr int value = 2;
};
template<>
struct RegisterCount<__nv_fp8_e5m2, 16, 16, 16, MatrixUse::MatrixB>
{
    static constexpr int value = 2;
};
template<>
struct RegisterCount<__nv_fp8_e5m2, 8, 32, 16, MatrixUse::MatrixA>
{
    static constexpr int value = 1;
};
template<>
struct RegisterCount<__nv_fp8_e5m2, 8, 32, 16, MatrixUse::MatrixB>
{
    static constexpr int value = 4;
};
template<>
struct RegisterCount<__nv_fp8_e5m2, 32, 8, 16, MatrixUse::MatrixA>
{
    static constexpr int value = 4;
};
template<>
struct RegisterCount<__nv_fp8_e5m2, 32, 8, 16, MatrixUse::MatrixB>
{
    static constexpr int value = 1;
};
#endif


// ====================================================================================
// MMA m16n8k16 Register Counts (override generic WMMA counts)
// A: 16x16 → 4 f16x2 regs, B: 16x8 → 2 f16x2 regs, C/D: 16x8 → 4 f32 or 2 f16x2
// ====================================================================================

#if SLANG_CUDA_ENABLE_HALF
template<>
struct RegisterCount<half, 16, 8, 16, MatrixUse::MatrixA>
{
    static constexpr int value = 4;
};
template<>
struct RegisterCount<half, 16, 8, 16, MatrixUse::MatrixB>
{
    static constexpr int value = 2;
};
template<>
struct RegisterCount<half, 16, 8, 16, MatrixUse::MatrixC>
{
    static constexpr int value = 2;
};
template<>
struct RegisterCount<half, 16, 8, 16, MatrixUse::MatrixD>
{
    static constexpr int value = 2;
};
#endif // #if SLANG_CUDA_ENABLE_HALF

#if SLANG_CUDA_ENABLE_BF16
template<>
struct RegisterCount<__nv_bfloat16, 16, 8, 16, MatrixUse::MatrixA>
{
    static constexpr int value = 4;
};
template<>
struct RegisterCount<__nv_bfloat16, 16, 8, 16, MatrixUse::MatrixB>
{
    static constexpr int value = 2;
};
template<>
struct RegisterCount<__nv_bfloat16, 16, 8, 16, MatrixUse::MatrixC>
{
    static constexpr int value = 2;
};
template<>
struct RegisterCount<__nv_bfloat16, 16, 8, 16, MatrixUse::MatrixD>
{
    static constexpr int value = 2;
};
#endif // #if SLANG_CUDA_ENABLE_BF16

template<>
struct RegisterCount<float, 16, 8, 16, MatrixUse::MatrixC>
{
    static constexpr int value = 4;
};
template<>
struct RegisterCount<float, 16, 8, 16, MatrixUse::MatrixD>
{
    static constexpr int value = 4;
};

// ====================================================================================
// Saturation at the output for integer MMA
// ====================================================================================
template<bool saturatingAccumulation>
struct IsSaturated;

template<>
struct IsSaturated<true>
{
    static constexpr const char name[] = ".satfinite";
};

template<>
struct IsSaturated<false>
{
    static constexpr const char name[] = "";
};

// ====================================================================================
// WMMA Load - Inline PTX
// ====================================================================================

template<typename ElemT, int M, int N, int K, MatrixUse use, Layout layout>
__device__ inline void wmmaLoad(uint32_t* regs, const void* ptr, int stride)
{
    constexpr int nregs = RegisterCount<ElemT, M, N, K, use>::value;

    switch (nregs)
    {
    case 1:
        asm volatile("wmma.load.%1.sync.aligned.%2.%3.%4 {%0}, [%5], %6;\n"
                     : "=r"(regs[0])
                     : "C"(PtxMatrixRoleName<use>::name),
                       "C"(PtxLayoutName<layout>::name),
                       "C"(PtxShapeName<M, N, K>::name),
                       "C"(PtxTypeName<ElemT>::name),
                       "l"(ptr),
                       "r"(stride));
        break;

    case 2:
        asm volatile("wmma.load.%2.sync.aligned.%3.%4.%5 {%0, %1}, [%6], %7;\n"
                     : "=r"(regs[0]), "=r"(regs[1])
                     : "C"(PtxMatrixRoleName<use>::name),
                       "C"(PtxLayoutName<layout>::name),
                       "C"(PtxShapeName<M, N, K>::name),
                       "C"(PtxTypeName<ElemT>::name),
                       "l"(ptr),
                       "r"(stride));
        break;

    case 4:
        asm volatile("wmma.load.%4.sync.aligned.%5.%6.%7 {%0, %1, %2, %3}, [%8], %9;\n"
                     : "=r"(regs[0]), "=r"(regs[1]), "=r"(regs[2]), "=r"(regs[3])
                     : "C"(PtxMatrixRoleName<use>::name),
                       "C"(PtxLayoutName<layout>::name),
                       "C"(PtxShapeName<M, N, K>::name),
                       "C"(PtxTypeName<ElemT>::name),
                       "l"(ptr),
                       "r"(stride));
        break;

    case 8:
        asm volatile("wmma.load.%8.sync.aligned.%9.%10.%11 "
                     "{%0, %1, %2, %3, %4, %5, %6, %7}, [%12], %13;\n"
                     : "=r"(regs[0]),
                       "=r"(regs[1]),
                       "=r"(regs[2]),
                       "=r"(regs[3]),
                       "=r"(regs[4]),
                       "=r"(regs[5]),
                       "=r"(regs[6]),
                       "=r"(regs[7])
                     : "C"(PtxMatrixRoleName<use>::name),
                       "C"(PtxLayoutName<layout>::name),
                       "C"(PtxShapeName<M, N, K>::name),
                       "C"(PtxTypeName<ElemT>::name),
                       "l"(ptr),
                       "r"(stride));
        break;
    }
}

// ====================================================================================
// WMMA Store - Inline PTX
// ====================================================================================

template<typename ElemT, int M, int N, int K, Layout layout>
__device__ inline void wmmaStore(void* ptr, const uint32_t* regs, int stride)
{
    constexpr int nregs = RegisterCount<ElemT, M, N, K, MatrixUse::MatrixD>::value;

    switch (nregs)
    {
    case 4:
        asm volatile("wmma.store.d.sync.aligned.%0.%1.%2 [%3], {%4, %5, %6, %7}, %8;\n"
                     :
                     : "C"(PtxLayoutName<layout>::name),
                       "C"(PtxShapeName<M, N, K>::name),
                       "C"(PtxTypeName<ElemT>::name),
                       "l"(ptr),
                       "r"(regs[0]),
                       "r"(regs[1]),
                       "r"(regs[2]),
                       "r"(regs[3]),
                       "r"(stride));
        break;

    case 8:
        asm volatile("wmma.store.d.sync.aligned.%0.%1.%2 "
                     "[%3], {%4, %5, %6, %7, %8, %9, %10, %11}, %12;\n"
                     :
                     : "C"(PtxLayoutName<layout>::name),
                       "C"(PtxShapeName<M, N, K>::name),
                       "C"(PtxTypeName<ElemT>::name),
                       "l"(ptr),
                       "r"(regs[0]),
                       "r"(regs[1]),
                       "r"(regs[2]),
                       "r"(regs[3]),
                       "r"(regs[4]),
                       "r"(regs[5]),
                       "r"(regs[6]),
                       "r"(regs[7]),
                       "r"(stride));
        break;
    }
}

// ====================================================================================
// MMA m16n8k16 Load/Store
// Uses 128-bit vectorized loads with warp shuffle redistribution for Matrix A,
// 64-bit paired loads for f32 C/D, and 32-bit coalesced loads for B.
// Falls back to element-wise access for column-major layouts where data is non-contiguous.
// ====================================================================================

__device__ inline uint32_t mmaPack2x16(const void* lo, const void* hi)
{
    uint16_t u0, u1;
    memcpy(&u0, lo, 2);
    memcpy(&u1, hi, 2);
    return (uint32_t)u0 | ((uint32_t)u1 << 16);
}

__device__ inline void mmaUnpack2x16(uint32_t packed, void* lo, void* hi)
{
    uint16_t u0 = (uint16_t)(packed & 0xFFFF);
    uint16_t u1 = (uint16_t)(packed >> 16);
    memcpy(lo, &u0, 2);
    memcpy(hi, &u1, 2);
}

template<Layout layout>
__device__ inline int mmaAddr(int row, int col, int stride)
{
    if constexpr (layout == Layout::RowMajor)
        return row * stride + col;
    else
        return col * stride + row;
}

// ====================================================================================
// MMA m16n8k16 Matrix A Load (f16, 16x16)
//
// Uses 128-bit vectorized load + warp shuffle redistribution.
// Each thread loads one 128-bit row-half, then 4 rounds of shuffles redistribute
// the data so each thread ends up with the correct MMA fragment registers.
//
// Target MMA fragment layout:
//
//          |<----------- columns 0-7 ----------->|<---------- columns 8-15 ----------->|
// R\C      |  c0,1    |  c2,3    |  c4,5    |  c6,7    ||  c8,9    | c10,11   | c12,13   | c14,15   |
// ---------+----------+----------+----------+----------++----------+----------+----------+----------+
// row  0   |  T0:a0a1 |  T1:a0a1 |  T2:a0a1 |  T3:a0a1 ||  T0:a4a5 |  T1:a4a5 |  T2:a4a5 |  T3:a4a5 |
// row  1   |  T4:a0a1 |  T5:a0a1 |  T6:a0a1 |  T7:a0a1 ||  T4:a4a5 |  T5:a4a5 |  T6:a4a5 |  T7:a4a5 |
//   ..     |    ..    |    ..    |    ..    |    ..    ||    ..    |    ..    |    ..    |    ..    |
// row  7   | T28:a0a1 | T29:a0a1 | T30:a0a1 | T31:a0a1 || T28:a4a5 | T29:a4a5 | T30:a4a5 | T31:a4a5 |
// ---------+----------+----------+----------+----------++----------+----------+----------+----------+
// row  8   |  T0:a2a3 |  T1:a2a3 |  T2:a2a3 |  T3:a2a3 ||  T0:a6a7 |  T1:a6a7 |  T2:a6a7 |  T3:a6a7 |
// row  9   |  T4:a2a3 |  T5:a2a3 |  T6:a2a3 |  T7:a2a3 ||  T4:a6a7 |  T5:a6a7 |  T6:a6a7 |  T7:a6a7 |
//   ..     |    ..    |    ..    |    ..    |    ..    ||    ..    |    ..    |    ..    |    ..    |
// row 15   | T28:a2a3 | T29:a2a3 | T30:a2a3 | T31:a2a3 || T28:a6a7 | T29:a6a7 | T30:a6a7 | T31:a6a7 |
// ---------+----------+----------+----------+----------++----------+----------+----------+----------+
//
// Initial state after 128-bit load (each thread holds one row-half):
//
//          |<----------- columns 0-7 ----------->|<---------- columns 8-15 ----------->|
// R\C      |  c0,1    |  c2,3    |  c4,5    |  c6,7    ||  c8,9    | c10,11   | c12,13   | c14,15   |
// ---------+----------+----------+----------+----------++----------+----------+----------+----------+
// row  0   |  T0:a0a1 |  T0:a2a3 |  T0:a4a5 |  T0:a6a7 ||  T1:a0a1 |  T1:a2a3 |  T1:a4a5 |  T1:a6a7 |
// row  1   |  T2:a0a1 |  T2:a2a3 |  T2:a4a5 |  T2:a6a7 ||  T3:a0a1 |  T3:a2a3 |  T3:a4a5 |  T3:a6a7 |
//   ..     |    ..    |    ..    |    ..    |    ..    ||    ..    |    ..    |    ..    |    ..    |
// row  7   | T14:a0a1 | T14:a2a3 | T14:a4a5 | T14:a6a7 || T15:a0a1 | T15:a2a3 | T15:a4a5 | T15:a6a7 |
// ---------+----------+----------+----------+----------++----------+----------+----------+----------+
// row  8   | T16:a0a1 | T16:a2a3 | T16:a4a5 | T16:a6a7 || T17:a0a1 | T17:a2a3 | T17:a4a5 | T17:a6a7 |
// row  9   | T18:a0a1 | T18:a2a3 | T18:a4a5 | T18:a6a7 || T19:a0a1 | T19:a2a3 | T19:a4a5 | T19:a6a7 |
//   ..     |    ..    |    ..    |    ..    |    ..    ||    ..    |    ..    |    ..    |    ..    |
// row 15   | T30:a0a1 | T30:a2a3 | T30:a4a5 | T30:a6a7 || T31:a0a1 | T31:a2a3 | T31:a4a5 | T31:a6a7 |
// ---------+----------+----------+----------+----------++----------+----------+----------+----------+
//
// After Round k=0 (shuffle loaded[0] -- captures col pairs 0,1 and 8,9):
//
//          |<----------- columns 0-7 ----------->|<---------- columns 8-15 ----------->|
// R\C      |  c0,1    |  c2,3    |  c4,5    |  c6,7    ||  c8,9    | c10,11   | c12,13   | c14,15   |
// ---------+----------+----------+----------+----------++----------+----------+----------+----------+
// row  0   |  T0:a0a1 |          |          |          ||  T0:a4a5 |          |          |          |
// row  1   |  T4:a0a1 |          |          |          ||  T4:a4a5 |          |          |          |
//   ..     |    ..    |          |          |          ||    ..    |          |          |          |
// row  7   | T28:a0a1 |          |          |          || T28:a4a5 |          |          |          |
// ---------+----------+----------+----------+----------++----------+----------+----------+----------+
// row  8   |  T0:a2a3 |          |          |          ||  T0:a6a7 |          |          |          |
// row  9   |  T4:a2a3 |          |          |          ||  T4:a6a7 |          |          |          |
//   ..     |    ..    |          |          |          ||    ..    |          |          |          |
// row 15   | T28:a2a3 |          |          |          || T28:a6a7 |          |          |          |
// ---------+----------+----------+----------+----------++----------+----------+----------+----------+
//
// After Round k=1 (shuffle loaded[1] -- adds col pairs 2,3 and 10,11):
//
//          |<----------- columns 0-7 ----------->|<---------- columns 8-15 ----------->|
// R\C      |  c0,1    |  c2,3    |  c4,5    |  c6,7    ||  c8,9    | c10,11   | c12,13   | c14,15   |
// ---------+----------+----------+----------+----------++----------+----------+----------+----------+
// row  0   |  T0:a0a1 |  T1:a0a1 |          |          ||  T0:a4a5 |  T1:a4a5 |          |          |
// row  1   |  T4:a0a1 |  T5:a0a1 |          |          ||  T4:a4a5 |  T5:a4a5 |          |          |
//   ..     |    ..    |    ..    |          |          ||    ..    |    ..    |          |          |
// row  7   | T28:a0a1 | T29:a0a1 |          |          || T28:a4a5 | T29:a4a5 |          |          |
// ---------+----------+----------+----------+----------++----------+----------+----------+----------+
// row  8   |  T0:a2a3 |  T1:a2a3 |          |          ||  T0:a6a7 |  T1:a6a7 |          |          |
// row  9   |  T4:a2a3 |  T5:a2a3 |          |          ||  T4:a6a7 |  T5:a6a7 |          |          |
//   ..     |    ..    |    ..    |          |          ||    ..    |    ..    |          |          |
// row 15   | T28:a2a3 | T29:a2a3 |          |          || T28:a6a7 | T29:a6a7 |          |          |
// ---------+----------+----------+----------+----------++----------+----------+----------+----------+
//
// After Round k=2 (shuffle loaded[2] -- adds col pairs 4,5 and 12,13):
//
//          |<----------- columns 0-7 ----------->|<---------- columns 8-15 ----------->|
// R\C      |  c0,1    |  c2,3    |  c4,5    |  c6,7    ||  c8,9    | c10,11   | c12,13   | c14,15   |
// ---------+----------+----------+----------+----------++----------+----------+----------+----------+
// row  0   |  T0:a0a1 |  T1:a0a1 |  T2:a0a1 |          ||  T0:a4a5 |  T1:a4a5 |  T2:a4a5 |          |
// row  1   |  T4:a0a1 |  T5:a0a1 |  T6:a0a1 |          ||  T4:a4a5 |  T5:a4a5 |  T6:a4a5 |          |
//   ..     |    ..    |    ..    |    ..    |          ||    ..    |    ..    |    ..    |          |
// row  7   | T28:a0a1 | T29:a0a1 | T30:a0a1 |          || T28:a4a5 | T29:a4a5 | T30:a4a5 |          |
// ---------+----------+----------+----------+----------++----------+----------+----------+----------+
// row  8   |  T0:a2a3 |  T1:a2a3 |  T2:a2a3 |          ||  T0:a6a7 |  T1:a6a7 |  T2:a6a7 |          |
// row  9   |  T4:a2a3 |  T5:a2a3 |  T6:a2a3 |          ||  T4:a6a7 |  T5:a6a7 |  T6:a6a7 |          |
//   ..     |    ..    |    ..    |    ..    |          ||    ..    |    ..    |    ..    |          |
// row 15   | T28:a2a3 | T29:a2a3 | T30:a2a3 |          || T28:a6a7 | T29:a6a7 | T30:a6a7 |          |
// ---------+----------+----------+----------+----------++----------+----------+----------+----------+
//
// After Round k=3 (shuffle loaded[3] -- adds col pairs 6,7 and 14,15 -- COMPLETE):
// (matches the target layout above)
//
// ====================================================================================
// Shuffle-only: redistributes pre-loaded uint32 data into MMA fragment registers.
// `loaded` must contain 4 uint32 values in the same format as a 128-bit memory load.
template<Layout layout>
__device__ inline void mmaShuffleMatrixA_16x16(
    uint32_t* regs,
    const uint32_t* loaded,
    unsigned laneid,
    unsigned gid,
    unsigned tid)
{
    if constexpr (layout == Layout::RowMajor)
    {
        const uint32_t mask = 0xFFFFFFFF;
        uint32_t tmp;
        #pragma unroll
        for (int k = 0; k < 4; k++)
        {
            tmp = __shfl_sync(mask, loaded[k], gid * 2);       if (tid == k) regs[0] = tmp;
            tmp = __shfl_sync(mask, loaded[k], (gid + 8) * 2); if (tid == k) regs[1] = tmp;
            tmp = __shfl_sync(mask, loaded[k], gid * 2 + 1);   if (tid == k) regs[2] = tmp;
            tmp = __shfl_sync(mask, loaded[k], (gid+8)*2 + 1); if (tid == k) regs[3] = tmp;
        }
    }
    else
    {
        const uint32_t mask = 0xFFFFFFFF;
        unsigned k = gid >> 1;
        unsigned half_sel = gid & 1;

        uint32_t s[4][8];
        #pragma unroll
        for (int ki = 0; ki < 4; ki++)
        {
            s[ki][0] = __shfl_sync(mask, loaded[ki], tid * 4);
            s[ki][1] = __shfl_sync(mask, loaded[ki], tid * 4 + 1);
            s[ki][2] = __shfl_sync(mask, loaded[ki], tid * 4 + 2);
            s[ki][3] = __shfl_sync(mask, loaded[ki], tid * 4 + 3);
            s[ki][4] = __shfl_sync(mask, loaded[ki], tid * 4 + 16);
            s[ki][5] = __shfl_sync(mask, loaded[ki], tid * 4 + 17);
            s[ki][6] = __shfl_sync(mask, loaded[ki], tid * 4 + 18);
            s[ki][7] = __shfl_sync(mask, loaded[ki], tid * 4 + 19);
        }

        unsigned shift = half_sel * 16;
        uint16_t h0 = (uint16_t)(s[k][0] >> shift);
        uint16_t h1 = (uint16_t)(s[k][2] >> shift);
        regs[0] = (uint32_t)h0 | ((uint32_t)h1 << 16);

        h0 = (uint16_t)(s[k][1] >> shift);
        h1 = (uint16_t)(s[k][3] >> shift);
        regs[1] = (uint32_t)h0 | ((uint32_t)h1 << 16);

        h0 = (uint16_t)(s[k][4] >> shift);
        h1 = (uint16_t)(s[k][6] >> shift);
        regs[2] = (uint32_t)h0 | ((uint32_t)h1 << 16);

        h0 = (uint16_t)(s[k][5] >> shift);
        h1 = (uint16_t)(s[k][7] >> shift);
        regs[3] = (uint32_t)h0 | ((uint32_t)h1 << 16);
    }
}

// Inverse shuffle: converts MMA fragment registers back to the pre-shuffle loaded format.
template<Layout layout>
__device__ inline void mmaUnshuffleMatrixA_16x16(
    uint32_t* loaded,
    const uint32_t* regs,
    unsigned laneid,
    unsigned gid,
    unsigned tid)
{
    const uint32_t mask = 0xFFFFFFFF;
    if constexpr (layout == Layout::RowMajor)
    {
        unsigned g = laneid >> 1;
        unsigned side = laneid & 1;
        unsigned reg_sel = side * 2 + (g >= 8 ? 1 : 0);
        unsigned g_adj = (g >= 8) ? (g - 8) : g;
        #pragma unroll
        for (int k = 0; k < 4; k++)
            loaded[k] = __shfl_sync(mask, regs[reg_sel], g_adj * 4 + k);
    }
    else
    {
        unsigned g = laneid >> 1;
        unsigned side = laneid & 1;
        unsigned g_adj = (g >= 8) ? (g - 8) : g;
        unsigned reg_base = side * 2 + (g >= 8 ? 1 : 0);
        // Column-major loaded was indexed as loaded[ki] from 128-bit column load.
        // Inverse of the column-major forward shuffle with half-extraction.
        // Each thread originally loaded from col=laneid>>1, side=laneid&1.
        // Reconstruct by gathering from the 4 threads that used each loaded[ki].
        #pragma unroll
        for (int ki = 0; ki < 4; ki++)
        {
            // In forward: s[ki][j] = shfl(loaded[ki], tid*4+offset[j])
            // regs depend on k=gid>>1, half_sel=gid&1.
            // For inverse, collect from threads that consumed our loaded[ki].
            // Thread tid*4+{0..3,16..19} read our loaded[ki].
            // We need to reconstruct loaded[ki] = packed f16x2 from original memory.
            // Use the forward relationship: thread (tid*4+j) has s[ki][j] = our loaded[ki].
            // But s[ki][j] was combined with extraction, so direct inverse is complex.
            // Fall back to gathering via regs.
            uint32_t val = 0;
            for (int bit = 0; bit < 2; bit++)
            {
                // Element at position ki*2+bit in the 8-half loaded vector.
                // In the original column-major load, this corresponds to a specific (row, col).
                // Map through the fragment layout to find which thread+reg holds it.
                unsigned elem_row = (side * 8) + ki * 2 + bit;
                unsigned elem_col = g;
                // Fragment mapping for A 16x16:
                //   row < 8: gid_owner = row, reg_pair = 0,1 (a0a1 or a4a5)
                //   row >= 8: gid_owner = row-8, reg_pair = 2,3 (a2a3 or a6a7)
                //   col < 8: tid_owner = col/2, half_in_reg = col%2, reg_offset = 0
                //   col >= 8: tid_owner = (col-8)/2, half_in_reg = (col-8)%2, reg_offset = 2
                unsigned gid_owner = (elem_row < 8) ? elem_row : (elem_row - 8);
                unsigned row_reg_offset = (elem_row < 8) ? 0 : 1;
                unsigned tid_owner = (elem_col < 8) ? (elem_col / 2) : ((elem_col - 8) / 2);
                unsigned half_in_reg = (elem_col < 8) ? (elem_col % 2) : ((elem_col - 8) % 2);
                unsigned col_reg_offset = (elem_col < 8) ? 0 : 2;
                unsigned src_reg = row_reg_offset + col_reg_offset;
                unsigned src_lane = gid_owner * 4 + tid_owner;
                uint32_t src_val = __shfl_sync(mask, regs[src_reg], src_lane);
                uint16_t half_val = (uint16_t)(src_val >> (half_in_reg * 16));
                val |= ((uint32_t)half_val << (bit * 16));
            }
            loaded[ki] = val;
        }
    }
}

template<typename ElemT, Layout layout>
__device__ inline void mmaLoadMatrixA_16x16(
    uint32_t* regs,
    const ElemT* buffer,
    int stride,
    unsigned laneid,
    unsigned gid,
    unsigned tid)
{
    unsigned row = laneid >> 1;
    unsigned side = laneid & 1;
    uint4 loaded_v =
        *reinterpret_cast<const uint4*>(&buffer[row * stride + side * 8]);
    uint32_t* loaded = reinterpret_cast<uint32_t*>(&loaded_v);

    mmaShuffleMatrixA_16x16<layout>(regs, loaded, laneid, gid, tid);
}

// ====================================================================================
// MMA m16n8k16 Matrix B Load (f16, 16x8)
//
// Uses 128-bit vectorized load + warp shuffle redistribution.
// B's register packing is vertical: each reg packs 2 adjacent rows at one column.
// Column-major aligns naturally (8 shuffles), row-major requires extraction (16 shuffles).
//
// Target MMA fragment layout:
//
//          |  col 0   |  col 1   |  col 2   |   ..   |  col 7   |
// ---------+----------+----------+----------+--------+----------+
// row  0,1 | T0:b0b1  | T4:b0b1  | T8:b0b1  |   ..   |T28:b0b1  |
// row  2,3 | T1:b0b1  | T5:b0b1  | T9:b0b1  |   ..   |T29:b0b1  |
// row  4,5 | T2:b0b1  | T6:b0b1  |T10:b0b1  |   ..   |T30:b0b1  |
// row  6,7 | T3:b0b1  | T7:b0b1  |T11:b0b1  |   ..   |T31:b0b1  |
// ---------+----------+----------+----------+--------+----------+
// row  8,9 | T0:b2b3  | T4:b2b3  | T8:b2b3  |   ..   |T28:b2b3  |
// row 10,11| T1:b2b3  | T5:b2b3  | T9:b2b3  |   ..   |T29:b2b3  |
// row 12,13| T2:b2b3  | T6:b2b3  |T10:b2b3  |   ..   |T30:b2b3  |
// row 14,15| T3:b2b3  | T7:b2b3  |T11:b2b3  |   ..   |T31:b2b3  |
// ---------+----------+----------+----------+--------+----------+
//
// --- Column-major path (8 shuffles) ---
//
// Thread t loads column t/2, half t%2. loaded[k] = {B[base+2k][col], B[base+2k+1][col]}.
// This directly matches the vertical register packing.
//
// Initial state (column-major):
//
//          |  col 0   |  col 1   |   ..   |  col 7   |
// ---------+----------+----------+--------+----------+
// row  0,1 | T0:b0b1  | T2:b0b1  |   ..   |T14:b0b1  |
// row  2,3 | T0:b2b3  | T2:b2b3  |   ..   |T14:b2b3  |
// row  4,5 | T0:b4b5  | T2:b4b5  |   ..   |T14:b4b5  |
// row  6,7 | T0:b6b7  | T2:b6b7  |   ..   |T14:b6b7  |
// ---------+----------+----------+--------+----------+
// row  8,9 | T1:b0b1  | T3:b0b1  |   ..   |T15:b0b1  |
// row 10,11| T1:b2b3  | T3:b2b3  |   ..   |T15:b2b3  |
// row 12,13| T1:b4b5  | T3:b4b5  |   ..   |T15:b4b5  |
// row 14,15| T1:b6b7  | T3:b6b7  |   ..   |T15:b6b7  |
// ---------+----------+----------+--------+----------+
//
// After Round k=0 (captures row-pair 0,1 and 8,9):
//
//          |  col 0   |  col 1   |   ..   |  col 7   |
// ---------+----------+----------+--------+----------+
// row  0,1 | T0:b0b1  | T4:b0b1  |   ..   |T28:b0b1  |
// row  2,3 |          |          |        |          |
// row  4,5 |          |          |        |          |
// row  6,7 |          |          |        |          |
// ---------+----------+----------+--------+----------+
// row  8,9 | T0:b2b3  | T4:b2b3  |   ..   |T28:b2b3  |
// row 10,11|          |          |        |          |
// row 12,13|          |          |        |          |
// row 14,15|          |          |        |          |
// ---------+----------+----------+--------+----------+
//
// After Round k=1 (adds row-pair 2,3 and 10,11):
//
//          |  col 0   |  col 1   |   ..   |  col 7   |
// ---------+----------+----------+--------+----------+
// row  0,1 | T0:b0b1  | T4:b0b1  |   ..   |T28:b0b1  |
// row  2,3 | T1:b0b1  | T5:b0b1  |   ..   |T29:b0b1  |
// row  4,5 |          |          |        |          |
// row  6,7 |          |          |        |          |
// ---------+----------+----------+--------+----------+
// row  8,9 | T0:b2b3  | T4:b2b3  |   ..   |T28:b2b3  |
// row 10,11| T1:b2b3  | T5:b2b3  |   ..   |T29:b2b3  |
// row 12,13|          |          |        |          |
// row 14,15|          |          |        |          |
// ---------+----------+----------+--------+----------+
//
// After Round k=2 (adds row-pair 4,5 and 12,13):
//
//          |  col 0   |  col 1   |   ..   |  col 7   |
// ---------+----------+----------+--------+----------+
// row  0,1 | T0:b0b1  | T4:b0b1  |   ..   |T28:b0b1  |
// row  2,3 | T1:b0b1  | T5:b0b1  |   ..   |T29:b0b1  |
// row  4,5 | T2:b0b1  | T6:b0b1  |   ..   |T30:b0b1  |
// row  6,7 |          |          |        |          |
// ---------+----------+----------+--------+----------+
// row  8,9 | T0:b2b3  | T4:b2b3  |   ..   |T28:b2b3  |
// row 10,11| T1:b2b3  | T5:b2b3  |   ..   |T29:b2b3  |
// row 12,13| T2:b2b3  | T6:b2b3  |   ..   |T30:b2b3  |
// row 14,15|          |          |        |          |
// ---------+----------+----------+--------+----------+
//
// After Round k=3 (COMPLETE -- matches target):
// (same as target layout above)
//
// --- Row-major path (16 shuffles + extraction) ---
//
// Thread t loads row t%16 (128 bits = one full row of 8 halves).
// loaded[k] = {B[row][2k], B[row][2k+1]} -- a horizontal pair.
// Registers need vertical pairs, so we extract halves from 2 source rows.
//
// Source mapping for thread L (gid, tid):
//   reg0: rows tid*2 and tid*2+1 at column gid
//     sources: thread tid*2 and tid*2+1, loaded[gid>>1], half gid&1
//   reg1: rows tid*2+8 and tid*2+9 at column gid
//     sources: thread tid*2+8 and tid*2+9, loaded[gid>>1], half gid&1
//
// ====================================================================================
// Shuffle-only for Matrix B (16x8 f16).
template<Layout layout>
__device__ inline void mmaShuffleMatrixB_16x8(
    uint32_t* regs,
    const uint32_t* loaded,
    unsigned laneid,
    unsigned gid,
    unsigned tid)
{
    const uint32_t mask = 0xFFFFFFFF;
    if constexpr (layout == Layout::ColMajor)
    {
        uint32_t tmp;
        #pragma unroll
        for (int k = 0; k < 4; k++)
        {
            tmp = __shfl_sync(mask, loaded[k], gid * 2);     if (tid == k) regs[0] = tmp;
            tmp = __shfl_sync(mask, loaded[k], gid * 2 + 1); if (tid == k) regs[1] = tmp;
        }
    }
    else
    {
        uint32_t s[4][4];
        #pragma unroll
        for (int ki = 0; ki < 4; ki++)
        {
            s[ki][0] = __shfl_sync(mask, loaded[ki], tid * 2);
            s[ki][1] = __shfl_sync(mask, loaded[ki], tid * 2 + 1);
            s[ki][2] = __shfl_sync(mask, loaded[ki], tid * 2 + 8);
            s[ki][3] = __shfl_sync(mask, loaded[ki], tid * 2 + 9);
        }

        unsigned k = gid >> 1;
        unsigned shift = (gid & 1) * 16;

        uint16_t h0 = (uint16_t)(s[k][0] >> shift);
        uint16_t h1 = (uint16_t)(s[k][1] >> shift);
        regs[0] = (uint32_t)h0 | ((uint32_t)h1 << 16);

        h0 = (uint16_t)(s[k][2] >> shift);
        h1 = (uint16_t)(s[k][3] >> shift);
        regs[1] = (uint32_t)h0 | ((uint32_t)h1 << 16);
    }
}

// Inverse shuffle for Matrix B (16x8 f16).
template<Layout layout>
__device__ inline void mmaUnshuffleMatrixB_16x8(
    uint32_t* loaded,
    const uint32_t* regs,
    unsigned laneid,
    unsigned gid,
    unsigned tid)
{
    const uint32_t mask = 0xFFFFFFFF;
    if constexpr (layout == Layout::ColMajor)
    {
        unsigned side = laneid & 1;
        unsigned g_half = laneid >> 1;
        #pragma unroll
        for (int k = 0; k < 4; k++)
            loaded[k] = __shfl_sync(mask, regs[side], g_half * 4 + k);
    }
    else
    {
        // Row-major: inverse of the extraction-based shuffle.
        // Each thread originally loaded row laneid&15, with 4 uint32 (8 halves = full row).
        // Fragment B layout: reg[0]={B[tid*2][gid], B[tid*2+1][gid]}, reg[1]={B[tid*2+8][gid], ...}
        unsigned row = laneid & 15;
        #pragma unroll
        for (int ki = 0; ki < 4; ki++)
        {
            // loaded[ki] originally held {B[row][2*ki], B[row][2*ki+1]} packed as f16x2.
            // B[row][col] is in fragment of thread with tid_owner and gid_owner:
            //   tid_owner = row/2 (if row<8) or (row-8)/2 (if row>=8)
            //   reg_idx = 0 (if row<8) or 1 (if row>=8)
            //   gid_owner such that gid_owner=col (since gid maps to column in B)
            //   half within reg: if row is even -> low half, if odd -> high half
            uint32_t val = 0;
            for (int bit = 0; bit < 2; bit++)
            {
                unsigned col = ki * 2 + bit;
                unsigned tid_owner = (row < 8) ? (row / 2) : ((row - 8) / 2);
                unsigned reg_idx = (row < 8) ? 0 : 1;
                unsigned half_in_reg = row & 1;
                unsigned gid_owner = col;
                unsigned src_lane = gid_owner * 4 + tid_owner;
                uint32_t src_val = __shfl_sync(mask, regs[reg_idx], src_lane);
                uint16_t half_val = (uint16_t)(src_val >> (half_in_reg * 16));
                val |= ((uint32_t)half_val << (bit * 16));
            }
            loaded[ki] = val;
        }
    }
}

template<typename ElemT, Layout layout>
__device__ inline void mmaLoadMatrixB_16x8(
    uint32_t* regs,
    const ElemT* buffer,
    int stride,
    unsigned laneid,
    unsigned gid,
    unsigned tid)
{
    if constexpr (layout == Layout::ColMajor)
    {
        unsigned col = laneid >> 1;
        unsigned side = laneid & 1;
        uint4 loaded_v =
            *reinterpret_cast<const uint4*>(&buffer[col * stride + side * 8]);
        uint32_t* loaded = reinterpret_cast<uint32_t*>(&loaded_v);
        mmaShuffleMatrixB_16x8<layout>(regs, loaded, laneid, gid, tid);
    }
    else
    {
        unsigned row = laneid & 15;
        uint4 loaded_v =
            *reinterpret_cast<const uint4*>(&buffer[row * stride]);
        uint32_t* loaded = reinterpret_cast<uint32_t*>(&loaded_v);
        mmaShuffleMatrixB_16x8<layout>(regs, loaded, laneid, gid, tid);
    }
}

// ====================================================================================
// MMA m16n8k16 Matrix C/D Load (f32, 16x8)
//
// Target layout: same row-group structure as Matrix A but 8 columns.
// regs[0]=C[gid][tid*2], regs[1]=C[gid][tid*2+1],
// regs[2]=C[gid+8][tid*2], regs[3]=C[gid+8][tid*2+1]
//
// Row-major: each row = 8 floats = 32 bytes = 2 x 128 bits.
//   Thread t loads row t/2, side t%2 (left cols 0-3, right cols 4-7).
//   16 shuffles, select by: k_base = (tid&1)*2, src_base = (tid>>1)*2
//
// Column-major: each column = 16 floats = 64 bytes = 4 x 128 bits.
//   Thread t loads column t/4, chunk t%4 (rows chunk*4 .. chunk*4+3).
//   16 shuffles, select by: k = gid % 4
// ====================================================================================
// Shuffle-only for Matrix C/D (f32, 16x8).
template<Layout layout>
__device__ inline void mmaShuffleMatrixCD_f32_16x8(
    uint32_t* regs,
    const uint32_t* loaded,
    unsigned laneid,
    unsigned gid,
    unsigned tid)
{
    const uint32_t mask = 0xFFFFFFFF;

    if constexpr (layout == Layout::RowMajor)
    {
        // Original: s[k][j] where j indexes 4 source lanes.
        // regs[0]=s[k_base][src_base], regs[1]=s[k_base+1][src_base],
        // regs[2]=s[k_base][src_base+1], regs[3]=s[k_base+1][src_base+1]
        // k_base=(tid&1)*2, src_base=(tid>>1)*2
        uint32_t tmp;
        unsigned kb = (tid & 1) * 2;
        unsigned sb = (tid >> 1) * 2;
        #pragma unroll
        for (int k = 0; k < 4; k++)
        {
            #pragma unroll
            for (int j = 0; j < 4; j++)
            {
                unsigned srcLane = (j < 2) ? ((j == 0) ? gid * 2 : (gid + 8) * 2)
                                           : ((j == 2) ? gid * 2 + 1 : (gid + 8) * 2 + 1);
                tmp = __shfl_sync(mask, loaded[k], srcLane);
                if (k == kb     && j == sb)     regs[0] = tmp;
                if (k == kb + 1 && j == sb)     regs[1] = tmp;
                if (k == kb     && j == sb + 1) regs[2] = tmp;
                if (k == kb + 1 && j == sb + 1) regs[3] = tmp;
            }
        }
    }
    else
    {
        uint32_t tmp;
        unsigned k = gid & 3;
        #pragma unroll
        for (int ki = 0; ki < 4; ki++)
        {
            tmp = __shfl_sync(mask, loaded[ki], tid * 8 + gid / 4);     if (ki == k) regs[0] = tmp;
            tmp = __shfl_sync(mask, loaded[ki], tid * 8 + 4 + gid / 4); if (ki == k) regs[1] = tmp;
            tmp = __shfl_sync(mask, loaded[ki], tid * 8 + 2 + gid / 4); if (ki == k) regs[2] = tmp;
            tmp = __shfl_sync(mask, loaded[ki], tid * 8 + 6 + gid / 4); if (ki == k) regs[3] = tmp;
        }
    }
}

// Inverse shuffle for Matrix C/D (f32, 16x8).
template<Layout layout>
__device__ inline void mmaUnshuffleMatrixCD_f32_16x8(
    uint32_t* loaded,
    const uint32_t* regs,
    unsigned laneid,
    unsigned gid,
    unsigned tid)
{
    const uint32_t mask = 0xFFFFFFFF;
    if constexpr (layout == Layout::RowMajor)
    {
        // Forward: loaded from buffer[row*stride + side*4], row=laneid>>1, side=laneid&1.
        // regs[0] = s[k_base][src_base] where k_base=(tid&1)*2, src_base=(tid>>1)*2.
        // Fragment: regs[0]=C[gid][tid*2], regs[1]=C[gid][tid*2+1],
        //           regs[2]=C[gid+8][tid*2], regs[3]=C[gid+8][tid*2+1].
        // loaded[k] originally held 4 consecutive floats from one row-half.
        unsigned row = laneid >> 1;
        unsigned side = laneid & 1;
        // loaded[k] = C[row][side*4 + k] as float (one f32 per uint32)
        #pragma unroll
        for (int k = 0; k < 4; k++)
        {
            unsigned col = side * 4 + k;
            // C[row][col] is held by thread with gid_owner, tid_owner:
            //   gid_owner = row (if row<8) or row-8 (if row>=8)
            //   reg_idx = 0,1 (if row<8) or 2,3 (if row>=8)
            //   tid_owner such that tid*2=col or tid*2+1=col
            //   tid_owner = col/2, reg_offset = col%2
            unsigned gid_owner = (row < 8) ? row : (row - 8);
            unsigned row_reg_offset = (row < 8) ? 0 : 2;
            unsigned tid_owner = col / 2;
            unsigned col_reg_offset = col & 1;
            unsigned src_reg = row_reg_offset + col_reg_offset;
            unsigned src_lane = gid_owner * 4 + tid_owner;
            loaded[k] = __shfl_sync(mask, regs[src_reg], src_lane);
        }
    }
    else
    {
        // Column-major: loaded from buffer[col*stride + chunk*4], col=laneid>>2, chunk=laneid&3.
        unsigned col = laneid >> 2;
        unsigned chunk = laneid & 3;
        #pragma unroll
        for (int k = 0; k < 4; k++)
        {
            unsigned row = chunk * 4 + k;
            unsigned gid_owner = (row < 8) ? row : (row - 8);
            unsigned row_reg_offset = (row < 8) ? 0 : 2;
            unsigned tid_owner = col / 2;
            unsigned col_reg_offset = col & 1;
            unsigned src_reg = row_reg_offset + col_reg_offset;
            unsigned src_lane = gid_owner * 4 + tid_owner;
            loaded[k] = __shfl_sync(mask, regs[src_reg], src_lane);
        }
    }
}

template<Layout layout>
__device__ inline void mmaLoadMatrixCD_f32_16x8(
    uint32_t* regs,
    const float* buffer,
    int stride,
    unsigned laneid,
    unsigned gid,
    unsigned tid)
{
    if constexpr (layout == Layout::RowMajor)
    {
        unsigned row = laneid >> 1;
        unsigned side = laneid & 1;
        uint4 loaded_v =
            *reinterpret_cast<const uint4*>(&buffer[row * stride + side * 4]);
        uint32_t* loaded = reinterpret_cast<uint32_t*>(&loaded_v);
        mmaShuffleMatrixCD_f32_16x8<layout>(regs, loaded, laneid, gid, tid);
    }
    else
    {
        unsigned col = laneid >> 2;
        unsigned chunk = laneid & 3;
        uint4 loaded_v =
            *reinterpret_cast<const uint4*>(&buffer[col * stride + chunk * 4]);
        uint32_t* loaded = reinterpret_cast<uint32_t*>(&loaded_v);
        mmaShuffleMatrixCD_f32_16x8<layout>(regs, loaded, laneid, gid, tid);
    }
}

// ====================================================================================
// MMA m16n8k16 Matrix C/D Load (f16, 16x8)
//
// Row-major: each row = 8 halves = 16 bytes = 1 x 128 bits.
//   Thread t loads row t%16. 8 shuffles, select by tid.
//
// Column-major: each column = 16 halves = 32 bytes = 2 x 128 bits.
//   Thread t loads column (t%16)/2, side t%2.
//   16 shuffles + half-extraction (same pattern as column-major B row-major).
// ====================================================================================
// Shuffle-only for Matrix C/D (f16, 16x8).
template<Layout layout>
__device__ inline void mmaShuffleMatrixCD_f16_16x8(
    uint32_t* regs,
    const uint32_t* loaded,
    unsigned laneid,
    unsigned gid,
    unsigned tid)
{
    const uint32_t mask = 0xFFFFFFFF;

    if constexpr (layout == Layout::RowMajor)
    {
        uint32_t tmp;
        #pragma unroll
        for (int k = 0; k < 4; k++)
        {
            tmp = __shfl_sync(mask, loaded[k], gid);     if (tid == k) regs[0] = tmp;
            tmp = __shfl_sync(mask, loaded[k], gid + 8); if (tid == k) regs[1] = tmp;
        }
    }
    else
    {
        uint32_t s[4][4];
        #pragma unroll
        for (int ki = 0; ki < 4; ki++)
        {
            s[ki][0] = __shfl_sync(mask, loaded[ki], tid * 4);
            s[ki][1] = __shfl_sync(mask, loaded[ki], tid * 4 + 1);
            s[ki][2] = __shfl_sync(mask, loaded[ki], tid * 4 + 2);
            s[ki][3] = __shfl_sync(mask, loaded[ki], tid * 4 + 3);
        }

        unsigned k = gid >> 1;
        unsigned shift = (gid & 1) * 16;

        uint16_t h0 = (uint16_t)(s[k][0] >> shift);
        uint16_t h1 = (uint16_t)(s[k][2] >> shift);
        regs[0] = (uint32_t)h0 | ((uint32_t)h1 << 16);

        h0 = (uint16_t)(s[k][1] >> shift);
        h1 = (uint16_t)(s[k][3] >> shift);
        regs[1] = (uint32_t)h0 | ((uint32_t)h1 << 16);
    }
}

// Inverse shuffle for Matrix C/D (f16, 16x8).
template<Layout layout>
__device__ inline void mmaUnshuffleMatrixCD_f16_16x8(
    uint32_t* loaded,
    const uint32_t* regs,
    unsigned laneid,
    unsigned gid,
    unsigned tid)
{
    const uint32_t mask = 0xFFFFFFFF;
    if constexpr (layout == Layout::RowMajor)
    {
        // Forward: row = laneid & 15, loaded = 128 bits from buffer[row*stride] (8 halves).
        // Fragment CD f16: regs[0]={C[gid][tid*2],C[gid][tid*2+1]}, regs[1]={C[gid+8]...}
        unsigned row = laneid & 15;
        #pragma unroll
        for (int ki = 0; ki < 4; ki++)
        {
            // loaded[ki] held {C[row][2*ki], C[row][2*ki+1]} as f16x2.
            uint32_t val = 0;
            for (int bit = 0; bit < 2; bit++)
            {
                unsigned col = ki * 2 + bit;
                unsigned gid_owner = (row < 8) ? row : (row - 8);
                unsigned reg_idx = (row < 8) ? 0 : 1;
                unsigned tid_owner = col / 2;
                unsigned half_in_reg = col & 1;
                unsigned src_lane = gid_owner * 4 + tid_owner;
                uint32_t src_val = __shfl_sync(mask, regs[reg_idx], src_lane);
                uint16_t half_val = (uint16_t)(src_val >> (half_in_reg * 16));
                val |= ((uint32_t)half_val << (bit * 16));
            }
            loaded[ki] = val;
        }
    }
    else
    {
        // Column-major: col = (laneid&15)>>1, side = laneid&1
        unsigned col = (laneid & 15) >> 1;
        unsigned side = laneid & 1;
        #pragma unroll
        for (int ki = 0; ki < 4; ki++)
        {
            uint32_t val = 0;
            for (int bit = 0; bit < 2; bit++)
            {
                unsigned row = side * 8 + ki * 2 + bit;
                unsigned gid_owner = (row < 8) ? row : (row - 8);
                unsigned reg_idx = (row < 8) ? 0 : 1;
                unsigned tid_owner = col / 2;
                unsigned half_in_reg = col & 1;
                unsigned src_lane = gid_owner * 4 + tid_owner;
                uint32_t src_val = __shfl_sync(mask, regs[reg_idx], src_lane);
                uint16_t half_val = (uint16_t)(src_val >> (half_in_reg * 16));
                val |= ((uint32_t)half_val << (bit * 16));
            }
            loaded[ki] = val;
        }
    }
}

template<typename ElemT, Layout layout>
__device__ inline void mmaLoadMatrixCD_f16_16x8(
    uint32_t* regs,
    const ElemT* buffer,
    int stride,
    unsigned laneid,
    unsigned gid,
    unsigned tid)
{
    if constexpr (layout == Layout::RowMajor)
    {
        unsigned row = laneid & 15;
        uint4 loaded_v =
            *reinterpret_cast<const uint4*>(&buffer[row * stride]);
        uint32_t* loaded = reinterpret_cast<uint32_t*>(&loaded_v);
        mmaShuffleMatrixCD_f16_16x8<layout>(regs, loaded, laneid, gid, tid);
    }
    else
    {
        unsigned col = (laneid & 15) >> 1;
        unsigned side = laneid & 1;
        uint4 loaded_v =
            *reinterpret_cast<const uint4*>(&buffer[col * stride + side * 8]);
        uint32_t* loaded = reinterpret_cast<uint32_t*>(&loaded_v);
        mmaShuffleMatrixCD_f16_16x8<layout>(regs, loaded, laneid, gid, tid);
    }
}

template<typename ElemT, int M, int N, int K, MatrixUse use, Layout layout>
__device__ inline void mmaLoad(uint32_t* regs, const void* ptr, int stride)
{
    const ElemT* buffer = static_cast<const ElemT*>(ptr);

    if constexpr (M == 16 && N == 8 && K == 16)
    {
        unsigned laneid;
        asm("mov.u32 %0, %%laneid;" : "=r"(laneid));
        unsigned gid = laneid >> 2;
        unsigned tid = laneid & 3;

        if constexpr (use == MatrixUse::MatrixA && sizeof(ElemT) == 2)
        {
            mmaLoadMatrixA_16x16<ElemT, layout>(regs, buffer, stride, laneid, gid, tid);
        }
        else if constexpr (use == MatrixUse::MatrixB && sizeof(ElemT) == 2)
        {
            mmaLoadMatrixB_16x8<ElemT, layout>(regs, buffer, stride, laneid, gid, tid);
        }
        else if constexpr (
            (use == MatrixUse::MatrixC || use == MatrixUse::MatrixD) && sizeof(ElemT) == 4)
        {
            mmaLoadMatrixCD_f32_16x8<layout>(
                regs, reinterpret_cast<const float*>(buffer), stride, laneid, gid, tid);
        }
        else if constexpr (
            (use == MatrixUse::MatrixC || use == MatrixUse::MatrixD) && sizeof(ElemT) == 2)
        {
            mmaLoadMatrixCD_f16_16x8<ElemT, layout>(regs, buffer, stride, laneid, gid, tid);
        }
    }
}

// ====================================================================================
// MMA m16n8k16 Store (C/D matrix, 16x8)
//
// Row-major: reverse shuffle to collect contiguous row-halves, then 128-bit write.
//   Writer thread t writes row t/2, side t%2.
//   8 shuffles (f32) or similar for f16.
//
// Column-major: addresses are stride-apart (scattered), use individual element stores.
// ====================================================================================

template<typename ElemT, int M, int N, int K, Layout layout>
__device__ inline void mmaStore(void* ptr, const uint32_t* regs, int stride)
{
    ElemT* buffer = static_cast<ElemT*>(ptr);

    if constexpr (M == 16 && N == 8 && K == 16)
    {
        unsigned laneid;
        asm("mov.u32 %0, %%laneid;" : "=r"(laneid));
        unsigned gid = laneid >> 2;
        unsigned tid = laneid & 3;

        if constexpr (sizeof(ElemT) == 4)
        {
            if constexpr (layout == Layout::RowMajor)
            {
                // Row-major f32: reverse shuffle to collect contiguous row-half data.
                // Writer thread t writes row t/2, side t%2 (left=cols 0-3, right=4-7).
                // Each 128-bit write covers 4 consecutive floats in one row-half.
                unsigned write_row = laneid >> 1;
                unsigned write_side = laneid & 1;
                unsigned source_gid = (write_row < 8) ? write_row : (write_row - 8);
                unsigned src0 = source_gid * 4 + write_side * 2;
                unsigned src1 = src0 + 1;

                const uint32_t mask = 0xFFFFFFFF;
                uint32_t r0_s0 = __shfl_sync(mask, regs[0], src0);
                uint32_t r1_s0 = __shfl_sync(mask, regs[1], src0);
                uint32_t r0_s1 = __shfl_sync(mask, regs[0], src1);
                uint32_t r1_s1 = __shfl_sync(mask, regs[1], src1);
                uint32_t r2_s0 = __shfl_sync(mask, regs[2], src0);
                uint32_t r3_s0 = __shfl_sync(mask, regs[3], src0);
                uint32_t r2_s1 = __shfl_sync(mask, regs[2], src1);
                uint32_t r3_s1 = __shfl_sync(mask, regs[3], src1);

                uint4 out_v;
                uint32_t* out = reinterpret_cast<uint32_t*>(&out_v);
                if (write_row < 8)
                {
                    out[0] = r0_s0; out[1] = r1_s0;
                    out[2] = r0_s1; out[3] = r1_s1;
                }
                else
                {
                    out[0] = r2_s0; out[1] = r3_s0;
                    out[2] = r2_s1; out[3] = r3_s1;
                }

                *reinterpret_cast<uint4*>(&buffer[write_row * stride + write_side * 4]) = out_v;
            }
            else
            {
                // Column-major f32: each column = 16 floats = 64 bytes = 4 x uint4.
                // 8 cols x 4 chunks = 32 writers. Thread t writes col t/4, chunk t%4.
                // All 4 stored elements need the same register index from different gids.
                // 16 shuffles (4 regs x 4 sources) + selection by reg_idx.
                unsigned write_col = laneid >> 2;
                unsigned write_chunk = laneid & 3;
                unsigned source_tid_val = write_col >> 1;
                unsigned gid_base = (write_chunk & 1) * 4;
                unsigned reg_idx = (write_chunk < 2) ? (write_col & 1) : (2 + (write_col & 1));

                const uint32_t mask = 0xFFFFFFFF;
                uint32_t s[4][4];
                #pragma unroll
                for (int r = 0; r < 4; r++)
                {
                    s[r][0] = __shfl_sync(mask, regs[r], (gid_base + 0) * 4 + source_tid_val);
                    s[r][1] = __shfl_sync(mask, regs[r], (gid_base + 1) * 4 + source_tid_val);
                    s[r][2] = __shfl_sync(mask, regs[r], (gid_base + 2) * 4 + source_tid_val);
                    s[r][3] = __shfl_sync(mask, regs[r], (gid_base + 3) * 4 + source_tid_val);
                }

                uint4 out_v;
                uint32_t* out = reinterpret_cast<uint32_t*>(&out_v);
                out[0] = s[reg_idx][0];
                out[1] = s[reg_idx][1];
                out[2] = s[reg_idx][2];
                out[3] = s[reg_idx][3];

                *reinterpret_cast<uint4*>(&buffer[write_col * stride + write_chunk * 4]) = out_v;
            }
        }
        else if constexpr (sizeof(ElemT) == 2)
        {
            if constexpr (layout == Layout::RowMajor)
            {
                // Row-major f16: reverse shuffle to collect contiguous row data.
                // Each row = 8 halves = 16 bytes = 128 bits.
                // Writer thread t writes row t%16.
                unsigned write_row = laneid & 15;

                const uint32_t mask = 0xFFFFFFFF;
                // Collect reg0 (row gid) and reg1 (row gid+8) from all 4 tids
                uint32_t s[4][2];
                #pragma unroll
                for (int k = 0; k < 4; k++)
                {
                    s[k][0] = __shfl_sync(mask, regs[0], write_row * 4 + k);
                    s[k][1] = __shfl_sync(mask, regs[1], write_row * 4 + k);
                }

                // Select top or bottom registers based on write_row
                uint4 out_v;
                uint32_t* out = reinterpret_cast<uint32_t*>(&out_v);
                if (write_row < 8)
                {
                    out[0] = s[0][0]; out[1] = s[1][0];
                    out[2] = s[2][0]; out[3] = s[3][0];
                }
                else
                {
                    out[0] = s[0][1]; out[1] = s[1][1];
                    out[2] = s[2][1]; out[3] = s[3][1];
                }

                *reinterpret_cast<uint4*>(&buffer[write_row * stride]) = out_v;
            }
            else
            {
                // Column-major f16: each column = 16 halves = 32 bytes = 2 x uint4.
                // 8 cols x 2 sides = 16 writers. Thread t writes col (t%16)/2, side t%2.
                // stored[k] = {C[side*8+2k][col], C[side*8+2k+1][col]} from 2 gids.
                // 16 shuffles (2 regs x 8 sources) + half extraction + packing.
                unsigned write_col = (laneid & 15) >> 1;
                unsigned write_side = laneid & 1;
                unsigned source_tid_val = write_col >> 1;
                unsigned col_shift = (write_col & 1) * 16;

                const uint32_t mask = 0xFFFFFFFF;
                uint32_t from_r0[8], from_r1[8];
                #pragma unroll
                for (int k = 0; k < 4; k++)
                {
                    unsigned src_even = (2 * k) * 4 + source_tid_val;
                    unsigned src_odd = (2 * k + 1) * 4 + source_tid_val;
                    from_r0[2 * k] = __shfl_sync(mask, regs[0], src_even);
                    from_r0[2 * k + 1] = __shfl_sync(mask, regs[0], src_odd);
                    from_r1[2 * k] = __shfl_sync(mask, regs[1], src_even);
                    from_r1[2 * k + 1] = __shfl_sync(mask, regs[1], src_odd);
                }

                uint4 out_v;
                uint32_t* out = reinterpret_cast<uint32_t*>(&out_v);
                #pragma unroll
                for (int k = 0; k < 4; k++)
                {
                    uint32_t val_even = (write_side == 0) ? from_r0[2 * k] : from_r1[2 * k];
                    uint32_t val_odd =
                        (write_side == 0) ? from_r0[2 * k + 1] : from_r1[2 * k + 1];
                    uint16_t h0 = (uint16_t)(val_even >> col_shift);
                    uint16_t h1 = (uint16_t)(val_odd >> col_shift);
                    out[k] = (uint32_t)h0 | ((uint32_t)h1 << 16);
                }

                *reinterpret_cast<uint4*>(&buffer[write_col * stride + write_side * 8]) = out_v;
            }
        }
    }
}

// Helper to get M, N, K from ShapeCombination
template<ShapeCombination shape>
struct ShapeToMNK;
template<>
struct ShapeToMNK<ShapeCombination::m16n16k16>
{
    static constexpr int M = 16, N = 16, K = 16;
};
template<>
struct ShapeToMNK<ShapeCombination::m8n32k16>
{
    static constexpr int M = 8, N = 32, K = 16;
};
template<>
struct ShapeToMNK<ShapeCombination::m32n8k16>
{
    static constexpr int M = 32, N = 8, K = 16;
};
template<>
struct ShapeToMNK<ShapeCombination::m16n8k16>
{
    static constexpr int M = 16, N = 8, K = 16;
};

template<typename T>
inline unsigned __device__ Pack32Helper(T value);

#if SLANG_CUDA_ENABLE_HALF
template<>
inline unsigned __device__ Pack32Helper<half>(half value)
{
    return __half_as_ushort(value) | (__half_as_ushort(value) << 16);
};
#endif

template<>
inline unsigned __device__ Pack32Helper<float>(float value)
{
    return __float_as_uint(value);
};

template<>
inline unsigned __device__ Pack32Helper<int>(int value)
{
    return (unsigned)value;
};
template<>
inline unsigned __device__ Pack32Helper<char>(char value)
{
    return value << 24 | value << 16 | value << 8 | value;
};
template<>
inline unsigned __device__ Pack32Helper<unsigned char>(unsigned char value)
{
    return value << 24 | value << 16 | value << 8 | value;
};


// ====================================================================================
// FromLocalRow / ToLocalRow shuffle helpers
//
// Each thread provides one full row of the matrix. 16 threads form one matrix
// (M=16 rows). With 32 threads per warp, matIndex (0 or 1) selects which
// group of 16 threads contributes.
// ====================================================================================

// MatrixA 16x16 f16: localRow has K=16 halves packed as 8 uint32.
// Fragment: regs[0..3] = 4 x f16x2.
__device__ inline void mmaFromRowMatrixA_16x16(
    uint32_t* regs,
    const void* localRow,
    int matIndex,
    unsigned gid,
    unsigned tid)
{
    uint32_t packedRow[8];
    memcpy(packedRow, localRow, 32);

    unsigned base = matIndex * 16;
    const uint32_t mask = 0xFFFFFFFF;
    regs[0] = 0; regs[1] = 0; regs[2] = 0; regs[3] = 0;
    #pragma unroll
    for (int j = 0; j < 8; j++)
    {
        uint32_t v0 = __shfl_sync(mask, packedRow[j], gid + base);
        uint32_t v1 = __shfl_sync(mask, packedRow[j], gid + 8 + base);
        if (tid == j)     { regs[0] = v0; regs[1] = v1; }
        if (tid + 4 == j) { regs[2] = v0; regs[3] = v1; }
    }
}

__device__ inline void mmaToRowMatrixA_16x16(
    void* localRow,
    const uint32_t* regs,
    int matIndex,
    unsigned laneid)
{
    unsigned row_in_mat = laneid % 16;
    unsigned adj = (row_in_mat < 8) ? row_in_mat : (row_in_mat - 8);
    unsigned rlo = (row_in_mat < 8) ? 0 : 1;
    unsigned rhi = (row_in_mat < 8) ? 2 : 3;

    const uint32_t mask = 0xFFFFFFFF;
    uint32_t packedRow[8];
    #pragma unroll
    for (int j = 0; j < 4; j++)
        packedRow[j] = __shfl_sync(mask, regs[rlo], adj * 4 + j);
    #pragma unroll
    for (int j = 0; j < 4; j++)
        packedRow[4 + j] = __shfl_sync(mask, regs[rhi], adj * 4 + j);

    memcpy(localRow, packedRow, 32);
}

// MatrixB 16x8 f16: localRow has N=8 halves packed as 4 uint32.
// Fragment: regs[0..1] = 2 x f16x2 (vertical pairs).
__device__ inline void mmaFromRowMatrixB_16x8(
    uint32_t* regs,
    const void* localRow,
    int matIndex,
    unsigned gid,
    unsigned tid)
{
    uint32_t packedRow[4];
    memcpy(packedRow, localRow, 16);

    unsigned base = matIndex * 16;
    const uint32_t mask = 0xFFFFFFFF;
    unsigned k = gid >> 1;
    unsigned shift = (gid & 1) * 16;

    uint32_t r0 = 0, r1 = 0;
    #pragma unroll
    for (int ki = 0; ki < 4; ki++)
    {
        uint32_t s0 = __shfl_sync(mask, packedRow[ki], tid * 2 + base);
        uint32_t s1 = __shfl_sync(mask, packedRow[ki], tid * 2 + 1 + base);
        uint32_t s2 = __shfl_sync(mask, packedRow[ki], tid * 2 + 8 + base);
        uint32_t s3 = __shfl_sync(mask, packedRow[ki], tid * 2 + 9 + base);

        if (k == ki)
        {
            uint16_t h0 = (uint16_t)(s0 >> shift);
            uint16_t h1 = (uint16_t)(s1 >> shift);
            r0 = (uint32_t)h0 | ((uint32_t)h1 << 16);

            h0 = (uint16_t)(s2 >> shift);
            h1 = (uint16_t)(s3 >> shift);
            r1 = (uint32_t)h0 | ((uint32_t)h1 << 16);
        }
    }
    regs[0] = r0;
    regs[1] = r1;
}

__device__ inline void mmaToRowMatrixB_16x8(
    void* localRow,
    const uint32_t* regs,
    int matIndex,
    unsigned laneid)
{
    unsigned row_in_mat = laneid % 16;
    unsigned tid_src = (row_in_mat < 8) ? (row_in_mat / 2) : ((row_in_mat - 8) / 2);
    unsigned reg_sel = (row_in_mat < 8) ? 0 : 1;
    unsigned half_shift = (row_in_mat & 1) * 16;

    const uint32_t mask = 0xFFFFFFFF;
    uint32_t packedRow[4];
    #pragma unroll
    for (int j = 0; j < 4; j++)
    {
        unsigned src0 = (2 * j) * 4 + tid_src;
        unsigned src1 = (2 * j + 1) * 4 + tid_src;
        uint32_t v0 = __shfl_sync(mask, regs[reg_sel], src0);
        uint32_t v1 = __shfl_sync(mask, regs[reg_sel], src1);
        uint16_t h0 = (uint16_t)(v0 >> half_shift);
        uint16_t h1 = (uint16_t)(v1 >> half_shift);
        packedRow[j] = (uint32_t)h0 | ((uint32_t)h1 << 16);
    }
    memcpy(localRow, packedRow, 16);
}

// MatrixC/D f32 16x8: localRow has N=8 floats = 8 uint32.
// Fragment: regs[0..3] = 4 x f32.
__device__ inline void mmaFromRowMatrixCD_f32_16x8(
    uint32_t* regs,
    const void* localRow,
    int matIndex,
    unsigned gid,
    unsigned tid)
{
    uint32_t rowU32[8];
    memcpy(rowU32, localRow, 32);

    unsigned base = matIndex * 16;
    const uint32_t mask = 0xFFFFFFFF;
    regs[0] = 0; regs[1] = 0; regs[2] = 0; regs[3] = 0;
    #pragma unroll
    for (int j = 0; j < 8; j++)
    {
        uint32_t v0 = __shfl_sync(mask, rowU32[j], gid + base);
        uint32_t v1 = __shfl_sync(mask, rowU32[j], gid + 8 + base);
        if (tid * 2 == j)     { regs[0] = v0; regs[2] = v1; }
        if (tid * 2 + 1 == j) { regs[1] = v0; regs[3] = v1; }
    }
}

__device__ inline void mmaToRowMatrixCD_f32_16x8(
    void* localRow,
    const uint32_t* regs,
    int matIndex,
    unsigned laneid)
{
    unsigned row_in_mat = laneid % 16;
    unsigned gid_src = (row_in_mat < 8) ? row_in_mat : (row_in_mat - 8);
    unsigned reg_base = (row_in_mat < 8) ? 0 : 2;

    const uint32_t mask = 0xFFFFFFFF;
    uint32_t rowU32[8];
    #pragma unroll
    for (int col = 0; col < 8; col++)
    {
        unsigned tid_src = col / 2;
        unsigned src_reg = reg_base + (col & 1);
        unsigned src_lane = gid_src * 4 + tid_src;
        rowU32[col] = __shfl_sync(mask, regs[src_reg], src_lane);
    }
    memcpy(localRow, rowU32, 32);
}

// MatrixC/D f16 16x8: localRow has N=8 halves packed as 4 uint32.
// Fragment: regs[0..1] = 2 x f16x2.
__device__ inline void mmaFromRowMatrixCD_f16_16x8(
    uint32_t* regs,
    const void* localRow,
    int matIndex,
    unsigned gid,
    unsigned tid)
{
    uint32_t packedRow[4];
    memcpy(packedRow, localRow, 16);

    unsigned base = matIndex * 16;
    const uint32_t mask = 0xFFFFFFFF;
    regs[0] = 0; regs[1] = 0;
    #pragma unroll
    for (int j = 0; j < 4; j++)
    {
        uint32_t v0 = __shfl_sync(mask, packedRow[j], gid + base);
        uint32_t v1 = __shfl_sync(mask, packedRow[j], gid + 8 + base);
        if (tid == j) { regs[0] = v0; regs[1] = v1; }
    }
}

__device__ inline void mmaToRowMatrixCD_f16_16x8(
    void* localRow,
    const uint32_t* regs,
    int matIndex,
    unsigned laneid)
{
    unsigned row_in_mat = laneid % 16;
    unsigned gid_src = (row_in_mat < 8) ? row_in_mat : (row_in_mat - 8);
    unsigned reg_sel = (row_in_mat < 8) ? 0 : 1;

    const uint32_t mask = 0xFFFFFFFF;
    uint32_t packedRow[4];
    #pragma unroll
    for (int j = 0; j < 4; j++)
    {
        unsigned src_lane = gid_src * 4 + j;
        packedRow[j] = __shfl_sync(mask, regs[reg_sel], src_lane);
    }
    memcpy(localRow, packedRow, 16);
}

// ====================================================================================
// FromLocalColumn / ToLocalColumn shuffle helpers
//
// Each thread provides one full column of the matrix (M elements).
// The number of threads per matrix = N (column count of the CoopMat).
// matIndex selects which group of N threads contributes.
// ====================================================================================

// MatrixA 16x16 f16: localCol has 16 halves packed as 8 uint32.
// N=16 columns, so 16 threads per matrix. matIndex selects group of 16.
__device__ inline void mmaFromColMatrixA_16x16(
    uint32_t* regs,
    const void* localCol,
    int matIndex,
    unsigned gid,
    unsigned tid)
{
    uint32_t packedCol[8];
    memcpy(packedCol, localCol, 32);

    unsigned base = matIndex * 16;
    const uint32_t mask = 0xFFFFFFFF;
    unsigned k = gid >> 1;
    unsigned shift = (gid & 1) * 16;

    uint32_t s[2][4];
    #pragma unroll
    for (int ko = 0; ko < 2; ko++)
    {
        uint32_t val = 0;
        #pragma unroll
        for (int p = 0; p < 4; p++)
            if (k == p) val = packedCol[p + ko * 4];

        s[ko][0] = __shfl_sync(mask, val, tid * 2 + base);
        s[ko][1] = __shfl_sync(mask, val, tid * 2 + 1 + base);
        s[ko][2] = __shfl_sync(mask, val, tid * 2 + 8 + base);
        s[ko][3] = __shfl_sync(mask, val, tid * 2 + 9 + base);
    }

    uint16_t h0, h1;

    h0 = (uint16_t)(s[0][0] >> shift);
    h1 = (uint16_t)(s[0][1] >> shift);
    regs[0] = (uint32_t)h0 | ((uint32_t)h1 << 16);

    h0 = (uint16_t)(s[1][0] >> shift);
    h1 = (uint16_t)(s[1][1] >> shift);
    regs[1] = (uint32_t)h0 | ((uint32_t)h1 << 16);

    h0 = (uint16_t)(s[0][2] >> shift);
    h1 = (uint16_t)(s[0][3] >> shift);
    regs[2] = (uint32_t)h0 | ((uint32_t)h1 << 16);

    h0 = (uint16_t)(s[1][2] >> shift);
    h1 = (uint16_t)(s[1][3] >> shift);
    regs[3] = (uint32_t)h0 | ((uint32_t)h1 << 16);
}

__device__ inline void mmaToColMatrixA_16x16(
    void* localCol,
    const uint32_t* regs,
    int matIndex,
    unsigned laneid)
{
    // Thread L owns column (L % 16). A[row][col] is in the fragment as:
    //   gid_src = (row<8) ? row : row-8,  tid_src = col/2
    //   reg = (row<8 ? 0 : 1) + (col>=8 ? 2 : 0),  half = col within pair
    unsigned col_in_mat = laneid % 16;
    unsigned tid_src = (col_in_mat < 8) ? (col_in_mat / 2) : ((col_in_mat - 8) / 2);
    unsigned col_reg_offset = (col_in_mat < 8) ? 0 : 2;
    unsigned half_in_reg = (col_in_mat < 8) ? (col_in_mat & 1) : ((col_in_mat - 8) & 1);
    unsigned col_shift = half_in_reg * 16;

    const uint32_t mask = 0xFFFFFFFF;
    uint32_t packedCol[8];
    #pragma unroll
    for (int row_pair = 0; row_pair < 8; row_pair++)
    {
        unsigned src_lane = row_pair * 4 + tid_src;
        unsigned src_reg_lo = 0 + col_reg_offset;
        unsigned src_reg_hi = 1 + col_reg_offset;
        uint32_t val_lo = __shfl_sync(mask, regs[src_reg_lo], src_lane);
        uint32_t val_hi = __shfl_sync(mask, regs[src_reg_hi], src_lane);
        uint16_t h_lo = (uint16_t)(val_lo >> col_shift);
        uint16_t h_hi = (uint16_t)(val_hi >> col_shift);
        packedCol[row_pair] = (uint32_t)h_lo | ((uint32_t)h_hi << 16);
    }
    memcpy(localCol, packedCol, 32);
}

// MatrixB 16x8 f16: localCol has M=16 halves packed as 8 uint32.
// N=8 columns, so 8 threads per matrix. matIndex selects group of 8.
// Fragment layout: reg[0] = packed(B[tid*2][gid], B[tid*2+1][gid]),
//                  reg[1] = packed(B[tid*2+8][gid], B[tid*2+9][gid])
// Column gid is held by thread (gid + base). The column data packs rows as pairs.
__device__ inline void mmaFromColMatrixB_16x8(
    uint32_t* regs,
    const void* localCol,
    int matIndex,
    unsigned gid,
    unsigned tid)
{
    const uint32_t* packedCol = reinterpret_cast<const uint32_t*>(localCol);

    unsigned base = matIndex * 8;
    const uint32_t mask = 0xFFFFFFFF;

    uint32_t tmp;
    #pragma unroll
    for (int t = 0; t < 4; t++)
    {
        tmp = __shfl_sync(mask, packedCol[t], gid + base);     if (tid == t) regs[0] = tmp;
        tmp = __shfl_sync(mask, packedCol[t + 4], gid + base); if (tid == t) regs[1] = tmp;
    }
}

// MatrixB 16x16 f16: combined lo+hi in one call. regs[0:1] = lo, regs[2:3] = hi.
__device__ inline void mmaFromColMatrixB_16x16(
    uint32_t* regs,
    const void* localCol,
    int matIndex,
    unsigned gid,
    unsigned tid)
{
    const uint32_t* packedCol = reinterpret_cast<const uint32_t*>(localCol);

    unsigned base = matIndex * 16;
    const uint32_t mask = 0xFFFFFFFF;

    uint32_t tmp;
    #pragma unroll
    for (int t = 0; t < 4; t++)
    {
        tmp = __shfl_sync(mask, packedCol[t],     gid + base);     if (tid == t) regs[0] = tmp;
        tmp = __shfl_sync(mask, packedCol[t + 4], gid + base);     if (tid == t) regs[1] = tmp;
        tmp = __shfl_sync(mask, packedCol[t],     gid + base + 8); if (tid == t) regs[2] = tmp;
        tmp = __shfl_sync(mask, packedCol[t + 4], gid + base + 8); if (tid == t) regs[3] = tmp;
    }
}

__device__ inline void mmaToColMatrixB_16x8(
    void* localCol,
    const uint32_t* regs,
    int matIndex,
    unsigned laneid)
{
    // Thread L owns column (L % 8). B[row][col] is in fragment as:
    //   gid_src = col, tid_src = (row<8 ? row/2 : (row-8)/2)
    //   reg_sel = (row<8) ? 0 : 1, half = row & 1
    unsigned col_in_mat = laneid % 8;
    const uint32_t mask = 0xFFFFFFFF;

    uint32_t packedCol[8];
    #pragma unroll
    for (int row_pair = 0; row_pair < 8; row_pair++)
    {
        unsigned row_lo = (row_pair < 4) ? (row_pair * 2) : (row_pair * 2);
        unsigned tid_src = (row_pair < 4) ? row_pair : (row_pair - 4);
        unsigned reg_sel = (row_pair < 4) ? 0 : 1;
        unsigned src_lane = col_in_mat * 4 + tid_src;
        packedCol[row_pair] = __shfl_sync(mask, regs[reg_sel], src_lane);
    }
    memcpy(localCol, packedCol, 32);
}

// MatrixC/D f32 16x8: localCol has M=16 floats = 16 uint32.
// N=8 columns, so 8 threads per matrix.
// Fragment: regs[0]=C[gid][tid*2], regs[1]=C[gid][tid*2+1],
//           regs[2]=C[gid+8][tid*2], regs[3]=C[gid+8][tid*2+1]
__device__ inline void mmaFromColMatrixCD_f32_16x8(
    uint32_t* regs,
    const void* localCol,
    int matIndex,
    unsigned gid,
    unsigned tid)
{
    uint32_t colU32[16];
    memcpy(colU32, localCol, 64);

    unsigned base = matIndex * 8;
    const uint32_t mask = 0xFFFFFFFF;

    regs[0] = 0; regs[1] = 0; regs[2] = 0; regs[3] = 0;
    #pragma unroll
    for (int g = 0; g < 8; g++)
    {
        uint32_t v0 = __shfl_sync(mask, colU32[g], tid * 2 + base);
        uint32_t v1 = __shfl_sync(mask, colU32[g], tid * 2 + 1 + base);
        uint32_t v0h = __shfl_sync(mask, colU32[g + 8], tid * 2 + base);
        uint32_t v1h = __shfl_sync(mask, colU32[g + 8], tid * 2 + 1 + base);
        if (gid == g) { regs[0] = v0; regs[1] = v1; regs[2] = v0h; regs[3] = v1h; }
    }
}

__device__ inline void mmaToColMatrixCD_f32_16x8(
    void* localCol,
    const uint32_t* regs,
    int matIndex,
    unsigned laneid)
{
    // Thread L owns column (L % 8).
    // C[row][col]: gid_src=(row<8)?row:row-8, tid_src=col/2, reg=(row<8?0:2)+(col&1)
    unsigned col_in_mat = laneid % 8;
    unsigned tid_src = col_in_mat / 2;
    unsigned col_offset = col_in_mat & 1;
    unsigned src_reg_lo = col_offset;
    unsigned src_reg_hi = 2 + col_offset;

    const uint32_t mask = 0xFFFFFFFF;
    uint32_t colU32[16];
    #pragma unroll
    for (int g = 0; g < 8; g++)
    {
        unsigned src_lane = g * 4 + tid_src;
        colU32[g] = __shfl_sync(mask, regs[src_reg_lo], src_lane);
        colU32[g + 8] = __shfl_sync(mask, regs[src_reg_hi], src_lane);
    }
    memcpy(localCol, colU32, 64);
}

// MatrixC/D f16 16x8: localCol has M=16 halves packed as 8 uint32.
// N=8 columns, so 8 threads per matrix.
// Fragment: regs[0]=packed(C[gid][tid*2],C[gid][tid*2+1]),
//           regs[1]=packed(C[gid+8][tid*2],C[gid+8][tid*2+1])
__device__ inline void mmaFromColMatrixCD_f16_16x8(
    uint32_t* regs,
    const void* localCol,
    int matIndex,
    unsigned gid,
    unsigned tid)
{
    uint32_t packedCol[8];
    memcpy(packedCol, localCol, 32);

    unsigned base = matIndex * 8;
    const uint32_t mask = 0xFFFFFFFF;
    unsigned k = gid >> 1;
    unsigned shift = (gid & 1) * 16;
    unsigned k2 = (gid + 8) >> 1;
    unsigned shift2 = ((gid + 8) & 1) * 16;

    regs[0] = 0; regs[1] = 0;
    #pragma unroll
    for (int p = 0; p < 8; p++)
    {
        uint32_t v0 = __shfl_sync(mask, packedCol[p], tid * 2 + base);
        uint32_t v1 = __shfl_sync(mask, packedCol[p], tid * 2 + 1 + base);

        if (k == p)
        {
            uint16_t h0 = (uint16_t)(v0 >> shift);
            uint16_t h1 = (uint16_t)(v1 >> shift);
            regs[0] = (uint32_t)h0 | ((uint32_t)h1 << 16);
        }
        if (k2 == p)
        {
            uint16_t h0 = (uint16_t)(v0 >> shift2);
            uint16_t h1 = (uint16_t)(v1 >> shift2);
            regs[1] = (uint32_t)h0 | ((uint32_t)h1 << 16);
        }
    }
}

__device__ inline void mmaToColMatrixCD_f16_16x8(
    void* localCol,
    const uint32_t* regs,
    int matIndex,
    unsigned laneid)
{
    // C[row][col]: fragment has regs[0]=packed(C[gid][tid*2], C[gid][tid*2+1]),
    //                           regs[1]=packed(C[gid+8][tid*2], C[gid+8][tid*2+1])
    // For col_in_mat: tid_src = col_in_mat/2, half_pos = col_in_mat & 1
    // C[row][col_in_mat] at thread (gid_src*4 + tid_src), reg (row<8?0:1), half half_pos
    unsigned col_in_mat = laneid % 8;
    unsigned tid_src = col_in_mat / 2;
    unsigned col_shift = (col_in_mat & 1) * 16;

    const uint32_t mask = 0xFFFFFFFF;
    uint32_t packedCol[8];
    #pragma unroll
    for (int row_pair = 0; row_pair < 8; row_pair++)
    {
        unsigned row0 = 2 * row_pair;
        unsigned row1 = row0 + 1;

        unsigned gid0 = (row0 < 8) ? row0 : (row0 - 8);
        unsigned reg0 = (row0 < 8) ? 0 : 1;
        uint32_t v0 = __shfl_sync(mask, regs[reg0], gid0 * 4 + tid_src);
        uint16_t elem0 = (uint16_t)(v0 >> col_shift);

        unsigned gid1 = (row1 < 8) ? row1 : (row1 - 8);
        unsigned reg1 = (row1 < 8) ? 0 : 1;
        uint32_t v1 = __shfl_sync(mask, regs[reg1], gid1 * 4 + tid_src);
        uint16_t elem1 = (uint16_t)(v1 >> col_shift);

        packedCol[row_pair] = (uint32_t)elem0 | ((uint32_t)elem1 << 16);
    }
    memcpy(localCol, packedCol, 32);
}

// MatrixC/D 16x16 f16: combined lo+hi extraction. regs[0:1] = lo, regs[2:3] = hi.
__device__ inline void mmaToColMatrixCD_f16_16x16(
    void* localCol,
    const uint32_t* regs,
    int matIndex,
    unsigned laneid)
{
    mmaToColMatrixCD_f16_16x8(localCol, regs,     matIndex * 2,     laneid);
    mmaToColMatrixCD_f16_16x8(localCol, regs + 2, matIndex * 2 + 1, laneid);
}

// MatrixB 16x16 FromLocalRow: combined lo+hi in one pass. regs[0:1] = lo, regs[2:3] = hi.
__device__ inline void mmaFromRowMatrixB_16x16(
    uint32_t* regs,
    const void* localRow,
    int matIndex,
    unsigned gid,
    unsigned tid)
{
    uint4 packedRow = *reinterpret_cast<const uint4*>(localRow);

    unsigned baseLo = matIndex * 32;
    unsigned baseHi = baseLo + 16;
    const uint32_t mask = 0xFFFFFFFF;
    unsigned k = gid >> 1;
    unsigned shift = (gid & 1) * 16;

    uint32_t r0 = 0, r1 = 0, r2 = 0, r3 = 0;
    const uint32_t* pr = reinterpret_cast<const uint32_t*>(&packedRow);
    #pragma unroll
    for (int ki = 0; ki < 4; ki++)
    {
        uint32_t sLo0 = __shfl_sync(mask, pr[ki], tid * 2 + baseLo);
        uint32_t sLo1 = __shfl_sync(mask, pr[ki], tid * 2 + 1 + baseLo);
        uint32_t sLo2 = __shfl_sync(mask, pr[ki], tid * 2 + 8 + baseLo);
        uint32_t sLo3 = __shfl_sync(mask, pr[ki], tid * 2 + 9 + baseLo);
        uint32_t sHi0 = __shfl_sync(mask, pr[ki], tid * 2 + baseHi);
        uint32_t sHi1 = __shfl_sync(mask, pr[ki], tid * 2 + 1 + baseHi);
        uint32_t sHi2 = __shfl_sync(mask, pr[ki], tid * 2 + 8 + baseHi);
        uint32_t sHi3 = __shfl_sync(mask, pr[ki], tid * 2 + 9 + baseHi);

        if (k == ki)
        {
            uint16_t h0, h1;
            h0 = (uint16_t)(sLo0 >> shift); h1 = (uint16_t)(sLo1 >> shift);
            r0 = (uint32_t)h0 | ((uint32_t)h1 << 16);
            h0 = (uint16_t)(sLo2 >> shift); h1 = (uint16_t)(sLo3 >> shift);
            r1 = (uint32_t)h0 | ((uint32_t)h1 << 16);
            h0 = (uint16_t)(sHi0 >> shift); h1 = (uint16_t)(sHi1 >> shift);
            r2 = (uint32_t)h0 | ((uint32_t)h1 << 16);
            h0 = (uint16_t)(sHi2 >> shift); h1 = (uint16_t)(sHi3 >> shift);
            r3 = (uint32_t)h0 | ((uint32_t)h1 << 16);
        }
    }
    *reinterpret_cast<uint4*>(regs) = make_uint4(r0, r1, r2, r3);
}

// MatrixC/D 16x16 f16 ToLocalRow: extract from lo half (all rows identical in reduction use).
__device__ inline void mmaToRowMatrixCD_f16_16x16(
    void* localRow,
    const uint32_t* regs,
    int matIndex,
    unsigned laneid)
{
    mmaToRowMatrixCD_f16_16x8(localRow, regs, matIndex * 2, laneid);
}

// MatrixC/D 16x16 f32 ToLocalColumn: combined lo+hi. regs[0:3] = lo, regs[4:7] = hi.
__device__ inline void mmaToColMatrixCD_f32_16x16(
    void* localCol,
    const uint32_t* regs,
    int matIndex,
    unsigned laneid)
{
    mmaToColMatrixCD_f32_16x8(localCol, regs,     matIndex * 2,     laneid);
    mmaToColMatrixCD_f32_16x8(localCol, regs + 4, matIndex * 2 + 1, laneid);
}

// MatrixC/D 16x16 f32 ToLocalRow: extract from lo half.
__device__ inline void mmaToRowMatrixCD_f32_16x16(
    void* localRow,
    const uint32_t* regs,
    int matIndex,
    unsigned laneid)
{
    mmaToRowMatrixCD_f32_16x8(localRow, regs, matIndex * 2, laneid);
}

// MatrixC/D 16x16 f16 FromLocalColumn: combined lo+hi.
__device__ inline void mmaFromColMatrixCD_f16_16x16(
    uint32_t* regs,
    const void* localCol,
    int matIndex,
    unsigned gid,
    unsigned tid)
{
    mmaFromColMatrixCD_f16_16x8(regs,     localCol, matIndex * 2,     gid, tid);
    mmaFromColMatrixCD_f16_16x8(regs + 2, localCol, matIndex * 2 + 1, gid, tid);
}

// MatrixC/D 16x16 f32 FromLocalColumn: combined lo+hi.
__device__ inline void mmaFromColMatrixCD_f32_16x16(
    uint32_t* regs,
    const void* localCol,
    int matIndex,
    unsigned gid,
    unsigned tid)
{
    mmaFromColMatrixCD_f32_16x8(regs,     localCol, matIndex * 2,     gid, tid);
    mmaFromColMatrixCD_f32_16x8(regs + 4, localCol, matIndex * 2 + 1, gid, tid);
}

// MatrixC/D 16x16 f16 FromLocalRow: combined lo+hi.
__device__ inline void mmaFromRowMatrixCD_f16_16x16(
    uint32_t* regs,
    const void* localRow,
    int matIndex,
    unsigned gid,
    unsigned tid)
{
    mmaFromRowMatrixCD_f16_16x8(regs,     localRow, matIndex * 2,     gid, tid);
    mmaFromRowMatrixCD_f16_16x8(regs + 2, localRow, matIndex * 2 + 1, gid, tid);
}

// MatrixC/D 16x16 f32 FromLocalRow: combined lo+hi.
__device__ inline void mmaFromRowMatrixCD_f32_16x16(
    uint32_t* regs,
    const void* localRow,
    int matIndex,
    unsigned gid,
    unsigned tid)
{
    mmaFromRowMatrixCD_f32_16x8(regs,     localRow, matIndex * 2,     gid, tid);
    mmaFromRowMatrixCD_f32_16x8(regs + 4, localRow, matIndex * 2 + 1, gid, tid);
}

// The dimensions of the fragment are specified by M, N, K which are totally determined during
// compile time, so slang already did the pre-filter on the shape & type combination.
template<typename T, int M, int N, int K, MatrixUse R>
struct WmmaFragment
{
    __device__ WmmaFragment() {}
    __device__ WmmaFragment(T scalarValue) { fill(scalarValue); }

    typedef WmmaFragment<T, M, N, K, R> This;
    template<Layout layout>
    void __device__ Store(RWStructuredBuffer<T> buffer, uint element, uint stride)
    {
        Store<layout>(buffer.data, element, stride);
    }

    template<Layout layout>
    static This __device__ Load(StructuredBuffer<T> buffer, uint element, uint stride)
    {
        return Load<layout>(buffer.data, element, stride);
    }

    // There is no fill intrinsic in PTX wmma, so it's just 'move' value
    // to the fragment registers.
    void __device__ fill(T value)
    {
        unsigned packed = Pack32Helper(value);
        constexpr int nregs = RegisterCount<T, M, N, K, R>::value;
#pragma unroll
        for (int i = 0; i < nregs; i++)
        {
            regs[i] = packed;
        }
    }

    // Zero-clear all registers using integer zero (enables CSE to single register).
    void __device__ clear()
    {
#pragma unroll
        for (int i = 0; i < RegsCount; i++)
            regs[i] = 0U;
    }

    __device__ This operator*(T b)
    {
        This result;
#if SLANG_CUDA_ENABLE_HALF
        if (sizeof(T) == 2)
        {
            __half bh = *reinterpret_cast<const __half*>(&b);
            __half2 bv = __half2half2(bh);
            #pragma unroll
            for (int i = 0; i < RegsCount; i++)
            {
                __half2 r = __hmul2(*reinterpret_cast<const __half2*>(&regs[i]), bv);
                memcpy(&result.regs[i], &r, 4);
            }
        }
        else
#endif
        {
            for (int i = 0; i < GetLength(); i++)
                result.set(i, get(i) * b);
        }
        return result;
    }

    __device__ This operator*(const This& b)
    {
        This result;
#if SLANG_CUDA_ENABLE_HALF
        if (sizeof(T) == 2)
        {
            #pragma unroll
            for (int i = 0; i < RegsCount; i++)
            {
                __half2 r = __hmul2(*reinterpret_cast<const __half2*>(&regs[i]),
                                    *reinterpret_cast<const __half2*>(&b.regs[i]));
                memcpy(&result.regs[i], &r, 4);
            }
        }
        else
#endif
        {
            for (int i = 0; i < GetLength(); i++)
                result.set(i, get(i) * b.get(i));
        }
        return result;
    }

    __device__ This operator/(const This& other)
    {
        This result;
#if SLANG_CUDA_ENABLE_HALF
        if (sizeof(T) == 2)
        {
            #pragma unroll
            for (int i = 0; i < RegsCount; i++)
            {
                __half2 r = __h2div(*reinterpret_cast<const __half2*>(&regs[i]),
                                    *reinterpret_cast<const __half2*>(&other.regs[i]));
                memcpy(&result.regs[i], &r, 4);
            }
        }
        else
#endif
        {
            for (int i = 0; i < GetLength(); i++)
                result.set(i, get(i) / other.get(i));
        }
        return result;
    }

    __device__ This operator-(const This& other)
    {
        This result;
#if SLANG_CUDA_ENABLE_HALF
        if (sizeof(T) == 2)
        {
            #pragma unroll
            for (int i = 0; i < RegsCount; i++)
            {
                __half2 r = __hsub2(*reinterpret_cast<const __half2*>(&regs[i]),
                                    *reinterpret_cast<const __half2*>(&other.regs[i]));
                memcpy(&result.regs[i], &r, 4);
            }
        }
        else
#endif
        {
            for (int i = 0; i < GetLength(); i++)
                result.set(i, get(i) - other.get(i));
        }
        return result;
    }

    __device__ This operator-()
    {
        This result;
#if SLANG_CUDA_ENABLE_HALF
        if (sizeof(T) == 2)
        {
            #pragma unroll
            for (int i = 0; i < RegsCount; i++)
            {
                __half2 r = __hneg2(*reinterpret_cast<const __half2*>(&regs[i]));
                memcpy(&result.regs[i], &r, 4);
            }
        }
        else
#endif
        {
            for (int i = 0; i < GetLength(); i++)
                result.set(i, -get(i));
        }
        return result;
    }

    __device__ This operator+(const This& other)
    {
        This result;
#if SLANG_CUDA_ENABLE_HALF
        if (sizeof(T) == 2)
        {
            #pragma unroll
            for (int i = 0; i < RegsCount; i++)
            {
                __half2 r = __hadd2(*reinterpret_cast<const __half2*>(&regs[i]),
                                    *reinterpret_cast<const __half2*>(&other.regs[i]));
                memcpy(&result.regs[i], &r, 4);
            }
        }
        else
#endif
        {
            for (int i = 0; i < GetLength(); i++)
                result.set(i, get(i) + other.get(i));
        }
        return result;
    }

    __device__ This operator%(const This& other)
    {
        This result;
        for (int i = 0; i < GetLength(); i++)
            result.set(i, get(i) % other.get(i));
        return result;
    }

    template<typename U>
    __device__ void copyFrom(const WmmaFragment<U, M, N, K, R>& other)
    {
        // If the data type is different, we need to copy element by element.
        // Since the shape of two matrices are the same, they have the same
        // number of elements.
        for (int i = 0; i < GetLength(); i++)
        {
            set(i, static_cast<T>(other.get(i)));
        }
    }

    // Get element by index (handles bit-level access for packed types)
    // For example: u8/s8 matrices have 4 elements per register (32-bit)
    //   - index 0: bits [0:7]   of regs[0]
    //   - index 1: bits [8:15]  of regs[0]
    //   - index 2: bits [16:23] of regs[0]
    //   - index 3: bits [24:31] of regs[0]
    __device__ T get(int index) const
    {
        if constexpr (sizeof(T) == 4)
        {
            // T is 32-bit (float or int32): 1 element per register
            T v;
            memcpy(&v, &regs[index], 4);
            return v;
        }
        else if constexpr (sizeof(T) == 2)
        {
            // T is 16-bit (half): 2 elements per register
            // Elements per register: [0:15] and [16:31]
            int regIndex = index / 2;
            int elementOffset = index % 2;
            int bitOffset = elementOffset * 16;
            uint32_t extracted = (regs[regIndex] >> bitOffset) & 0xFFFF;
            uint16_t value16 = static_cast<uint16_t>(extracted);
            T v;
            memcpy(&v, &value16, 2);
            return v;
        }
        else if constexpr (sizeof(T) == 1)
        {
            // T is 8-bit (int8_t, uint8_t): 4 elements per register
            // Elements per register: [0:7], [8:15], [16:23], [24:31]
            int regIndex = index / 4;
            int elementOffset = index % 4;
            int bitOffset = elementOffset * 8;
            uint32_t extracted = (regs[regIndex] >> bitOffset) & 0xFF;
            uint8_t value8 = static_cast<uint8_t>(extracted);
            return *reinterpret_cast<const T*>(&value8);
        }
    }

    // Set element by index (handles bit-level access for packed types)
    __device__ void set(int index, T value)
    {
        if constexpr (sizeof(T) == 4)
        {
            // T is 32-bit (float or int32): 1 element per register
            memcpy(&regs[index], &value, 4);
        }
        else if constexpr (sizeof(T) == 2)
        {
            // T is 16-bit (half): 2 elements per register
            int regIndex = index / 2;
            int elementOffset = index % 2;
            int bitOffset = elementOffset * 16;
            uint32_t mask = 0xFFFF;
            uint16_t value16;
            memcpy(&value16, &value, 2);

            // Clear the bits at the target position
            regs[regIndex] &= ~(mask << bitOffset);

            // Set the new value
            regs[regIndex] |= (static_cast<uint32_t>(value16) << bitOffset);
        }
        else if constexpr (sizeof(T) == 1)
        {
            // T is 8-bit (int8_t, uint8_t): 4 elements per register
            int regIndex = index / 4;
            int elementOffset = index % 4;
            int bitOffset = elementOffset * 8;
            uint32_t mask = 0xFF;
            uint8_t value8 = *reinterpret_cast<const uint8_t*>(&value);

            // Clear the bits at the target position
            regs[regIndex] &= ~(mask << bitOffset);

            // Set the new value
            regs[regIndex] |= (static_cast<uint32_t>(value8) << bitOffset);
        }
    }

    __device__ void FragmentWrite(int regIndex, unsigned value) { regs[regIndex] = value; }
    __device__ unsigned FragmentRead(int regIndex) const { return regs[regIndex]; }

    template<Layout layout>
    void __device__ Store(T* buffer, uint element, uint stride)
    {
        // Force compile-time check, so we know the template parameter comibination is valid.
        (void)RegisterCount<T, M, N, K, R>::value;
        if constexpr (M == 16 && N == 8 && K == 16)
            mmaStore<T, M, N, K, layout>(buffer + element, regs, stride);
        else
            wmmaStore<T, M, N, K, layout>(buffer + element, regs, stride);
    }

    template<Layout layout, typename U>
    void __device__ Store(U* buffer, uint stride)
    {
        // Force compile-time check, so we know the template parameter comibination is valid.
        (void)RegisterCount<T, M, N, K, R>::value;
        if constexpr (M == 16 && N == 8 && K == 16)
            mmaStore<T, M, N, K, layout>(buffer, regs, stride * sizeof(U) / sizeof(T));
        else
            wmmaStore<T, M, N, K, layout>(buffer, regs, stride * sizeof(U) / sizeof(T));
    }

    template<Layout layout>
    static This __device__ Load(T* buffer, uint element, uint stride)
    {
        WmmaFragment<T, M, N, K, R> fragment;

        // Force compile-time check, so we know the template parameter comibination is valid.
        (void)RegisterCount<T, M, N, K, R>::value;
        if constexpr (M == 16 && N == 8 && K == 16)
            mmaLoad<T, M, N, K, R, layout>(fragment.regs, buffer + element, stride);
        else
            wmmaLoad<T, M, N, K, R, layout>(fragment.regs, buffer + element, stride);
        fragment.m_layout = layout;
        return fragment;
    }

    template<Layout layout, typename U>
    static This __device__ Load(U* buffer, uint stride)
    {
        WmmaFragment<T, M, N, K, R> fragment;

        // Force compile-time check, so we know the template parameter comibination is valid.
        (void)RegisterCount<T, M, N, K, R>::value;
        if constexpr (M == 16 && N == 8 && K == 16)
            mmaLoad<T, M, N, K, R, layout>(fragment.regs, buffer, stride * sizeof(U) / sizeof(T));
        else
            wmmaLoad<T, M, N, K, R, layout>(fragment.regs, buffer, stride * sizeof(U) / sizeof(T));
        fragment.m_layout = layout;
        return fragment;
    }

    // Load a cooperative matrix from native (fragment-register) layout.
    // The tile data must be pre-arranged so that thread laneid's RegsCount registers
    // are stored contiguously at offset laneid * RegsCount * 4 bytes.
    // This generates a single vectorized load per thread — no shuffles.
    static This __device__ LoadNative(const T* tileData)
    {
        WmmaFragment<T, M, N, K, R> fragment;
        unsigned laneid;
        asm("mov.u32 %0, %%laneid;" : "=r"(laneid));

        const uint32_t* src = reinterpret_cast<const uint32_t*>(tileData) + laneid * RegsCount;

        if constexpr (RegsCount == 2)
        {
            *reinterpret_cast<uint2*>(fragment.regs) = *reinterpret_cast<const uint2*>(src);
        }
        else if constexpr (RegsCount == 4)
        {
            *reinterpret_cast<uint4*>(fragment.regs) = *reinterpret_cast<const uint4*>(src);
        }
        else if constexpr (RegsCount == 8)
        {
            *reinterpret_cast<uint4*>(fragment.regs) = *reinterpret_cast<const uint4*>(src);
            *reinterpret_cast<uint4*>(fragment.regs + 4) = *reinterpret_cast<const uint4*>(src + 4);
        }

        fragment.m_layout = Layout::RowMajor;
        return fragment;
    }

    // Construct a cooperative matrix from per-thread local rows.
    // Each thread provides one full row of the matrix (colsPerRow elements).
    // M (=16) threads form one matrix. matIndex selects which group of M threads
    // contributes: threads [matIndex*M .. (matIndex+1)*M).
    static This __device__ FromLocalRow(const T* localRow, int matIndex)
    {
        WmmaFragment<T, M, N, K, R> fragment;

        static_assert(M == 16 && N == 16 && K == 16,
            "FromLocalRow only supports the m16n16k16 MMA shape");

        unsigned laneid;
        asm("mov.u32 %0, %%laneid;" : "=r"(laneid));
        unsigned gid = laneid >> 2;
        unsigned tid = laneid & 3;

        if constexpr (R == MatrixUse::MatrixA && sizeof(T) == 2)
            mmaFromRowMatrixA_16x16(fragment.regs, localRow, matIndex, gid, tid);
        else if constexpr (R == MatrixUse::MatrixB && sizeof(T) == 2)
            mmaFromRowMatrixB_16x16(fragment.regs, localRow, matIndex, gid, tid);
        else if constexpr (
            (R == MatrixUse::MatrixC || R == MatrixUse::MatrixD) && sizeof(T) == 4)
            mmaFromRowMatrixCD_f32_16x16(fragment.regs, localRow, matIndex, gid, tid);
        else if constexpr (
            (R == MatrixUse::MatrixC || R == MatrixUse::MatrixD) && sizeof(T) == 2)
            mmaFromRowMatrixCD_f16_16x16(fragment.regs, localRow, matIndex, gid, tid);

        fragment.m_layout = Layout::RowMajor;
        return fragment;
    }

    void __device__ ToLocalRow(T* localRow, int matIndex) const
    {
        static_assert(M == 16 && N == 16 && K == 16,
            "ToLocalRow only supports the m16n16k16 MMA shape");

        unsigned laneid;
        asm("mov.u32 %0, %%laneid;" : "=r"(laneid));

        if constexpr (R == MatrixUse::MatrixA && sizeof(T) == 2)
            mmaToRowMatrixA_16x16(localRow, regs, matIndex, laneid);
        else if constexpr (
            (R == MatrixUse::MatrixC || R == MatrixUse::MatrixD) && sizeof(T) == 2)
            mmaToRowMatrixCD_f16_16x16(localRow, regs, matIndex, laneid);
        else if constexpr (
            (R == MatrixUse::MatrixC || R == MatrixUse::MatrixD) && sizeof(T) == 4)
            mmaToRowMatrixCD_f32_16x16(localRow, regs, matIndex, laneid);
    }

    // Construct a cooperative matrix from per-thread local columns.
    // Each thread provides one full column of the matrix (M elements for the row count of this matrix).
    // matIndex selects which group of threads contributes.
    static This __device__ FromLocalColumn(const T* localCol, int matIndex)
    {
        WmmaFragment<T, M, N, K, R> fragment;

        static_assert(M == 16 && N == 16 && K == 16,
            "FromLocalColumn only supports the m16n16k16 MMA shape");

        unsigned laneid;
        asm("mov.u32 %0, %%laneid;" : "=r"(laneid));
        unsigned gid = laneid >> 2;
        unsigned tid = laneid & 3;

        if constexpr (R == MatrixUse::MatrixA && sizeof(T) == 2)
            mmaFromColMatrixA_16x16(fragment.regs, localCol, matIndex, gid, tid);
        else if constexpr (R == MatrixUse::MatrixB && sizeof(T) == 2)
            mmaFromColMatrixB_16x16(fragment.regs, localCol, matIndex, gid, tid);
        else if constexpr (
            (R == MatrixUse::MatrixC || R == MatrixUse::MatrixD) && sizeof(T) == 4)
            mmaFromColMatrixCD_f32_16x16(fragment.regs, localCol, matIndex, gid, tid);
        else if constexpr (
            (R == MatrixUse::MatrixC || R == MatrixUse::MatrixD) && sizeof(T) == 2)
            mmaFromColMatrixCD_f16_16x16(fragment.regs, localCol, matIndex, gid, tid);

        fragment.m_layout = Layout::ColMajor;
        return fragment;
    }

    // Extract one column per thread from the cooperative matrix.
    // Inverse of FromLocalColumn. Each thread receives M elements (one full column).
    void __device__ ToLocalColumn(T* localCol, int matIndex) const
    {
        static_assert(M == 16 && N == 16 && K == 16,
            "ToLocalColumn only supports the m16n16k16 MMA shape");

        unsigned laneid;
        asm("mov.u32 %0, %%laneid;" : "=r"(laneid));

        if constexpr (R == MatrixUse::MatrixA && sizeof(T) == 2)
            mmaToColMatrixA_16x16(localCol, regs, matIndex, laneid);
        else if constexpr (
            (R == MatrixUse::MatrixC || R == MatrixUse::MatrixD) && sizeof(T) == 2)
            mmaToColMatrixCD_f16_16x16(localCol, regs, matIndex, laneid);
        else if constexpr (
            (R == MatrixUse::MatrixC || R == MatrixUse::MatrixD) && sizeof(T) == 4)
            mmaToColMatrixCD_f32_16x16(localCol, regs, matIndex, laneid);
    }

    // Convert a 16x16 MatA fragment to a 16x16 MatB fragment (register copy, no shuffles).
    // After ChangeMajor on MatA, its regs[0:3] hold column-major data that can be
    // directly reinterpreted as two 16x8 MatB halves: lo=regs[0:1], hi=regs[2:3].
    static This __device__ FromMatA(const WmmaFragment<T, M, N, K, MatrixUse::MatrixA>& matA)
    {
        static_assert(R == MatrixUse::MatrixB && M == 16 && N == 16 && K == 16,
            "FromMatA only valid for 16x16 MatrixB");
        This result;
        *reinterpret_cast<uint4*>(result.regs) = *reinterpret_cast<const uint4*>(matA.regs);
        return result;
    }

    static constexpr __device__ uint32_t GetLength() { return This::elements_per_thread; }
    static constexpr __device__ int GetPackedFragmentCount() { return RegsCount; }

    // For referencing those template parameters outside the struct
    using ElementType = T;
    static constexpr int m_M = M;
    static constexpr int m_N = N;
    static constexpr int m_K = K;
    Layout m_layout = Layout::RowMajor;

    // Register Count requirement
    static constexpr int RegsCount = RegisterCount<T, M, N, K, R>::value;
    unsigned regs[RegsCount] = {};

    static constexpr uint32_t elements_per_thread = RegsCount * (4 / sizeof(T));

    // Native store/load for efficient shared memory reductions.
    // Each lane stores/loads its raw registers as a contiguous block at shmem[offset + laneId].

    // Number of uint4s per lane needed to store all registers.
    static constexpr int Uint4PerLane = (RegsCount + 3) / 4;

    // Stores raw registers to shmem. Each lane writes Uint4PerLane uint4s
    static constexpr int NativeStridePerLane = RegsCount;

    template<typename U>
    __device__ void storeNative(U* shmem, unsigned offset)
    {
        unsigned lane_id = threadIdx.x % 32;
        unsigned saddr = (unsigned)__cvta_generic_to_shared(shmem) + offset * sizeof(U) + lane_id * NativeStridePerLane * sizeof(unsigned);
        size_t saddr64 = saddr;
        unsigned* dst;
        asm("cvta.shared.u64 %0, %1;" : "=l"(dst) : "l"(saddr64));
        if constexpr (RegsCount == 2)
            *reinterpret_cast<uint2*>(dst) = *reinterpret_cast<const uint2*>(regs);
        else if constexpr (RegsCount == 4)
            *reinterpret_cast<uint4*>(dst) = *reinterpret_cast<const uint4*>(regs);
        else if constexpr (RegsCount == 8)
        {
            *reinterpret_cast<uint4*>(dst) = *reinterpret_cast<const uint4*>(regs);
            *reinterpret_cast<uint4*>(dst + 4) = *reinterpret_cast<const uint4*>(regs + 4);
        }
    }

    template<typename U>
    __device__ void loadNative(const U* shmem, unsigned offset)
    {
        unsigned lane_id = threadIdx.x % 32;
        unsigned saddr = (unsigned)__cvta_generic_to_shared(shmem) + offset * sizeof(U) + lane_id * NativeStridePerLane * sizeof(unsigned);
        size_t saddr64 = saddr;
        const unsigned* src;
        asm("cvta.shared.u64 %0, %1;" : "=l"(src) : "l"(saddr64));
        if constexpr (RegsCount == 2)
            *reinterpret_cast<uint2*>(regs) = *reinterpret_cast<const uint2*>(src);
        else if constexpr (RegsCount == 4)
            *reinterpret_cast<uint4*>(regs) = *reinterpret_cast<const uint4*>(src);
        else if constexpr (RegsCount == 8)
        {
            *reinterpret_cast<uint4*>(regs) = *reinterpret_cast<const uint4*>(src);
            *reinterpret_cast<uint4*>(regs + 4) = *reinterpret_cast<const uint4*>(src + 4);
        }
    }
};

// ====================================================================================
// FP16 MMA Helper - For half x half inputs
// Specialized on CType and DType (accumulator types)
//
// PTX Syntax: wmma.mma.sync.aligned.alayout.blayout.shape.dtype.ctype d, a, b, c;
//   where:
//     dtype = type of d (output accumulator): {.f16, .f32}
//     ctype = type of c (input accumulator):  {.f16, .f32}
//
// Note: Types of a and b are implicitly f16 (not specified in PTX instruction).
//       Shape (M, N, K) is passed as template parameters, so one template handles all shapes.
//       We only need to specialize on CType and DType.
// ====================================================================================

template<typename CType, typename DType, int M, int N, int K, Layout LayoutA, Layout LayoutB>
struct Fp16MMAHelper;

#if SLANG_CUDA_ENABLE_HALF
// Specialization: c=half, d=half (f16.f16)
template<int M, int N, int K, Layout LayoutA, Layout LayoutB>
struct Fp16MMAHelper<half, half, M, N, K, LayoutA, LayoutB>
{
    __device__ static void eval(
        WmmaFragment<half, M, N, K, MatrixC>& d,
        const WmmaFragment<half, M, N, K, MatrixUse::MatrixA>& a,
        const WmmaFragment<half, M, N, K, MatrixUse::MatrixB>& b,
        const WmmaFragment<half, M, N, K, MatrixUse::MatrixC>& c)
    {
        asm volatile("wmma.mma.sync.aligned.%4.%5.%6.%7.%8 "
                     "{%0, %1, %2, %3}, "
                     "{%9, %10, %11, %12, %13, %14, %15, %16}, "
                     "{%17, %18, %19, %20, %21, %22, %23, %24}, "
                     "{%25, %26, %27, %28};\n"
                     : "=r"(d.regs[0]), "=r"(d.regs[1]), "=r"(d.regs[2]), "=r"(d.regs[3])
                     : "C"(PtxLayoutName<LayoutA>::name),
                       "C"(PtxLayoutName<LayoutB>::name),
                       "C"(PtxShapeName<M, N, K>::name),
                       "C"(PtxTypeName<half>::name),
                       "C"(PtxTypeName<half>::name),
                       "r"(a.regs[0]),
                       "r"(a.regs[1]),
                       "r"(a.regs[2]),
                       "r"(a.regs[3]),
                       "r"(a.regs[4]),
                       "r"(a.regs[5]),
                       "r"(a.regs[6]),
                       "r"(a.regs[7]),
                       "r"(b.regs[0]),
                       "r"(b.regs[1]),
                       "r"(b.regs[2]),
                       "r"(b.regs[3]),
                       "r"(b.regs[4]),
                       "r"(b.regs[5]),
                       "r"(b.regs[6]),
                       "r"(b.regs[7]),
                       "r"(c.regs[0]),
                       "r"(c.regs[1]),
                       "r"(c.regs[2]),
                       "r"(c.regs[3]));
    }
};

// Specialization: c=float, d=half (f16.f32)
template<int M, int N, int K, Layout LayoutA, Layout LayoutB>
struct Fp16MMAHelper<float, half, M, N, K, LayoutA, LayoutB>
{
    __device__ static void eval(
        WmmaFragment<half, M, N, K, MatrixUse::MatrixC>& d,
        const WmmaFragment<half, M, N, K, MatrixUse::MatrixA>& a,
        const WmmaFragment<half, M, N, K, MatrixUse::MatrixB>& b,
        const WmmaFragment<float, M, N, K, MatrixUse::MatrixC>& c)
    {
        asm volatile("wmma.mma.sync.aligned.%4.%5.%6.%7.%8 "
                     "{%0, %1, %2, %3}, "
                     "{%9, %10, %11, %12, %13, %14, %15, %16}, "
                     "{%17, %18, %19, %20, %21, %22, %23, %24}, "
                     "{%25, %26, %27, %28, %29, %30, %31, %32};\n"
                     : "=r"(d.regs[0]), "=r"(d.regs[1]), "=r"(d.regs[2]), "=r"(d.regs[3])
                     : "C"(PtxLayoutName<LayoutA>::name),
                       "C"(PtxLayoutName<LayoutB>::name),
                       "C"(PtxShapeName<M, N, K>::name),
                       "C"(PtxTypeName<half>::name),
                       "C"(PtxTypeName<float>::name),
                       "r"(a.regs[0]),
                       "r"(a.regs[1]),
                       "r"(a.regs[2]),
                       "r"(a.regs[3]),
                       "r"(a.regs[4]),
                       "r"(a.regs[5]),
                       "r"(a.regs[6]),
                       "r"(a.regs[7]),
                       "r"(b.regs[0]),
                       "r"(b.regs[1]),
                       "r"(b.regs[2]),
                       "r"(b.regs[3]),
                       "r"(b.regs[4]),
                       "r"(b.regs[5]),
                       "r"(b.regs[6]),
                       "r"(b.regs[7]),
                       "r"(c.regs[0]),
                       "r"(c.regs[1]),
                       "r"(c.regs[2]),
                       "r"(c.regs[3]),
                       "r"(c.regs[4]),
                       "r"(c.regs[5]),
                       "r"(c.regs[6]),
                       "r"(c.regs[7]));
    }
};

// Specialization: c=half, d=float (f32.f16)
template<int M, int N, int K, Layout LayoutA, Layout LayoutB>
struct Fp16MMAHelper<half, float, M, N, K, LayoutA, LayoutB>
{
    __device__ static void eval(
        WmmaFragment<float, M, N, K, MatrixUse::MatrixC>& d,
        const WmmaFragment<half, M, N, K, MatrixUse::MatrixA>& a,
        const WmmaFragment<half, M, N, K, MatrixUse::MatrixB>& b,
        const WmmaFragment<half, M, N, K, MatrixUse::MatrixC>& c)
    {
        asm volatile("wmma.mma.sync.aligned.%8.%9.%10.%11.%12 "
                     "{%0, %1, %2, %3, %4, %5, %6, %7}, "
                     "{%13, %14, %15, %16, %17, %18, %19, %20}, "
                     "{%21, %22, %23, %24, %25, %26, %27, %28}, "
                     "{%29, %30, %31, %32};\n"
                     : "=r"(d.regs[0]),
                       "=r"(d.regs[1]),
                       "=r"(d.regs[2]),
                       "=r"(d.regs[3]),
                       "=r"(d.regs[4]),
                       "=r"(d.regs[5]),
                       "=r"(d.regs[6]),
                       "=r"(d.regs[7])
                     : "C"(PtxLayoutName<LayoutA>::name),
                       "C"(PtxLayoutName<LayoutB>::name),
                       "C"(PtxShapeName<M, N, K>::name),
                       "C"(PtxTypeName<float>::name),
                       "C"(PtxTypeName<half>::name),
                       "r"(a.regs[0]),
                       "r"(a.regs[1]),
                       "r"(a.regs[2]),
                       "r"(a.regs[3]),
                       "r"(a.regs[4]),
                       "r"(a.regs[5]),
                       "r"(a.regs[6]),
                       "r"(a.regs[7]),
                       "r"(b.regs[0]),
                       "r"(b.regs[1]),
                       "r"(b.regs[2]),
                       "r"(b.regs[3]),
                       "r"(b.regs[4]),
                       "r"(b.regs[5]),
                       "r"(b.regs[6]),
                       "r"(b.regs[7]),
                       "r"(c.regs[0]),
                       "r"(c.regs[1]),
                       "r"(c.regs[2]),
                       "r"(c.regs[3]));
    }
};

// Specialization: c=float, d=float (f32.f32)
template<int M, int N, int K, Layout LayoutA, Layout LayoutB>
struct Fp16MMAHelper<float, float, M, N, K, LayoutA, LayoutB>
{
    __device__ static void eval(
        WmmaFragment<float, M, N, K, MatrixUse::MatrixC>& d,
        const WmmaFragment<half, M, N, K, MatrixUse::MatrixA>& a,
        const WmmaFragment<half, M, N, K, MatrixUse::MatrixB>& b,
        const WmmaFragment<float, M, N, K, MatrixUse::MatrixC>& c)
    {
        asm volatile("wmma.mma.sync.aligned.%8.%9.%10.%11.%12 "
                     "{%0, %1, %2, %3, %4, %5, %6, %7}, "
                     "{%13, %14, %15, %16, %17, %18, %19, %20}, "
                     "{%21, %22, %23, %24, %25, %26, %27, %28}, "
                     "{%29, %30, %31, %32, %33, %34, %35, %36};\n"
                     : "=r"(d.regs[0]),
                       "=r"(d.regs[1]),
                       "=r"(d.regs[2]),
                       "=r"(d.regs[3]),
                       "=r"(d.regs[4]),
                       "=r"(d.regs[5]),
                       "=r"(d.regs[6]),
                       "=r"(d.regs[7])
                     : "C"(PtxLayoutName<LayoutA>::name),
                       "C"(PtxLayoutName<LayoutB>::name),
                       "C"(PtxShapeName<M, N, K>::name),
                       "C"(PtxTypeName<float>::name),
                       "C"(PtxTypeName<float>::name),
                       "r"(a.regs[0]),
                       "r"(a.regs[1]),
                       "r"(a.regs[2]),
                       "r"(a.regs[3]),
                       "r"(a.regs[4]),
                       "r"(a.regs[5]),
                       "r"(a.regs[6]),
                       "r"(a.regs[7]),
                       "r"(b.regs[0]),
                       "r"(b.regs[1]),
                       "r"(b.regs[2]),
                       "r"(b.regs[3]),
                       "r"(b.regs[4]),
                       "r"(b.regs[5]),
                       "r"(b.regs[6]),
                       "r"(b.regs[7]),
                       "r"(c.regs[0]),
                       "r"(c.regs[1]),
                       "r"(c.regs[2]),
                       "r"(c.regs[3]),
                       "r"(c.regs[4]),
                       "r"(c.regs[5]),
                       "r"(c.regs[6]),
                       "r"(c.regs[7]));
    }
};
#endif // #if SLANG_CUDA_ENABLE_HALF

// ====================================================================================
// Integer MMA Helper - For int8/uint8 inputs
// Specialized on shape (register counts depend on shape)
//
// PTX Syntax: wmma.mma.sync.aligned.alayout.blayout.shape.s32.atype.btype.s32{.satfinite} d, a, b,
// c;
//   where:
//     atype = type of a (input matrix A): {.s8, .u8}
//     btype = type of b (input matrix B): {.s8, .u8}
//     C and D are always s32 (int32)
//
// Note: Unlike FP16, integer operations explicitly specify atype and btype in the instruction.
//       We must specialize on shape because register counts vary:
//         m16n16k16: a=2 regs, b=2 regs
//         m8n32k16:  a=1 reg,  b=4 regs
//         m32n8k16:  a=4 regs, b=1 reg
//       C and D always use 8 registers (int32).
// ====================================================================================

template<
    typename AType,
    typename BType,
    ShapeCombination shape,
    Layout LayoutA,
    Layout LayoutB,
    bool saturatingAccumulation>
struct IntegerMMAHelper;

// Specialization: m16n16k16 (a=2 regs, b=2 regs)
template<
    typename AType,
    typename BType,
    Layout LayoutA,
    Layout LayoutB,
    bool saturatingAccumulation>
struct IntegerMMAHelper<
    AType,
    BType,
    ShapeCombination::m16n16k16,
    LayoutA,
    LayoutB,
    saturatingAccumulation>
{
    __device__ static void eval(
        WmmaFragment<int, 16, 16, 16, MatrixUse::MatrixC>& d,
        const WmmaFragment<AType, 16, 16, 16, MatrixUse::MatrixA>& a,
        const WmmaFragment<BType, 16, 16, 16, MatrixUse::MatrixB>& b,
        const WmmaFragment<int, 16, 16, 16, MatrixUse::MatrixC>& c)
    {
        asm volatile("wmma.mma.sync.aligned.%8.%9.%10.s32.%11.%12.s32%13 "
                     "{%0, %1, %2, %3, %4, %5, %6, %7}, "
                     "{%14, %15}, "
                     "{%16, %17}, "
                     "{%18, %19, %20, %21, %22, %23, %24, %25};\n"
                     : "=r"(d.regs[0]),
                       "=r"(d.regs[1]),
                       "=r"(d.regs[2]),
                       "=r"(d.regs[3]),
                       "=r"(d.regs[4]),
                       "=r"(d.regs[5]),
                       "=r"(d.regs[6]),
                       "=r"(d.regs[7])
                     : "C"(PtxLayoutName<LayoutA>::name),
                       "C"(PtxLayoutName<LayoutB>::name),
                       "C"(PtxShapeName<16, 16, 16>::name),
                       "C"(PtxTypeName<AType>::name),
                       "C"(PtxTypeName<BType>::name),
                       "C"(IsSaturated<saturatingAccumulation>::name),
                       "r"(a.regs[0]),
                       "r"(a.regs[1]),
                       "r"(b.regs[0]),
                       "r"(b.regs[1]),
                       "r"(c.regs[0]),
                       "r"(c.regs[1]),
                       "r"(c.regs[2]),
                       "r"(c.regs[3]),
                       "r"(c.regs[4]),
                       "r"(c.regs[5]),
                       "r"(c.regs[6]),
                       "r"(c.regs[7]));
    }
};

// Specialization: m8n32k16 (a=1 reg, b=4 regs)
template<
    typename AType,
    typename BType,
    Layout LayoutA,
    Layout LayoutB,
    bool saturatingAccumulation>
struct IntegerMMAHelper<
    AType,
    BType,
    ShapeCombination::m8n32k16,
    LayoutA,
    LayoutB,
    saturatingAccumulation>
{
    __device__ static void eval(
        WmmaFragment<int, 8, 32, 16, MatrixUse::MatrixC>& d,
        const WmmaFragment<AType, 8, 32, 16, MatrixUse::MatrixA>& a,
        const WmmaFragment<BType, 8, 32, 16, MatrixUse::MatrixB>& b,
        const WmmaFragment<int, 8, 32, 16, MatrixUse::MatrixC>& c)
    {
        asm volatile("wmma.mma.sync.aligned.%8.%9.%10.s32.%11.%12.s32%13 "
                     "{%0, %1, %2, %3, %4, %5, %6, %7}, "
                     "{%14}, "
                     "{%15, %16, %17, %18}, "
                     "{%19, %20, %21, %22, %23, %24, %25, %26};\n"
                     : "=r"(d.regs[0]),
                       "=r"(d.regs[1]),
                       "=r"(d.regs[2]),
                       "=r"(d.regs[3]),
                       "=r"(d.regs[4]),
                       "=r"(d.regs[5]),
                       "=r"(d.regs[6]),
                       "=r"(d.regs[7])
                     : "C"(PtxLayoutName<LayoutA>::name),
                       "C"(PtxLayoutName<LayoutB>::name),
                       "C"(PtxShapeName<8, 32, 16>::name),
                       "C"(PtxTypeName<AType>::name),
                       "C"(PtxTypeName<BType>::name),
                       "C"(IsSaturated<saturatingAccumulation>::name),
                       "r"(a.regs[0]),
                       "r"(b.regs[0]),
                       "r"(b.regs[1]),
                       "r"(b.regs[2]),
                       "r"(b.regs[3]),
                       "r"(c.regs[0]),
                       "r"(c.regs[1]),
                       "r"(c.regs[2]),
                       "r"(c.regs[3]),
                       "r"(c.regs[4]),
                       "r"(c.regs[5]),
                       "r"(c.regs[6]),
                       "r"(c.regs[7]));
    }
};

// Specialization: m32n8k16 (a=4 regs, b=1 reg)
template<
    typename AType,
    typename BType,
    Layout LayoutA,
    Layout LayoutB,
    bool saturatingAccumulation>
struct IntegerMMAHelper<
    AType,
    BType,
    ShapeCombination::m32n8k16,
    LayoutA,
    LayoutB,
    saturatingAccumulation>
{
    __device__ static void eval(
        WmmaFragment<int, 32, 8, 16, MatrixUse::MatrixC>& d,
        const WmmaFragment<AType, 32, 8, 16, MatrixUse::MatrixA>& a,
        const WmmaFragment<BType, 32, 8, 16, MatrixUse::MatrixB>& b,
        const WmmaFragment<int, 32, 8, 16, MatrixUse::MatrixC>& c)
    {
        asm volatile("wmma.mma.sync.aligned.%8.%9.%10.s32.%11.%12.s32%13 "
                     "{%0, %1, %2, %3, %4, %5, %6, %7}, "
                     "{%14, %15, %16, %17}, "
                     "{%18}, "
                     "{%19, %20, %21, %22, %23, %24, %25, %26};\n"
                     : "=r"(d.regs[0]),
                       "=r"(d.regs[1]),
                       "=r"(d.regs[2]),
                       "=r"(d.regs[3]),
                       "=r"(d.regs[4]),
                       "=r"(d.regs[5]),
                       "=r"(d.regs[6]),
                       "=r"(d.regs[7])
                     : "C"(PtxLayoutName<LayoutA>::name),
                       "C"(PtxLayoutName<LayoutB>::name),
                       "C"(PtxShapeName<32, 8, 16>::name),
                       "C"(PtxTypeName<AType>::name),
                       "C"(PtxTypeName<BType>::name),
                       "C"(IsSaturated<saturatingAccumulation>::name),
                       "r"(a.regs[0]),
                       "r"(a.regs[1]),
                       "r"(a.regs[2]),
                       "r"(a.regs[3]),
                       "r"(b.regs[0]),
                       "r"(c.regs[0]),
                       "r"(c.regs[1]),
                       "r"(c.regs[2]),
                       "r"(c.regs[3]),
                       "r"(c.regs[4]),
                       "r"(c.regs[5]),
                       "r"(c.regs[6]),
                       "r"(c.regs[7]));
    }
};


// ====================================================================================
// MMA m16n8k16 ChangeMajor (in-register transpose) for Matrix A (16x16, f16)
//
// Uses movmatrix.sync.aligned.m8n8.trans.b16 to transpose each 8x8 quadrant,
// then swaps off-diagonal blocks for the 2x2 block-level transpose.
//
// Before:  reg0=A00, reg1=A10, reg2=A01, reg3=A11  (row-major fragments)
// After:   reg0=A00^T, reg1=A01^T, reg2=A10^T, reg3=A11^T  (column-major fragments)
// ====================================================================================

// ====================================================================================
// BatchFromArray: optimized batch shuffle for constructing multiple MatA (16x16)
// fragments from a per-thread array. Implements Tin2's OPTIMIZED_SHUFFLE algorithm.
//
// Produces 2 * (N_COLS/16) fragments covering a 32×N_COLS row-major matrix
// (all 32 warp threads contribute simultaneously).
// Uses 1 shuffle per packed column = N_COLS/2 total shuffles.
// ====================================================================================

#if SLANG_CUDA_ENABLE_HALF
template<int N_COLS>
__device__ inline void mmaBatchFromArray_MatA_16x16(
    WmmaFragment<half, 16, 16, 16, MatrixUse::MatrixA>* __restrict__ fragments,
    const uint32_t* __restrict__ ip_packed_arr)
{
    unsigned laneid;
    asm("mov.u32 %0, %%laneid;" : "=r"(laneid));

    static constexpr uint32_t ColsPacked = N_COLS / 2;
    static constexpr uint32_t RegColsPacked = 4;
    static constexpr uint32_t RegRowsPacked = 8;
    static constexpr uint32_t RegRows = 2;
    static constexpr uint32_t MMARows = 2;

    #pragma unroll
    for (uint32_t col_packed = 0; col_packed < ColsPacked; col_packed += RegColsPacked)
    {
        uint32_t regsi[RegColsPacked];

        #pragma unroll
        for (uint32_t i = 0; i < RegColsPacked; i++)
        {
            uint32_t src = col_packed + i;
            regsi[i] = (src < ColsPacked) ? ip_packed_arr[src] : 0;

            #pragma unroll
            for (uint32_t j = 1; j < RegColsPacked; j++)
            {
                uint32_t alt_src = col_packed + (i + j) % RegColsPacked;
                uint32_t val_packed = (alt_src < ColsPacked) ? ip_packed_arr[alt_src] : 0;
                if (laneid / RegRowsPacked == j)
                    regsi[i] = val_packed;
            }
        }

        #pragma unroll
        for (uint32_t i = 0; i < RegColsPacked; i++)
        {
            uint32_t shfl_idx = ((laneid + (RegColsPacked - i)) % RegColsPacked) * RegRowsPacked + laneid / RegColsPacked;
            regsi[i] = __shfl_sync(0xFFFFFFFF, regsi[i], shfl_idx);
        }

        uint32_t mma_col = col_packed / (16 / 2);
        uint32_t r_col = (col_packed % (16 / 2)) / RegColsPacked;

        #pragma unroll
        for (uint32_t i = 0; i < RegColsPacked; i++)
        {
            uint32_t r_row = i % RegRows;
            uint32_t mma_row = i / RegRows;

            uint32_t frag_idx = mma_col * MMARows + mma_row;
            uint32_t reg_idx = r_col * RegRows + r_row;

            fragments[frag_idx].regs[reg_idx] = regsi[0];

            #pragma unroll
            for (uint32_t j = 1; j < RegColsPacked; j++)
            {
                if (laneid % RegColsPacked == (i + j) % RegColsPacked)
                    fragments[frag_idx].regs[reg_idx] = regsi[j];
            }
        }
    }
}
#endif

// ====================================================================================
// Shared memory based fragment construction using ldmatrix.
// Each thread writes its local row to shared memory, then ldmatrix loads
// the fragment directly from shmem — zero shuffles.
// ====================================================================================

// Store a per-thread local row (16 halves = 32 bytes) to shared memory.
// All 32 threads write their rows at base + laneId * 16 (in half units).
// After a __syncwarp(), loadShMem can read the fragments.
__device__ inline void mmaStoreLocalRowToShMem(
    half* smem_base,
    const uint32_t* local_packed,
    int num_halves)
{
    unsigned laneid;
    asm("mov.u32 %0, %%laneid;" : "=r"(laneid));

    uint32_t* dst = reinterpret_cast<uint32_t*>(smem_base + laneid * num_halves);
    int num_packed = num_halves / 2;
    #pragma unroll
    for (int i = 0; i < num_packed; i++)
        dst[i] = local_packed[i];
}

// Load two 16x16 MatA fragments from a 32x16 shared memory tile using ldmatrix.
// smem_base points to a 32-row x 16-col (half) tile in shared memory.
// Each row is `stride` halves wide (typically 16).
// outTiles[0] = MatA from rows 0-15, outTiles[1] = MatA from rows 16-31.
//
// For mma.m16n8k16, MatA (row-major, 16x16) occupies 4 regs per thread.
// ldmatrix.sync.aligned.m8n8.x4 loads exactly these 4 regs when each
// thread provides the address of its contribution row in the tile.
//
// Thread-to-address mapping for a 16x16 row-major tile:
//   ldmatrix.x4 groups threads into 4 groups of 8.
//   Group g (g=0..3), lane-in-group l (l=0..7):
//     row = (g & 1) * 8 + l        (groups 0,2 -> rows 0-7; groups 1,3 -> rows 8-15)
//     col_offset = (g >> 1) * 8     (groups 0,1 -> cols 0-7; groups 2,3 -> cols 8-15)
//     addr = base + row * stride + col_offset
#if SLANG_CUDA_ENABLE_HALF
__device__ inline void mmaLoadShMemMatA_16x16(
    WmmaFragment<half, 16, 16, 16, MatrixUse::MatrixA>* outTiles,
    const half* smem_base,
    int stride)
{
    unsigned laneid;
    asm("mov.u32 %0, %%laneid;" : "=r"(laneid));

    unsigned group = laneid / 8;        // 0,1,2,3
    unsigned lane_in_group = laneid % 8;
    unsigned base_row = (group & 1) * 8 + lane_in_group;
    unsigned col_off = (group >> 1) * 8;

    // Tile 0: rows 0-15
    // Use generic addressing (no .shared) because the pointer has already
    // been converted from .shared to generic by Slang's cvta.shared.u64.
    {
        const half* row_ptr = smem_base + base_row * stride + col_off;
        uint32_t r0, r1, r2, r3;
        asm volatile(
            "ldmatrix.sync.aligned.m8n8.x4.b16 {%0, %1, %2, %3}, [%4];"
            : "=r"(r0), "=r"(r1), "=r"(r2), "=r"(r3)
            : "l"(row_ptr));
        outTiles[0].regs[0] = r0;
        outTiles[0].regs[1] = r1;
        outTiles[0].regs[2] = r2;
        outTiles[0].regs[3] = r3;
    }

    // Tile 1: rows 16-31
    {
        const half* row_ptr = smem_base + (16 + base_row) * stride + col_off;
        uint32_t r0, r1, r2, r3;
        asm volatile(
            "ldmatrix.sync.aligned.m8n8.x4.b16 {%0, %1, %2, %3}, [%4];"
            : "=r"(r0), "=r"(r1), "=r"(r2), "=r"(r3)
            : "l"(row_ptr));
        outTiles[1].regs[0] = r0;
        outTiles[1].regs[1] = r1;
        outTiles[1].regs[2] = r2;
        outTiles[1].regs[3] = r3;
    }
}
#endif

// Load a single 16x16 MatA fragment from shared memory using ldmatrix.
// smem_base: pointer to shmem tile start.
// stride: number of halves per row.
// rowOffset: starting row in shmem (0 for threads 0-15, 16 for threads 16-31).
#if SLANG_CUDA_ENABLE_HALF
__device__ inline void mmaLoadShMemMatA_16x16_single(
    WmmaFragment<half, 16, 16, 16, MatrixUse::MatrixA>& outTile,
    const half* smem_base,
    int stride,
    int rowOffset)
{
    unsigned laneid;
    asm("mov.u32 %0, %%laneid;" : "=r"(laneid));

    unsigned group = laneid / 8;
    unsigned lane_in_group = laneid % 8;
    unsigned base_row = rowOffset + (group & 1) * 8 + lane_in_group;
    unsigned col_off = (group >> 1) * 8;

    const half* row_ptr = smem_base + base_row * stride + col_off;
    uint32_t r0, r1, r2, r3;
    asm volatile(
        "ldmatrix.sync.aligned.m8n8.x4.b16 {%0, %1, %2, %3}, [%4];"
        : "=r"(r0), "=r"(r1), "=r"(r2), "=r"(r3)
        : "l"(row_ptr));
    outTile.regs[0] = r0;
    outTile.regs[1] = r1;
    outTile.regs[2] = r2;
    outTile.regs[3] = r3;
}
#endif

// Extract the lower 16x8 half (columns 0-7) of a ChangeMajor'd MatA as MatB.
__device__ inline WmmaFragment<half, 16, 8, 16, MatrixUse::MatrixB>
mmaExtractMatBLo(const WmmaFragment<half, 16, 16, 16, MatrixUse::MatrixA>& a)
{
    WmmaFragment<half, 16, 8, 16, MatrixUse::MatrixB> b;
    b.regs[0] = a.regs[0];
    b.regs[1] = a.regs[1];
    return b;
}

// Extract the upper 16x8 half (columns 8-15) of a ChangeMajor'd MatA as MatB.
__device__ inline WmmaFragment<half, 16, 8, 16, MatrixUse::MatrixB>
mmaExtractMatBHi(const WmmaFragment<half, 16, 16, 16, MatrixUse::MatrixA>& a)
{
    WmmaFragment<half, 16, 8, 16, MatrixUse::MatrixB> b;
    b.regs[0] = a.regs[2];
    b.regs[1] = a.regs[3];
    return b;
}

// ====================================================================================
// MMA m16n8k16 ChangeMajor (in-register transpose) for Matrix A (16x16, f16)
// ====================================================================================

#if SLANG_CUDA_ENABLE_HALF
template<int M, int N, int K, MatrixUse R>
__device__ inline void mmaChangeMajor(WmmaFragment<half, M, N, K, R>& frag)
{
    if constexpr (M == 16 && K == 16 && R == MatrixUse::MatrixA)
    {
        uint32_t t0, t1, t2, t3;
        asm volatile("movmatrix.sync.aligned.m8n8.trans.b16 %0, %1;" : "=r"(t0) : "r"(frag.regs[0]));
        asm volatile("movmatrix.sync.aligned.m8n8.trans.b16 %0, %1;" : "=r"(t1) : "r"(frag.regs[1]));
        asm volatile("movmatrix.sync.aligned.m8n8.trans.b16 %0, %1;" : "=r"(t2) : "r"(frag.regs[2]));
        asm volatile("movmatrix.sync.aligned.m8n8.trans.b16 %0, %1;" : "=r"(t3) : "r"(frag.regs[3]));
        frag.regs[0] = t0;
        frag.regs[1] = t1;
        frag.regs[2] = t2;
        frag.regs[3] = t3;
    }
    else if constexpr (M == 16 && N == 16 && K == 16 && R == MatrixUse::MatrixB)
    {
        uint32_t t0, t1, t2, t3;
        asm volatile("movmatrix.sync.aligned.m8n8.trans.b16 %0, %1;" : "=r"(t0) : "r"(frag.regs[0]));
        asm volatile("movmatrix.sync.aligned.m8n8.trans.b16 %0, %1;" : "=r"(t1) : "r"(frag.regs[1]));
        asm volatile("movmatrix.sync.aligned.m8n8.trans.b16 %0, %1;" : "=r"(t2) : "r"(frag.regs[2]));
        asm volatile("movmatrix.sync.aligned.m8n8.trans.b16 %0, %1;" : "=r"(t3) : "r"(frag.regs[3]));
        frag.regs[0] = t0;
        frag.regs[1] = t1;
        frag.regs[2] = t2;
        frag.regs[3] = t3;
    }
}
#endif // #if SLANG_CUDA_ENABLE_HALF

// ====================================================================================
// MMA m16n8k16 Multiply-Accumulate Helper
// PTX: mma.sync.aligned.m16n8k16.row.col.dtype.f16.f16.ctype d, a, b, c
// Layout is always row.col (handled in load/store, not here).
// ====================================================================================

#if SLANG_CUDA_ENABLE_HALF
// f16 input, f32 accumulator
struct Mma16n8k16_f16_f32
{
    __device__ static void eval(
        WmmaFragment<float, 16, 8, 16, MatrixUse::MatrixC>& d,
        const WmmaFragment<half, 16, 8, 16, MatrixUse::MatrixA>& a,
        const WmmaFragment<half, 16, 8, 16, MatrixUse::MatrixB>& b,
        const WmmaFragment<float, 16, 8, 16, MatrixUse::MatrixC>& c)
    {
        eval(d.regs, a.regs, b.regs, c.regs);
    }

    __device__ static void eval(unsigned* d, const unsigned* a, const unsigned* b, const unsigned* c)
    {
        asm volatile("mma.sync.aligned.m16n8k16.row.col.f32.f16.f16.f32 "
                     "{%0, %1, %2, %3}, "
                     "{%4, %5, %6, %7}, "
                     "{%8, %9}, "
                     "{%10, %11, %12, %13};\n"
                     : "=r"(d[0]), "=r"(d[1]), "=r"(d[2]), "=r"(d[3])
                     : "r"(a[0]), "r"(a[1]), "r"(a[2]), "r"(a[3]),
                       "r"(b[0]), "r"(b[1]),
                       "r"(c[0]), "r"(c[1]), "r"(c[2]), "r"(c[3]));
    }
};

// f16 input, f16 accumulator
struct Mma16n8k16_f16_f16
{
    __device__ static void eval(
        WmmaFragment<half, 16, 8, 16, MatrixUse::MatrixC>& d,
        const WmmaFragment<half, 16, 8, 16, MatrixUse::MatrixA>& a,
        const WmmaFragment<half, 16, 8, 16, MatrixUse::MatrixB>& b,
        const WmmaFragment<half, 16, 8, 16, MatrixUse::MatrixC>& c)
    {
        eval(d.regs, a.regs, b.regs, c.regs);
    }

    __device__ static void eval(unsigned* d, const unsigned* a, const unsigned* b, const unsigned* c)
    {
        asm volatile("mma.sync.aligned.m16n8k16.row.col.f16.f16.f16.f16 "
                     "{%0, %1}, "
                     "{%2, %3, %4, %5}, "
                     "{%6, %7}, "
                     "{%8, %9};\n"
                     : "=r"(d[0]), "=r"(d[1])
                     : "r"(a[0]), "r"(a[1]), "r"(a[2]), "r"(a[3]),
                       "r"(b[0]), "r"(b[1]),
                       "r"(c[0]), "r"(c[1]));
    }
};
#endif // #if SLANG_CUDA_ENABLE_HALF

template<typename AType, typename BType, typename CType>
struct Mma16n8k16Helper;

#if SLANG_CUDA_ENABLE_HALF
template<>
struct Mma16n8k16Helper<half, half, float>
{
    __device__ static void eval(
        WmmaFragment<float, 16, 8, 16, MatrixUse::MatrixC>& d,
        const WmmaFragment<half, 16, 8, 16, MatrixUse::MatrixA>& a,
        const WmmaFragment<half, 16, 8, 16, MatrixUse::MatrixB>& b,
        const WmmaFragment<float, 16, 8, 16, MatrixUse::MatrixC>& c)
    {
        Mma16n8k16_f16_f32::eval(d, a, b, c);
    }
    __device__ static void eval(unsigned* d, const unsigned* a, const unsigned* b, const unsigned* c)
    {
        Mma16n8k16_f16_f32::eval(d, a, b, c);
    }
};

template<>
struct Mma16n8k16Helper<half, half, half>
{
    __device__ static void eval(
        WmmaFragment<half, 16, 8, 16, MatrixUse::MatrixC>& d,
        const WmmaFragment<half, 16, 8, 16, MatrixUse::MatrixA>& a,
        const WmmaFragment<half, 16, 8, 16, MatrixUse::MatrixB>& b,
        const WmmaFragment<half, 16, 8, 16, MatrixUse::MatrixC>& c)
    {
        Mma16n8k16_f16_f16::eval(d, a, b, c);
    }
    __device__ static void eval(unsigned* d, const unsigned* a, const unsigned* b, const unsigned* c)
    {
        Mma16n8k16_f16_f16::eval(d, a, b, c);
    }
};
#endif // #if SLANG_CUDA_ENABLE_HALF

// ====================================================================================
// MMA Helper - Primary Template (dispatcher)
// ====================================================================================

template<
    typename AType,
    typename BType,
    typename CType,
    typename DType,
    ShapeCombination shape,
    Layout LayoutA,
    Layout LayoutB,
    bool saturatingAccumulation>
struct MMAHelper
{
    static constexpr int M = ShapeToMNK<shape>::M;
    static constexpr int N = ShapeToMNK<shape>::N;
    static constexpr int K = ShapeToMNK<shape>::K;

    __device__ static void eval(
        WmmaFragment<DType, M, N, K, MatrixUse::MatrixC>& d,
        const WmmaFragment<AType, M, N, K, MatrixUse::MatrixA>& a,
        const WmmaFragment<BType, M, N, K, MatrixUse::MatrixB>& b,
        const WmmaFragment<CType, M, N, K, MatrixUse::MatrixC>& c,
        bool saturate = false)
    {
        // Dispatch to appropriate helper based on input types
        if constexpr (sizeof(AType) == 2 && sizeof(BType) == 2)
        {
            // FP16 inputs: dispatch to Fp16MMAHelper
            Fp16MMAHelper<CType, DType, M, N, K, LayoutA, LayoutB>::eval(d, a, b, c);
        }
        else
        {
            // Integer inputs (int8/uint8): dispatch to IntegerMMAHelper
            IntegerMMAHelper<AType, BType, shape, LayoutA, LayoutB, saturatingAccumulation>::eval(
                d,
                a,
                b,
                c);
        }
    }
};

//
template<
    typename AType,
    typename BType,
    typename CType,
    typename DType,
    int M,
    int N,
    int K,
    bool saturatingAccumulation>
WmmaFragment<DType, M, N, K, MatrixC> __device__ coopMatMulAdd(
    WmmaFragment<AType, M, N, K, MatrixUse::MatrixA> matA,
    WmmaFragment<BType, M, N, K, MatrixUse::MatrixB> matB,
    WmmaFragment<CType, M, N, K, MatrixUse::MatrixC> matC)
{
    WmmaFragment<DType, M, N, K, MatrixC> matD;

    if constexpr (M == 16 && N == 16 && K == 16)
    {
        // m16n16k16 via 2x m16n8k16: lo half in lower regs, hi half in upper regs.
        constexpr int cRegsPerHalf = RegisterCount<CType, 16, 8, 16, MatrixUse::MatrixC>::value;
        Mma16n8k16Helper<AType, BType, CType>::eval(matD.regs,                matA.regs, matB.regs,     matC.regs);
        Mma16n8k16Helper<AType, BType, CType>::eval(matD.regs + cRegsPerHalf, matA.regs, matB.regs + 2, matC.regs + cRegsPerHalf);
    }
    else
    {
        constexpr ShapeCombination shape =
            (M == 8 && N == 32 && K == 16) ? ShapeCombination::m8n32k16
                                           : ShapeCombination::m32n8k16;

        uint32_t encodedLayout = (matA.m_layout == Layout::RowMajor ? 1 : 0) << 1 |
                                 (matB.m_layout == Layout::RowMajor ? 1 : 0);

        switch (encodedLayout)
        {
        case 0x3:
            MMAHelper<
                AType,
                BType,
                CType,
                DType,
                shape,
                Layout::RowMajor,
                Layout::RowMajor,
                saturatingAccumulation>::eval(matD, matA, matB, matC);
            break;
        case 0x2:
            MMAHelper<
                AType,
                BType,
                CType,
                DType,
                shape,
                Layout::RowMajor,
                Layout::ColMajor,
                saturatingAccumulation>::eval(matD, matA, matB, matC);
            break;
        case 0x01:
            MMAHelper<
                AType,
                BType,
                CType,
                DType,
                shape,
                Layout::ColMajor,
                Layout::RowMajor,
                saturatingAccumulation>::eval(matD, matA, matB, matC);
            break;
        case 0x00:
            MMAHelper<
                AType,
                BType,
                CType,
                DType,
                shape,
                Layout::ColMajor,
                Layout::ColMajor,
                saturatingAccumulation>::eval(matD, matA, matB, matC);
            break;
        }
    }

    return matD;
}

} // namespace Slang_CUDA_WMMA
#endif // #if (((__CUDACC_VER_MAJOR__ >=12)&&(__CUDACC_VER_MINOR__>=5)) || (CUDA_VERSION >= 12050))

#endif
