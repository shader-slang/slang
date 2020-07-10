#ifdef SLANG_CUDA_ENABLE_OPTIX
#include <optix.h>
#endif


// Must be large enough to cause overflow and therefore infinity
#ifndef SLANG_INFINITY
#   define SLANG_INFINITY   ((float)(1e+300 * 1e+300))
#endif

// For now we'll disable any asserts in this prelude
#define SLANG_PRELUDE_ASSERT(x) 

#ifndef SLANG_CUDA_WARP_SIZE 
#   define SLANG_CUDA_WARP_SIZE 32
#endif

#define SLANG_CUDA_WARP_MASK (SLANG_CUDA_WARP_SIZE - 1)

//
#define SLANG_FORCE_INLINE inline

#define SLANG_CUDA_CALL __device__ 

#define SLANG_FORCE_INLINE inline
#define SLANG_INLINE inline

// Bound checks. Can be replaced by defining before including header. 
// NOTE! 
// The default behaviour, if out of bounds is to index 0. This is of course quite wrong - and different 
// behavior to hlsl typically. The problem here though is more around a write reference. That unless 
// some kind of proxy is used it is hard and/or slow to emulate the typical GPU behavior.

#ifndef SLANG_CUDA_BOUND_CHECK
#   define SLANG_CUDA_BOUND_CHECK(index, count) SLANG_PRELUDE_ASSERT(index < count); index = (index < count) ? index : 0; 
#endif

#ifndef SLANG_CUDA_BYTE_ADDRESS_BOUND_CHECK
#   define SLANG_CUDA_BYTE_ADDRESS_BOUND_CHECK(index, size, count) SLANG_PRELUDE_ASSERT(index + 4 <= sizeInBytes && (index & 3) == 0); index = (index + 4 <= sizeInBytes) ? index : 0; 
#endif    

// Here we don't have the index zeroing behavior, as such bounds checks are generally not on GPU targets either. 
#ifndef SLANG_CUDA_FIXED_ARRAY_BOUND_CHECK
#   define SLANG_CUDA_FIXED_ARRAY_BOUND_CHECK(index, count) SLANG_PRELUDE_ASSERT(index < count); 
#endif

 // This macro handles how out-of-range surface coordinates are handled; 
 // I can equal
 // cudaBoundaryModeClamp, in which case out-of-range coordinates are clamped to the valid range
 // cudaBoundaryModeZero, in which case out-of-range reads return zero and out-of-range writes are ignored
 // cudaBoundaryModeTrap, in which case out-of-range accesses cause the kernel execution to fail. 
 
#ifndef SLANG_CUDA_BOUNDARY_MODE
#   define SLANG_CUDA_BOUNDARY_MODE cudaBoundaryModeZero
#endif

struct TypeInfo
{
    size_t typeSize;
};

template <typename T, size_t SIZE>
struct FixedArray
{
    SLANG_CUDA_CALL const T& operator[](size_t index) const { SLANG_CUDA_FIXED_ARRAY_BOUND_CHECK(index, SIZE); return m_data[index]; }
    SLANG_CUDA_CALL T& operator[](size_t index) { SLANG_CUDA_FIXED_ARRAY_BOUND_CHECK(index, SIZE); return m_data[index]; }
    
    T m_data[SIZE];
};

// An array that has no specified size, becomes a 'Array'. This stores the size so it can potentially 
// do bounds checking.  
template <typename T>
struct Array
{
    SLANG_CUDA_CALL const T& operator[](size_t index) const { SLANG_CUDA_BOUND_CHECK(index, count); return data[index]; }
    SLANG_CUDA_CALL T& operator[](size_t index) { SLANG_CUDA_BOUND_CHECK(index, count); return data[index]; }
    
    T* data;
    size_t count;
};

// Typically defined in cuda.h, but we can't ship/rely on that, so just define here
typedef unsigned long long CUtexObject;                   
typedef unsigned long long CUsurfObject;                  

// On CUDA sampler state is actually bound up with the texture object. We have a SamplerState type, 
// backed as a pointer, to simplify code generation, with the downside that such a binding will take up 
// uniform space, even though it will have no effect. 
// TODO(JS): Consider ways to strip use of variables of this type so have no binding,
struct SamplerStateUnused;
typedef SamplerStateUnused* SamplerState;


// TODO(JS): Not clear yet if this can be handled on CUDA, by just ignoring.
// For now, just map to the index type. 
typedef size_t NonUniformResourceIndex;

// Code generator will generate the specific type
template <typename T, int ROWS, int COLS>
struct Matrix;

typedef bool bool1;
typedef int2 bool2;
typedef int3 bool3;
typedef int4 bool4; 


typedef signed char int8_t;
typedef short int16_t;
typedef int int32_t;
typedef long long int64_t;

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

typedef long long longlong;
typedef unsigned long long ulonglong;

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;

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

// ----------------------------- F32 -----------------------------------------

// Unary 
SLANG_CUDA_CALL float F32_ceil(float f) { return ::ceilf(f); }
SLANG_CUDA_CALL float F32_floor(float f) { return ::floorf(f); }
SLANG_CUDA_CALL float F32_round(float f) { return ::roundf(f); }
SLANG_CUDA_CALL float F32_sin(float f) { return ::sinf(f); }
SLANG_CUDA_CALL float F32_cos(float f) { return ::cosf(f); }
SLANG_CUDA_CALL void F32_sincos(float f, float* s, float* c) { ::sincosf(f, s, c); }
SLANG_CUDA_CALL float F32_tan(float f) { return ::tanf(f); }
SLANG_CUDA_CALL float F32_asin(float f) { return ::asinf(f); }
SLANG_CUDA_CALL float F32_acos(float f) { return ::acosf(f); }
SLANG_CUDA_CALL float F32_atan(float f) { return ::atanf(f); }
SLANG_CUDA_CALL float F32_sinh(float f) { return ::sinhf(f); }
SLANG_CUDA_CALL float F32_cosh(float f) { return ::coshf(f); }
SLANG_CUDA_CALL float F32_tanh(float f) { return ::tanhf(f); }
SLANG_CUDA_CALL float F32_log2(float f) { return ::log2f(f); }
SLANG_CUDA_CALL float F32_log(float f) { return ::logf(f); }
SLANG_CUDA_CALL float F32_log10(float f) { return ::log10f(f); }
SLANG_CUDA_CALL float F32_exp2(float f) { return ::exp2f(f); }
SLANG_CUDA_CALL float F32_exp(float f) { return ::expf(f); }
SLANG_CUDA_CALL float F32_abs(float f) { return ::fabsf(f); }
SLANG_CUDA_CALL float F32_trunc(float f) { return ::truncf(f); }
SLANG_CUDA_CALL float F32_sqrt(float f) { return ::sqrtf(f); }
SLANG_CUDA_CALL float F32_rsqrt(float f) { return ::rsqrtf(f); }
SLANG_CUDA_CALL float F32_sign(float f) { return ( f == 0.0f) ? f : (( f < 0.0f) ? -1.0f : 1.0f); } 
SLANG_CUDA_CALL float F32_frac(float f) { return f - F32_floor(f); }

SLANG_CUDA_CALL bool F32_isnan(float f) { return isnan(f); }
SLANG_CUDA_CALL bool F32_isfinite(float f) { return isfinite(f); }
SLANG_CUDA_CALL bool F32_isinf(float f) { return isinf(f); }

// Binary
SLANG_CUDA_CALL float F32_min(float a, float b) { return ::fminf(a, b); }
SLANG_CUDA_CALL float F32_max(float a, float b) { return ::fmaxf(a, b); }
SLANG_CUDA_CALL float F32_pow(float a, float b) { return ::powf(a, b); }
SLANG_CUDA_CALL float F32_fmod(float a, float b) { return ::fmodf(a, b); }
SLANG_CUDA_CALL float F32_remainder(float a, float b) { return ::remainderf(a, b); }
SLANG_CUDA_CALL float F32_atan2(float a, float b) { return float(::atan2(a, b)); }

SLANG_CUDA_CALL float F32_frexp(float x, float* e)
{
    int ei;
    float m = ::frexpf(x, &ei);
    *e = ei;
    return m;
}
SLANG_CUDA_CALL float F32_modf(float x, float* ip)
{
    return ::modff(x, ip);
}

SLANG_CUDA_CALL uint32_t F32_asuint(float f) { Union32 u; u.f = f; return u.u; }
SLANG_CUDA_CALL int32_t F32_asint(float f) { Union32 u; u.f = f; return u.i; }

// Ternary
SLANG_CUDA_CALL float F32_fma(float a, float b, float c) { return ::fmaf(a, b, c); }


// ----------------------------- F64 -----------------------------------------

// Unary 
SLANG_CUDA_CALL double F64_ceil(double f) { return ::ceil(f); }
SLANG_CUDA_CALL double F64_floor(double f) { return ::floor(f); }
SLANG_CUDA_CALL double F64_round(double f) { return ::round(f); }
SLANG_CUDA_CALL double F64_sin(double f) { return ::sin(f); }
SLANG_CUDA_CALL double F64_cos(double f) { return ::cos(f); }
SLANG_CUDA_CALL void F64_sincos(double f, double* s, double* c) { ::sincos(f, s, c); }
SLANG_CUDA_CALL double F64_tan(double f) { return ::tan(f); }
SLANG_CUDA_CALL double F64_asin(double f) { return ::asin(f); }
SLANG_CUDA_CALL double F64_acos(double f) { return ::acos(f); }
SLANG_CUDA_CALL double F64_atan(double f) { return ::atan(f); }
SLANG_CUDA_CALL double F64_sinh(double f) { return ::sinh(f); }
SLANG_CUDA_CALL double F64_cosh(double f) { return ::cosh(f); }
SLANG_CUDA_CALL double F64_tanh(double f) { return ::tanh(f); }
SLANG_CUDA_CALL double F64_log2(double f) { return ::log2(f); }
SLANG_CUDA_CALL double F64_log(double f) { return ::log(f); }
SLANG_CUDA_CALL double F64_log10(float f) { return ::log10(f); }
SLANG_CUDA_CALL double F64_exp2(double f) { return ::exp2(f); }
SLANG_CUDA_CALL double F64_exp(double f) { return ::exp(f); }
SLANG_CUDA_CALL double F64_abs(double f) { return ::fabs(f); }
SLANG_CUDA_CALL double F64_trunc(double f) { return ::trunc(f); }
SLANG_CUDA_CALL double F64_sqrt(double f) { return ::sqrt(f); }
SLANG_CUDA_CALL double F64_rsqrt(double f) { return ::rsqrt(f); }
SLANG_CUDA_CALL double F64_sign(double f) { return (f == 0.0) ? f : ((f < 0.0) ? -1.0 : 1.0); }
SLANG_CUDA_CALL double F64_frac(double f) { return f - F64_floor(f); }

SLANG_CUDA_CALL bool F64_isnan(double f) { return isnan(f); }
SLANG_CUDA_CALL bool F64_isfinite(double f) { return isfinite(f); }
SLANG_CUDA_CALL bool F64_isinf(double f) { return isinf(f); }

// Binary
SLANG_CUDA_CALL double F64_min(double a, double b) { return ::fmin(a, b); }
SLANG_CUDA_CALL double F64_max(double a, double b) { return ::fmax(a, b); }
SLANG_CUDA_CALL double F64_pow(double a, double b) { return ::pow(a, b); }
SLANG_CUDA_CALL double F64_fmod(double a, double b) { return ::fmod(a, b); }
SLANG_CUDA_CALL double F64_remainder(double a, double b) { return ::remainder(a, b); }
SLANG_CUDA_CALL double F64_atan2(double a, double b) { return ::atan2(a, b); }

SLANG_CUDA_CALL double F64_frexp(double x, double* e)
{
    int ei;
    double m = ::frexp(x, &ei);
    *e = ei;
    return m;
}
SLANG_CUDA_CALL double F64_modf(double x, double* ip)
{
    return ::modf(x, ip);
}

SLANG_CUDA_CALL void F64_asuint(double d, uint32_t* low, uint32_t* hi)
{
    Union64 u;
    u.d = d;
    *low = uint32_t(u.u);
    *hi = uint32_t(u.u >> 32);
}

SLANG_CUDA_CALL void F64_asint(double d, int32_t* low, int32_t* hi)
{
    Union64 u;
    u.d = d;
    *low = int32_t(u.u);
    *hi = int32_t(u.u >> 32);
}

// Ternary
SLANG_CUDA_CALL double F64_fma(double a, double b, double c) { return ::fma(a, b, c); }

// ----------------------------- I32 -----------------------------------------

// Unary
SLANG_CUDA_CALL int32_t I32_abs(int32_t f) { return (f < 0) ? -f : f; }

// Binary
SLANG_CUDA_CALL int32_t I32_min(int32_t a, int32_t b) { return a < b ? a : b; }
SLANG_CUDA_CALL int32_t I32_max(int32_t a, int32_t b) { return a > b ? a : b; }

SLANG_CUDA_CALL float I32_asfloat(int32_t x) { Union32 u; u.i = x; return u.f; }
SLANG_CUDA_CALL uint32_t I32_asuint(int32_t x) { return uint32_t(x); }
SLANG_CUDA_CALL double I32_asdouble(int32_t low, int32_t hi )
{
    Union64 u;
    u.u = (uint64_t(hi) << 32) | uint32_t(low);
    return u.d;
}

// ----------------------------- U32 -----------------------------------------

// Unary 
SLANG_CUDA_CALL uint32_t U32_abs(uint32_t f) { return f; }

// Binary
SLANG_CUDA_CALL uint32_t U32_min(uint32_t a, uint32_t b) { return a < b ? a : b; }
SLANG_CUDA_CALL uint32_t U32_max(uint32_t a, uint32_t b) { return a > b ? a : b; }

SLANG_CUDA_CALL float U32_asfloat(uint32_t x) { Union32 u; u.u = x; return u.f; }
SLANG_CUDA_CALL uint32_t U32_asint(int32_t x) { return uint32_t(x); } 

SLANG_CUDA_CALL double U32_asdouble(uint32_t low, uint32_t hi)
{
    Union64 u;
    u.u = (uint64_t(hi) << 32) | low;
    return u.d;
}

SLANG_CUDA_CALL uint32_t U32_countbits(uint32_t v)
{
    // https://docs.nvidia.com/cuda/cuda-math-api/group__CUDA__MATH__INTRINSIC__INT.html#group__CUDA__MATH__INTRINSIC__INT_1g43c9c7d2b9ebf202ff1ef5769989be46
    return __popc(v);
}


// ----------------------------- I64 -----------------------------------------

SLANG_CUDA_CALL int64_t I64_abs(int64_t f) { return (f < 0) ? -f : f; }

SLANG_CUDA_CALL int64_t I64_min(int64_t a, int64_t b) { return a < b ? a : b; }
SLANG_CUDA_CALL int64_t I64_max(int64_t a, int64_t b) { return a > b ? a : b; }

// ----------------------------- U64 -----------------------------------------

SLANG_CUDA_CALL int64_t U64_abs(uint64_t f) { return f; }

SLANG_CUDA_CALL int64_t U64_min(uint64_t a, uint64_t b) { return a < b ? a : b; }
SLANG_CUDA_CALL int64_t U64_max(uint64_t a, uint64_t b) { return a > b ? a : b; }

SLANG_CUDA_CALL uint32_t U64_countbits(uint64_t v)
{
    // https://docs.nvidia.com/cuda/cuda-math-api/group__CUDA__MATH__INTRINSIC__INT.html#group__CUDA__MATH__INTRINSIC__INT_1g43c9c7d2b9ebf202ff1ef5769989be46
    return __popcll(v);
}


// ----------------------------- ResourceType -----------------------------------------


// https://docs.microsoft.com/en-us/windows/win32/direct3dhlsl/sm5-object-structuredbuffer-getdimensions
// Missing  Load(_In_  int  Location, _Out_ uint Status);

template <typename T>
struct RWStructuredBuffer
{
    SLANG_CUDA_CALL T& operator[](size_t index) const { SLANG_CUDA_BOUND_CHECK(index, count); return data[index]; }
    SLANG_CUDA_CALL const T& Load(size_t index) const { SLANG_CUDA_BOUND_CHECK(index, count); return data[index]; }  
    SLANG_CUDA_CALL void GetDimensions(uint32_t* outNumStructs, uint32_t* outStride) { *outNumStructs = uint32_t(count); *outStride = uint32_t(sizeof(T)); }
  
    T* data;
    size_t count;
};

template <typename T>
struct StructuredBuffer
{
    SLANG_CUDA_CALL const T& operator[](size_t index) const { SLANG_CUDA_BOUND_CHECK(index, count); return data[index]; }
    SLANG_CUDA_CALL const T& Load(size_t index) const { SLANG_CUDA_BOUND_CHECK(index, count); return data[index]; }
    SLANG_CUDA_CALL void GetDimensions(uint32_t* outNumStructs, uint32_t* outStride) { *outNumStructs = uint32_t(count); *outStride = uint32_t(sizeof(T)); }
    
    T* data;
    size_t count;
};

    
// Missing  Load(_In_  int  Location, _Out_ uint Status);
struct ByteAddressBuffer
{
    SLANG_CUDA_CALL void GetDimensions(uint32_t* outDim) const { *outDim = uint32_t(sizeInBytes); }
    SLANG_CUDA_CALL uint32_t Load(size_t index) const 
    { 
        SLANG_CUDA_BYTE_ADDRESS_BOUND_CHECK(index, 4, sizeInBytes);
        return data[index >> 2]; 
    }
    SLANG_CUDA_CALL uint2 Load2(size_t index) const 
    { 
        SLANG_CUDA_BYTE_ADDRESS_BOUND_CHECK(index, 8, sizeInBytes); 
        const size_t dataIdx = index >> 2; 
        return uint2{data[dataIdx], data[dataIdx + 1]}; 
    }
    SLANG_CUDA_CALL uint3 Load3(size_t index) const 
    { 
        SLANG_CUDA_BYTE_ADDRESS_BOUND_CHECK(index, 12, sizeInBytes);
        const size_t dataIdx = index >> 2; 
        return uint3{data[dataIdx], data[dataIdx + 1], data[dataIdx + 2]}; 
    }
    SLANG_CUDA_CALL uint4 Load4(size_t index) const 
    { 
        SLANG_CUDA_BYTE_ADDRESS_BOUND_CHECK(index, 16, sizeInBytes);
        const size_t dataIdx = index >> 2; 
        return uint4{data[dataIdx], data[dataIdx + 1], data[dataIdx + 2], data[dataIdx + 3]}; 
    }
    template<typename T>
    SLANG_CUDA_CALL T Load(size_t offset) const
    {
        SLANG_PRELUDE_ASSERT(offset + sizeof(T) <= sizeInBytes && (offset & (alignof(T)-1)) == 0); 
        return *(T const*)((char*)data + offset);
    }
    
    const uint32_t* data;
    size_t sizeInBytes;  //< Must be multiple of 4
};

// https://docs.microsoft.com/en-us/windows/win32/direct3dhlsl/sm5-object-rwbyteaddressbuffer
// Missing support for Atomic operations 
// Missing support for Load with status
struct RWByteAddressBuffer
{
    SLANG_CUDA_CALL void GetDimensions(uint32_t* outDim) const { *outDim = uint32_t(sizeInBytes); }
    
    SLANG_CUDA_CALL uint32_t Load(size_t index) const 
    { 
        SLANG_CUDA_BYTE_ADDRESS_BOUND_CHECK(index, 4, sizeInBytes);
        return data[index >> 2]; 
    }
    SLANG_CUDA_CALL uint2 Load2(size_t index) const 
    { 
        SLANG_CUDA_BYTE_ADDRESS_BOUND_CHECK(index, 8, sizeInBytes);
        const size_t dataIdx = index >> 2; 
        return uint2{data[dataIdx], data[dataIdx + 1]}; 
    }
    SLANG_CUDA_CALL uint3 Load3(size_t index) const 
    { 
        SLANG_CUDA_BYTE_ADDRESS_BOUND_CHECK(index, 12, sizeInBytes);
        const size_t dataIdx = index >> 2; 
        return uint3{data[dataIdx], data[dataIdx + 1], data[dataIdx + 2]}; 
    }
    SLANG_CUDA_CALL uint4 Load4(size_t index) const 
    { 
        SLANG_CUDA_BYTE_ADDRESS_BOUND_CHECK(index, 16, sizeInBytes);
        const size_t dataIdx = index >> 2; 
        return uint4{data[dataIdx], data[dataIdx + 1], data[dataIdx + 2], data[dataIdx + 3]}; 
    }
    template<typename T>
    SLANG_CUDA_CALL T Load(size_t offset) const
    {
        SLANG_PRELUDE_ASSERT(offset + sizeof(T) <= sizeInBytes && (offset & (alignof(T)-1)) == 0); 
        return *(T const*)((char*)data + offset);
    }
    
    SLANG_CUDA_CALL void Store(size_t index, uint32_t v) const 
    { 
        SLANG_CUDA_BYTE_ADDRESS_BOUND_CHECK(index, 4, sizeInBytes);
        data[index >> 2] = v; 
    }
    SLANG_CUDA_CALL void Store2(size_t index, uint2 v) const 
    { 
        SLANG_CUDA_BYTE_ADDRESS_BOUND_CHECK(index, 8, sizeInBytes);
        const size_t dataIdx = index >> 2; 
        data[dataIdx + 0] = v.x;
        data[dataIdx + 1] = v.y;
    }
    SLANG_CUDA_CALL void Store3(size_t index, uint3 v) const 
    { 
        SLANG_CUDA_BYTE_ADDRESS_BOUND_CHECK(index, 12, sizeInBytes);
        const size_t dataIdx = index >> 2; 
        data[dataIdx + 0] = v.x;
        data[dataIdx + 1] = v.y;
        data[dataIdx + 2] = v.z;
    }
    SLANG_CUDA_CALL void Store4(size_t index, uint4 v) const 
    { 
        SLANG_CUDA_BYTE_ADDRESS_BOUND_CHECK(index, 16, sizeInBytes);
        const size_t dataIdx = index >> 2; 
        data[dataIdx + 0] = v.x;
        data[dataIdx + 1] = v.y;
        data[dataIdx + 2] = v.z;
        data[dataIdx + 3] = v.w;
    }
    template<typename T>
    SLANG_CUDA_CALL void Store(size_t offset, T const& value) const
    {
        SLANG_PRELUDE_ASSERT(offset + sizeof(T) <= sizeInBytes && (offset & (alignof(T)-1)) == 0); 
        *(T*)((char*)data + offset) = value;
    }
    
    uint32_t* data;
    size_t sizeInBytes; //< Must be multiple of 4 
};


// ---------------------- Wave --------------------------------------

// TODO(JS): It appears that cuda does not have a simple way to get a lane index. 
// 
// Another approach could be... 
// laneId = ((threadIdx.z * blockDim.y + threadIdx.y) * blockDim.x + threadIdx.x) & SLANG_CUDA_WARP_MASK
// If that is really true another way to do this, would be for code generator to add this function 
// with the [numthreads] baked in. 
// 
// For now I'll just assume you have a launch that makes the following correct if the kernel uses WaveGetLaneIndex()
#ifndef SLANG_USE_ASM_LANE_ID
 __forceinline__ __device__ uint32_t _getLaneId()
{
    // If the launch is (or I guess some multiple of the warp size) 
    // we try this mechanism, which is apparently faster. 
    return threadIdx.x & SLANG_CUDA_WARP_MASK;
}
#else
__forceinline__ __device__ uint32_t _getLaneId()
{
    // https://stackoverflow.com/questions/44337309/whats-the-most-efficient-way-to-calculate-the-warp-id-lane-id-in-a-1-d-grid#
    // This mechanism is not the fastest way to do it, and that is why the other mechanism 
    // is the default. But the other mechanism relies on a launch that makes the assumption 
    // true.
    unsigned ret; 
    asm volatile ("mov.u32 %0, %laneid;" : "=r"(ret));
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
// ```For __all_sync, __any_sync, and __ballot_sync, a mask must be passed that specifies the threads 
// participating in the call. A bit, representing the thread's lane ID, must be set for each participating thread 
// to ensure they are properly converged before the intrinsic is executed by the hardware. All active threads named 
// in mask must execute the same intrinsic with the same mask, or the result is undefined.```
//
// Currently there isn't a mechanism to correctly get the mask without it being passed through.
// Doing so will most likely require some changes to slang code generation to track masks, for now then we use
// _getActiveMask. 

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
__inline__ __device__ int _waveCalcPow2Offset(WarpMask mask)
{
    // This should be the most common case, so fast path it
    if (mask == SLANG_CUDA_WARP_MASK)
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
    //return (mask & 1 ) || ((mask & -mask) == (1 << _getLaneId()));
    
    // This mechanism is most similar to what was in an nVidia post, so assume it is prefered. 
    return (mask & 1 ) || ((__ffs(mask) - 1) == _getLaneId());
}

template <typename T>
struct WaveOpOr
{
    __inline__ __device__ static T getInitial(T a) { return 0; }
    __inline__ __device__ static T doOp(T a, T b) { return a | b; }
};

template <typename T>
struct WaveOpAnd
{
    __inline__ __device__ static T getInitial(T a) { return ~T(0); }
    __inline__ __device__ static T doOp(T a, T b) { return a & b; }
};

template <typename T>
struct WaveOpXor
{
    __inline__ __device__ static T getInitial(T a) { return 0; }
    __inline__ __device__ static T doOp(T a, T b) { return a ^ b; }
    __inline__ __device__ static T doInverse(T a, T b) { return a ^ b; }
};

template <typename T>
struct WaveOpAdd
{
    __inline__ __device__ static T getInitial(T a) { return 0; }
    __inline__ __device__ static T doOp(T a, T b) { return a + b; }
    __inline__ __device__ static T doInverse(T a, T b) { return a - b; }
};

template <typename T>
struct WaveOpMul
{
    __inline__ __device__ static T getInitial(T a) { return T(1); }
    __inline__ __device__ static T doOp(T a, T b) { return a * b; }
    // Using this inverse for int is probably undesirable - because in general it requires T to have more precision
    // There is also a performance aspect to it, where divides are generally significantly slower
    __inline__ __device__ static T doInverse(T a, T b) { return a / b; }
};

template <typename T>
struct WaveOpMax
{
    __inline__ __device__ static T getInitial(T a) { return a; }
    __inline__ __device__ static T doOp(T a, T b) { return a > b ? a : b; }
};

template <typename T>
struct WaveOpMin
{
    __inline__  __device__ static T getInitial(T a) { return a; }
    __inline__ __device__ static T doOp(T a, T b) { return a < b ? a : b; }
};

template <typename T>
struct ElementTypeTrait;

// Scalar
template <> struct ElementTypeTrait<int> { typedef int Type; };
template <> struct ElementTypeTrait<uint> { typedef uint Type; };
template <> struct ElementTypeTrait<float> { typedef float Type; };
template <> struct ElementTypeTrait<double> { typedef double Type; };
template <> struct ElementTypeTrait<uint64_t> { typedef uint64_t Type; };
template <> struct ElementTypeTrait<int64_t> { typedef int64_t Type; };

// Vector
template <> struct ElementTypeTrait<int1> { typedef int Type; };
template <> struct ElementTypeTrait<int2> { typedef int Type; };
template <> struct ElementTypeTrait<int3> { typedef int Type; };
template <> struct ElementTypeTrait<int4> { typedef int Type; };

template <> struct ElementTypeTrait<uint1> { typedef uint Type; };
template <> struct ElementTypeTrait<uint2> { typedef uint Type; };
template <> struct ElementTypeTrait<uint3> { typedef uint Type; };
template <> struct ElementTypeTrait<uint4> { typedef uint Type; };

template <> struct ElementTypeTrait<float1> { typedef float Type; };
template <> struct ElementTypeTrait<float2> { typedef float Type; };
template <> struct ElementTypeTrait<float3> { typedef float Type; };
template <> struct ElementTypeTrait<float4> { typedef float Type; };

template <> struct ElementTypeTrait<double1> { typedef double Type; };
template <> struct ElementTypeTrait<double2> { typedef double Type; };
template <> struct ElementTypeTrait<double3> { typedef double Type; };
template <> struct ElementTypeTrait<double4> { typedef double Type; };

// Matrix
template <typename T, int ROWS, int COLS> 
struct ElementTypeTrait<Matrix<T, ROWS, COLS> >  
{ 
    typedef T Type; 
};

// Scalar 
template <typename INTF, typename T>
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
template <typename INTF, typename T, size_t COUNT>
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

template <typename INTF, typename T>
__device__ void _waveReduceMultiple(WarpMask mask, T* val)
{
    typedef typename ElementTypeTrait<T>::Type ElemType;    
    _waveReduceMultiple<INTF, ElemType, sizeof(T) / sizeof(ElemType)>(mask, (ElemType*)val);
}

template <typename T>
__inline__ __device__  T _waveOr(WarpMask mask, T val) { return _waveReduceScalar<WaveOpOr<T>, T>(mask, val); }

template <typename T>
__inline__ __device__ T _waveAnd(WarpMask mask, T val) { return _waveReduceScalar<WaveOpAnd<T>, T>(mask, val); }

template <typename T>
__inline__ __device__ T _waveXor(WarpMask mask, T val) { return _waveReduceScalar<WaveOpXor<T>, T>(mask, val); }

template <typename T>
__inline__ __device__ T _waveProduct(WarpMask mask, T val) { return _waveReduceScalar<WaveOpMul<T>, T>(mask, val); }

template <typename T>
__inline__ __device__ T _waveSum(WarpMask mask, T val) { return _waveReduceScalar<WaveOpAdd<T>, T>(mask, val); }

template <typename T>
__inline__ __device__ T _waveMin(WarpMask mask, T val) { return _waveReduceScalar<WaveOpMin<T>, T>(mask, val); }

template <typename T>
__inline__ __device__ T _waveMax(WarpMask mask, T val) { return _waveReduceScalar<WaveOpMax<T>, T>(mask, val); }


// Multiple

template <typename T>
__inline__ __device__  T _waveOrMultiple(WarpMask mask, T val) { typedef typename ElementTypeTrait<T>::Type ElemType; _waveReduceMultiple<WaveOpOr<ElemType> >(mask, &val); return val; }

template <typename T>
__inline__ __device__  T _waveAndMultiple(WarpMask mask, T val) { typedef typename ElementTypeTrait<T>::Type ElemType; _waveReduceMultiple<WaveOpAnd<ElemType> >(mask, &val); return val; }

template <typename T>
__inline__ __device__  T _waveXorMultiple(WarpMask mask, T val) { typedef typename ElementTypeTrait<T>::Type ElemType; _waveReduceMultiple<WaveOpXor<ElemType> >(mask, &val); return val; }

template <typename T>
__inline__ __device__  T _waveProductMultiple(WarpMask mask, T val) { typedef typename ElementTypeTrait<T>::Type ElemType; _waveReduceMultiple<WaveOpMul<ElemType> >(mask, &val); return val; }

template <typename T>
__inline__ __device__  T _waveSumMultiple(WarpMask mask, T val) { typedef typename ElementTypeTrait<T>::Type ElemType; _waveReduceMultiple<WaveOpAdd<ElemType> >(mask, &val); return val; }

template <typename T>
__inline__ __device__  T _waveMinMultiple(WarpMask mask, T val) { typedef typename ElementTypeTrait<T>::Type ElemType; _waveReduceMultiple<WaveOpMin<ElemType> >(mask, &val); return val; }

template <typename T>
__inline__ __device__  T _waveMaxMultiple(WarpMask mask, T val) { typedef typename ElementTypeTrait<T>::Type ElemType; _waveReduceMultiple<WaveOpMax<ElemType> >(mask, &val); return val; }


template <typename T>
__inline__ __device__ bool _waveAllEqual(WarpMask mask, T val) 
{
    int pred;
    __match_all_sync(mask, val, &pred);
    return pred != 0;
}

template <typename T>
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

template <typename T>
__inline__ __device__ T _waveReadFirst(WarpMask mask, T val) 
{
    const int lowestLaneId = __ffs(mask) - 1;
    return __shfl_sync(mask, val, lowestLaneId);   
}

template <typename T>
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

template <typename T>
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

// Invertable means that when we get to the end of the reduce, we can remove val (to make exclusive), using 
// the inverse of the op.
template <typename INTF, typename T>
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
template <typename INTF, typename T>
__device__ T _wavePrefixScalar(WarpMask mask, T val)
{
    const int offsetSize = _waveCalcPow2Offset(mask);
    
    const int laneId = _getLaneId();
    T result = INTF::getInitial(val);           
    if (offsetSize > 0)
    {    
        // For transmitted value we will do it inclusively with this lanes value
        // For the result we do not include the lanes value. This means an extra multiply for each iteration
        // but means we don't need to have a divide at the end and also removes overflow issues in that scenario.
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


template <typename INTF, typename T, size_t COUNT>
__device__ T _waveOpCopy(T* dst, const T* src)
{
    for (size_t j = 0; j < COUNT; ++j)
    {
        dst[j] = src[j];
    }
}    


template <typename INTF, typename T, size_t COUNT>
__device__ T _waveOpDoInverse(T* inOut, const T* val)
{
    for (size_t j = 0; j < COUNT; ++j)
    {
        inOut[j] = INTF::doInverse(inOut[j], val[j]);
    }
}    

template <typename INTF, typename T, size_t COUNT>
__device__ T _waveOpSetInitial(T* out, const T* val)
{
    for (size_t j = 0; j < COUNT; ++j)
    {
        out[j] = INTF::getInitial(val[j]);
    }
} 

template <typename INTF, typename T, size_t COUNT>
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
 
template <typename INTF, typename T, size_t COUNT>
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
        // For the result we do not include the lanes value. This means an extra op for each iteration
        // but means we don't need to have a divide at the end and also removes overflow issues in that scenario.
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

template <typename T>
__inline__ __device__ T _wavePrefixProduct(WarpMask mask, T val) { return _wavePrefixScalar<WaveOpMul<T>, T>(mask, val); }

template <typename T>
__inline__ __device__ T _wavePrefixSum(WarpMask mask, T val) { return _wavePrefixInvertableScalar<WaveOpAdd<T>, T>(mask, val); }    

template <typename T>
__inline__ __device__ T _wavePrefixXor(WarpMask mask, T val) { return _wavePrefixInvertableScalar<WaveOpXor<T>, T>(mask, val); }    
    
template <typename T>
__inline__ __device__ T _wavePrefixOr(WarpMask mask, T val) { return _wavePrefixScalar<WaveOpOr<T>, T>(mask, val); }      
    
template <typename T>
__inline__ __device__ T _wavePrefixAnd(WarpMask mask, T val) { return _wavePrefixScalar<WaveOpAnd<T>, T>(mask, val); }      
    
    
template <typename T>
__inline__ __device__ T _wavePrefixProductMultiple(WarpMask mask, T val)  
{ 
    typedef typename ElementTypeTrait<T>::Type ElemType;    
    _wavePrefixInvertableMultiple<WaveOpMul<ElemType>, ElemType, sizeof(T) / sizeof(ElemType)>(mask, (ElemType*)&val);    
    return val;
}

template <typename T>
__inline__ __device__ T _wavePrefixSumMultiple(WarpMask mask, T val) 
{ 
    typedef typename ElementTypeTrait<T>::Type ElemType;    
    _wavePrefixInvertableMultiple<WaveOpAdd<ElemType>, ElemType, sizeof(T) / sizeof(ElemType)>(mask, (ElemType*)&val);    
    return val;
}

template <typename T>
__inline__ __device__ T _wavePrefixXorMultiple(WarpMask mask, T val)  
{ 
    typedef typename ElementTypeTrait<T>::Type ElemType;    
    _wavePrefixInvertableMultiple<WaveOpXor<ElemType>, ElemType, sizeof(T) / sizeof(ElemType)>(mask, (ElemType*)&val);    
    return val;
}

template <typename T>
__inline__ __device__ T _wavePrefixOrMultiple(WarpMask mask, T val) 
{ 
    typedef typename ElementTypeTrait<T>::Type ElemType;    
    _wavePrefixMultiple<WaveOpOr<ElemType>, ElemType, sizeof(T) / sizeof(ElemType)>(mask, (ElemType*)&val);    
    return val;
}

template <typename T>
__inline__ __device__ T _wavePrefixAndMultiple(WarpMask mask, T val)  
{ 
    typedef typename ElementTypeTrait<T>::Type ElemType;    
    _wavePrefixMultiple<WaveOpAnd<ElemType>, ElemType, sizeof(T) / sizeof(ElemType)>(mask, (ElemType*)&val);    
    return val;
}

template <typename T>
__inline__ __device__ uint4 _waveMatchScalar(WarpMask mask, T val) 
{
    int pred;
    return make_uint4(__match_all_sync(mask, val, &pred), 0, 0, 0);
}

template <typename T>
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

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */


/* Type that defines the uniform entry point params. The actual content of this type is dependent on the entry point parameters, and can be
found via reflection or defined such that it matches the shader appropriately.
*/
struct UniformEntryPointParams;
struct UniformState;
