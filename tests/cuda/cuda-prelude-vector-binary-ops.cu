// Meant to be compiled directly by offline nvcc, next to the cuda-prelude-vec1-make.cu step in
// ci-slang-test.yml; no slang-test directive on purpose, because slang-test's CUDA path uses NVRTC,
// which is the other prelude branch. Compiling is most of the test: the width dispatch in
// SLANG_CUDA_VECTOR_BINARY_BODY names .z and .w literally, so a body applied at the wrong width is
// a compile error here rather than a silent miscompile.
//
// nvcc -std=c++17 -arch=compute_80 -c tests/cuda/cuda-prelude-vector-binary-ops.cu -I prelude
//
// The static_asserts additionally pin the two type facts that make the direct component access
// better-defined than the byte-pointer accessor it replaced: char2's component is signed char
// regardless of whether plain char is signed, and longlong2's component is long long even where
// the prelude's own `longlong` typedef is long (Linux).

#define SLANG_CUDA_ENABLE_HALF 1
#define SLANG_CUDA_ENABLE_BF16 1
#define SLANG_CUDA_ENABLE_FP8 1
#include "slang-cuda-prelude.h"

#include <type_traits>

static_assert(
    !SLANG_CUDA_RTC,
    "this fixture must be compiled with offline nvcc, not NVRTC; it pins the non-RTC prelude "
    "branch, whose longlong typedef is platform-dependent");

static_assert(
    std::is_same<decltype(char2().x), signed char>::value,
    "char2's component is signed char; the operators must read it as that and not through a "
    "plain-char pointer, whose signedness is implementation-defined");

static_assert(
    std::is_same<decltype(longlong2().x), long long>::value,
    "longlong2's component is long long even where the prelude's longlong typedef is long, so "
    "component access must not go through a longlong* reinterpretation");

// The accessor casts to `longlong*`, so where `longlong` is not the component's own type that cast
// reads a `long long` object through a different type. Pinned as a two-way check because the
// mismatch is platform-dependent: it holds on LP64 (int64_t is long), and not on Windows.
#if defined(_WIN32)
static_assert(
    std::is_same<longlong, decltype(longlong2().x)>::value,
    "on Windows int64_t is long long, so the prelude's longlong alias should match the component");
#else
static_assert(
    !std::is_same<longlong, decltype(longlong2().x)>::value,
    "on LP64 the prelude's longlong alias is long while longlong2's component is long long; if "
    "these ever coincide, the aliasing argument in the operators' comment no longer applies here");
#endif

// Every operator the integer families instantiate the bodies with. Applied below to each of the
// nine integer element types at all three widths.
template<typename V, typename B>
__device__ void exerciseIntegerOps(V a, V b, V* out, B* outCompare)
{
    V value = a + b;
    value = value - b;
    value = value * b;
    value = value / b;
    value = value % b;
    value = value ^ b;
    value = value & b;
    value = value | b;
    value = value >> b;
    value = value << b;
    *out = value;

    B compare = a > b;
    compare = compare && (a < b);
    compare = compare || (a >= b);
    compare = compare && (a <= b);
    compare = compare || (a == b);
    compare = compare && (a != b);
    *outCompare = compare;
}

template<typename V, typename B>
__device__ void exerciseFloatOps(V a, V b, V* out, B* outCompare)
{
    V value = a + b;
    value = value - b;
    value = value * b;
    value = value / b;
    *out = value;

    B compare = a > b;
    compare = compare && (a < b);
    compare = compare || (a >= b);
    compare = compare && (a <= b);
    compare = compare || (a == b);
    compare = compare && (a != b);
    *outCompare = compare;
}

// One instantiation per (element type, width) that SLANG_CUDA_VECTOR_INT_OPS covers, so every
// integer definition the bodies generate is compiled: all nine element types at widths 2, 3 and 4.
// bool goes through the same operator set further below, where its own vector types are the operands
// rather than only the comparison results.
#define SLANG_TEST_INTEGER_VECTOR_OPS(T)                                        \
    __global__ void testIntegerVectorOps_##T(                                   \
        T##2* v2,                                                               \
        T##3* v3,                                                               \
        T##4* v4,                                                               \
        bool2* b2,                                                              \
        bool3* b3,                                                              \
        bool4* b4)                                                              \
    {                                                                           \
        exerciseIntegerOps(v2[0], v2[1], v2, b2);                               \
        exerciseIntegerOps(v3[0], v3[1], v3, b3);                               \
        exerciseIntegerOps(v4[0], v4[1], v4, b4);                               \
    }

SLANG_TEST_INTEGER_VECTOR_OPS(int)
SLANG_TEST_INTEGER_VECTOR_OPS(uint)
SLANG_TEST_INTEGER_VECTOR_OPS(short)
SLANG_TEST_INTEGER_VECTOR_OPS(ushort)
SLANG_TEST_INTEGER_VECTOR_OPS(char)
SLANG_TEST_INTEGER_VECTOR_OPS(uchar)
SLANG_TEST_INTEGER_VECTOR_OPS(longlong)
SLANG_TEST_INTEGER_VECTOR_OPS(ulonglong)

__global__ void testFloatVectorOps(
    float2* f2,
    float3* f3,
    double4* d4,
    __half3* h3,
    __half4* h4,
    bool2* b2,
    bool3* b3,
    bool4* b4)
{
    exerciseFloatOps(f2[0], f2[1], f2, b2);
    exerciseFloatOps(f3[0], f3[1], f3, b3);
    exerciseFloatOps(d4[0], d4[1], d4, b4);
    exerciseFloatOps(h3[0], h3[1], h3, b3);
    exerciseFloatOps(h4[0], h4[1], h4, b4);
}

// bool vectors are their own operand type, not just the comparison result type:
// SLANG_CUDA_VECTOR_INT_OPS(bool) instantiates the same bodies with T##n == bool##n, so the operand
// and result types coincide. Covered with the same operator set as the other integer families.
__global__ void testBoolVectorOps(bool2* b2, bool3* b3, bool4* b4)
{
    exerciseIntegerOps(b2[0], b2[1], b2, b2);
    exerciseIntegerOps(b3[0], b3[1], b3, b3);
    exerciseIntegerOps(b4[0], b4[1], b4, b4);
}

// __half2 keeps its own arithmetic operators, backed by the packed intrinsics; only its comparison
// and logical operators come from the width-dispatched bodies. Instantiating both sets together
// catches a change that made the two ambiguous, or let the generic macro shadow the packed
// arithmetic — either would be an overload-resolution error here.
__global__ void testHalf2ArithmeticAndCompareOperatorsResolve(__half2* h2, bool2* b2)
{
    __half2 value = h2[0] + h2[1];
    value = value - h2[1];
    value = value * h2[1];
    value = value / h2[1];
    h2[0] = value;

    bool2 compare = h2[0] > h2[1];
    compare = compare && (h2[0] != h2[1]);
    b2[0] = compare;
}
