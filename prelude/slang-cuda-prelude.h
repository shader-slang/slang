
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
SLANG_CUDA_CALL float F32_rcp(float f) { return 1.0f / f; }
SLANG_CUDA_CALL float F32_sign(float f) { return ( f == 0.0f) ? f : (( f < 0.0f) ? -1.0f : 1.0f); } 
SLANG_CUDA_CALL float F32_saturate(float f) { return (f < 0.0f) ? 0.0f : (f > 1.0f) ? 1.0f : f; }
SLANG_CUDA_CALL float F32_frac(float f) { return f - floorf(f); }

SLANG_CUDA_CALL bool F32_isnan(float f) { return isnan(f); }
SLANG_CUDA_CALL bool F32_isfinite(float f) { return isfinite(f); }
SLANG_CUDA_CALL bool F32_isinf(float f) { return isinf(f); }

// Binary
SLANG_CUDA_CALL float F32_min(float a, float b) { return a < b ? a : b; }
SLANG_CUDA_CALL float F32_max(float a, float b) { return a > b ? a : b; }
SLANG_CUDA_CALL float F32_step(float a, float b) { return float(b >= a); }

// TODO(JS): 
// Note CUDA has ldexp, but it takes an integer for the exponent, it seems HLSL takes both as float
SLANG_CUDA_CALL float F32_ldexp(float m, float e) { return m * powf(2.0f, e); }

// Ternary 
SLANG_CUDA_CALL float F32_lerp(float x, float y, float s) { return x + s * (y - x); }
SLANG_CUDA_CALL void F32_sincos(float f, float& outSin, float& outCos) { sincosf(f, &outSin, &outCos); }
SLANG_CUDA_CALL float F32_smoothstep(float min, float max, float x) 
{
    const float t = x < min ? 0.0f : ((x > max) ? 1.0f : (x - min) / (max - min)); 
    return t * t * (3.0 - 2.0 * t);
}
SLANG_CUDA_CALL float F32_clamp(float x, float min, float max) { return ( x < min) ? min : ((x > max) ? max : x); }

SLANG_CUDA_CALL uint32_t F32_asuint(float f) { Union32 u; u.f = f; return u.u; }
SLANG_CUDA_CALL int32_t F32_asint(float f) { Union32 u; u.f = f; return u.i; }

// ----------------------------- F64 -----------------------------------------

// Unary 
SLANG_CUDA_CALL double F64_rcp(double f) { return 1.0 / f; }
SLANG_CUDA_CALL double F64_sign(double f) { return (f == 0.0) ? f : ((f < 0.0) ? -1.0 : 1.0); }
SLANG_CUDA_CALL double F64_saturate(double f) { return (f < 0.0) ? 0.0 : (f > 1.0) ? 1.0 : f; }
SLANG_CUDA_CALL double F64_frac(double f) { return f - floor(f); }

SLANG_CUDA_CALL bool F64_isnan(double f) { return isnan(f); }
SLANG_CUDA_CALL bool F64_isfinite(double f) { return isfinite(f); }
SLANG_CUDA_CALL bool F64_isinf(double f) { return isinf(f); }

// Binary
SLANG_CUDA_CALL double F64_min(double a, double b) { return a < b ? a : b; }
SLANG_CUDA_CALL double F64_max(double a, double b) { return a > b ? a : b; }
SLANG_CUDA_CALL double F64_step(double a, double b) { return double(b >= a); }

// TODO(JS): 
// Note CUDA has ldexp, but it takes an integer for the exponent, it seems HLSL takes both as float
SLANG_CUDA_CALL double F64_ldexp(double m, double e) { return m * pow(2.0, e); }

// Ternary 
SLANG_CUDA_CALL double F64_lerp(double x, double y, double s) { return x + s * (y - x); }
SLANG_CUDA_CALL void F64_sincos(double f, double& outSin, double& outCos) { sincos(f, &outSin, &outCos); }
SLANG_CUDA_CALL double F64_smoothstep(double min, double max, double x) 
{ 
    const double t = x < min ? 0.0 : ((x > max) ? 1.0 : (x - min) / (max - min)); 
    return t * t * (3.0 - 2.0 * t);
}
SLANG_CUDA_CALL double F64_clamp(double x, double min, double max) { return (x < min) ? min : ((x > max) ? max : x); }

SLANG_CUDA_CALL void F64_asuint(double d, uint32_t& low, uint32_t& hi)
{
    Union64 u;
    u.d = d;
    low = uint32_t(u.u);
    hi = uint32_t(u.u >> 32);
}

SLANG_CUDA_CALL void F64_asint(double d, int32_t& low, int32_t& hi)
{
    Union64 u;
    u.d = d;
    low = int32_t(u.u);
    hi = int32_t(u.u >> 32);
}

// ----------------------------- I32 -----------------------------------------

// Unary
SLANG_CUDA_CALL int32_t I32_abs(int32_t f) { return (f < 0) ? -f : f; }

// Binary
SLANG_CUDA_CALL int32_t I32_min(int32_t a, int32_t b) { return a < b ? a : b; }
SLANG_CUDA_CALL int32_t I32_max(int32_t a, int32_t b) { return a > b ? a : b; }

// Ternary 
SLANG_CUDA_CALL int32_t I32_clamp(int32_t x, int32_t min, int32_t max) { return ( x < min) ? min : ((x > max) ? max : x); }

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

// Ternary 
SLANG_CUDA_CALL uint32_t U32_clamp(uint32_t x, uint32_t min, uint32_t max) { return ( x < min) ? min : ((x > max) ? max : x); }

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

SLANG_CUDA_CALL int64_t I64_clamp(int64_t x, int64_t min, int64_t max) { return ( x < min) ? min : ((x > max) ? max : x); }

// ----------------------------- U64 -----------------------------------------

SLANG_CUDA_CALL int64_t U64_abs(uint64_t f) { return f; }

SLANG_CUDA_CALL int64_t U64_min(uint64_t a, uint64_t b) { return a < b ? a : b; }
SLANG_CUDA_CALL int64_t U64_max(uint64_t a, uint64_t b) { return a > b ? a : b; }

SLANG_CUDA_CALL int64_t U64_clamp(uint64_t x, uint64_t min, uint64_t max) { return ( x < min) ? min : ((x > max) ? max : x); }

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
    SLANG_CUDA_CALL void GetDimensions(uint32_t& outNumStructs, uint32_t& outStride) { outNumStructs = uint32_t(count); outStride = uint32_t(sizeof(T)); }
  
    T* data;
    size_t count;
};

template <typename T>
struct StructuredBuffer
{
    SLANG_CUDA_CALL const T& operator[](size_t index) const { SLANG_CUDA_BOUND_CHECK(index, count); return data[index]; }
    SLANG_CUDA_CALL const T& Load(size_t index) const { SLANG_CUDA_BOUND_CHECK(index, count); return data[index]; }
    SLANG_CUDA_CALL void GetDimensions(uint32_t& outNumStructs, uint32_t& outStride) { outNumStructs = uint32_t(count); outStride = uint32_t(sizeof(T)); }
    
    T* data;
    size_t count;
};

    
// Missing  Load(_In_  int  Location, _Out_ uint Status);
struct ByteAddressBuffer
{
    SLANG_CUDA_CALL void GetDimensions(uint32_t& outDim) const { outDim = uint32_t(sizeInBytes); }
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
    
    const uint32_t* data;
    size_t sizeInBytes;  //< Must be multiple of 4
};

// https://docs.microsoft.com/en-us/windows/win32/direct3dhlsl/sm5-object-rwbyteaddressbuffer
// Missing support for Atomic operations 
// Missing support for Load with status
struct RWByteAddressBuffer
{
    SLANG_CUDA_CALL void GetDimensions(uint32_t& outDim) const { outDim = uint32_t(sizeInBytes); }
    
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
// To get the right results we need to use the __activemask() within _ballot_sync it seems.
// 
// Also note that __all_sync and __any_sync are listed with __ballot_sync. That if they have a similar synchronizing behavior
// we can use __activemask() there (instead of _getConvergedMask), because they will converge too. 
__forceinline__ __device__ int _getConvergedMask()
{
    //return __activemask(); 
    //return __ballot_sync(SLANG_CUDA_WARP_MASK, true);
    return __ballot_sync(__activemask(), true);
}

// Return mask of all the lanes less than the current lane
__forceinline__ __device__ int _getLaneLtMask()
{
    return (int(1) << _getLaneId()) - 1;
}    

// Note! Note will return true if mask is 0, but thats okay, because there must be one
// lane active to execute anything
__inline__ __device__ bool _waveIsSingleLane(int mask)
{
    return (mask & (mask - 1)) == 0;
}

// Returns the power of 2 size of run of set bits. Returns 0 if not a suitable run.
__inline__ __device__ int _waveCalcPow2Offset(int mask)
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
    const int mask = __activemask();
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
};

template <typename T>
struct WaveOpAdd
{
    __inline__ __device__ static T getInitial(T a) { return 0; }
    __inline__ __device__ static T doOp(T a, T b) { return a + b; }
};

template <typename T>
struct WaveOpMul
{
    __inline__ __device__ static T getInitial(T a) { return T(1); }
    __inline__ __device__ static T doOp(T a, T b) { return a * b; }
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
__device__ T _waveReduceScalar(T val)
{
    // The shuffles appear to converge on set bits, so it appears ok to use __activemask()
    //const int mask = _getConvergedMask();
    const int mask = __activemask();
    
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
            /* Get the sourceLane */
            const int srcLane = __ffs(laneBit) - 1;
            /* Broadcast (can also broadcast to self) */
            result = INTF::doOp(result, __shfl_sync(mask, val, srcLane));
            remaining &= ~laneBit;
        }
        return result;
    }
    return val;
}


// Multiple values
template <typename INTF, typename T, size_t COUNT>
__device__ void _waveReduceMultiple(T* val)
{
    // The shuffles appear to converge on set bits, so it appears ok to use __activemask()
    //const int mask = _getConvergedMask();
    const int mask = __activemask();
    
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
            /* Get the sourceLane */
            const int srcLane = __ffs(laneBit) - 1;
            /* Broadcast (can also broadcast to self) */
            for (size_t i = 0; i < COUNT; ++i)
            {
                val[i] = INTF::doOp(val[i], __shfl_sync(mask, originalVal[i], srcLane));
            }
            remaining &= ~laneBit;
        }
    }
}

template <typename INTF, typename T>
__device__ void _waveReduceMultiple(T* val)
{
    typedef typename ElementTypeTrait<T>::Type ElemType;    
    _waveReduceMultiple<INTF, ElemType, sizeof(T) / sizeof(ElemType)>((ElemType*)val);
}

template <typename T>
__inline__ __device__  T _waveOr(T val) { return _waveReduceScalar<WaveOpOr<T>, T>(val); }

template <typename T>
__inline__ __device__ T _waveAnd(T val) { return _waveReduceScalar<WaveOpAnd<T>, T>(val); }

template <typename T>
__inline__ __device__ T _waveXor(T val) { return _waveReduceScalar<WaveOpXor<T>, T>(val); }

template <typename T>
__inline__ __device__ T _waveProduct(T val) { return _waveReduceScalar<WaveOpMul<T>, T>(val); }

template <typename T>
__inline__ __device__ T _waveSum(T val) { return _waveReduceScalar<WaveOpAdd<T>, T>(val); }

template <typename T>
__inline__ __device__ T _waveMin(T val) { return _waveReduceScalar<WaveOpMin<T>, T>(val); }

template <typename T>
__inline__ __device__ T _waveMax(T val) { return _waveReduceScalar<WaveOpMax<T>, T>(val); }


// Multiple

template <typename T>
__inline__ __device__  T _waveOrMultiple(T val) { typedef typename ElementTypeTrait<T>::Type ElemType; _waveReduceMultiple<WaveOpOr<ElemType> >(&val); return val; }

template <typename T>
__inline__ __device__  T _waveAndMultiple(T val) { typedef typename ElementTypeTrait<T>::Type ElemType; _waveReduceMultiple<WaveOpAnd<ElemType> >(&val); return val; }

template <typename T>
__inline__ __device__  T _waveXorMultiple(T val) { typedef typename ElementTypeTrait<T>::Type ElemType; _waveReduceMultiple<WaveOpXor<ElemType> >(&val); return val; }

template <typename T>
__inline__ __device__  T _waveProductMultiple(T val) { typedef typename ElementTypeTrait<T>::Type ElemType; _waveReduceMultiple<WaveOpMul<ElemType> >(&val); return val; }

template <typename T>
__inline__ __device__  T _waveSumMultiple(T val) { typedef typename ElementTypeTrait<T>::Type ElemType; _waveReduceMultiple<WaveOpAdd<ElemType> >(&val); return val; }

template <typename T>
__inline__ __device__  T _waveMinMultiple(T val) { typedef typename ElementTypeTrait<T>::Type ElemType; _waveReduceMultiple<WaveOpMin<ElemType> >(&val); return val; }

template <typename T>
__inline__ __device__  T _waveMaxMultiple(T val) { typedef typename ElementTypeTrait<T>::Type ElemType; _waveReduceMultiple<WaveOpMax<ElemType> >(&val); return val; }


template <typename T>
__inline__ __device__ bool _waveAllEqual(T val) 
{
    // __match_all_sync is a synchronises so can use __activemask()
    const int mask = __activemask();
    int pred;
    __match_all_sync(mask, val, &pred);
    return pred != 0;
}

template <typename T>
__inline__ __device__ bool _waveAllEqualMultiple(T inVal) 
{
    typedef typename ElementTypeTrait<T>::Type ElemType;
    const size_t count = sizeof(T) / sizeof(ElemType);
    
    // __match_all_sync is a synchronises so can use __activemask()
    const int mask = __activemask();
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
__inline__ __device__ T _waveReadFirst(T val) 
{
    const int mask = __activemask();
    const int lowestLaneId = __ffs(mask) - 1;
    return __shfl_sync(mask, val, lowestLaneId);   
}

template <typename T>
__inline__ __device__ T _waveReadFirstMultiple(T inVal) 
{
    typedef typename ElementTypeTrait<T>::Type ElemType;
    const size_t count = sizeof(T) / sizeof(ElemType);
    
    T outVal;
    
    const ElemType* src = (const ElemType*)&inVal;
    ElemType* dst = (ElemType*)&outVal;
    
    const int mask = __activemask();
    const int lowestLaneId = __ffs(mask) - 1;
    
    for (size_t i = 0; i < count; ++i)
    {
        dst[i] = __shfl_sync(mask, src[i], lowestLaneId);   
    }
    
    return outVal;
}

template <typename T>
__inline__ __device__ T _waveReadLaneAtMultiple(T inVal, int lane)
{
    typedef typename ElementTypeTrait<T>::Type ElemType;
    const size_t count = sizeof(T) / sizeof(ElemType);
    
    T outVal;
    
    const ElemType* src = (const ElemType*)&inVal;
    ElemType* dst = (ElemType*)&outVal;
    
    const int mask = __activemask();
    
    for (size_t i = 0; i < count; ++i)
    {
        dst[i] = __shfl_sync(mask, src[i], lane);   
    }
    
    return outVal;
}

__device__ int _wavePrefixSum(int val)
{
    const int mask = __activemask();
    const int offsetSize = _waveCalcPow2Offset(mask);
    
    const int laneId = _getLaneId();
    if (offsetSize > 0)
    {    
        int sum = val;
        for (int i = 1; i < offsetSize; i += i) 
        {
            const int readVal = __shfl_up_sync(mask, sum, i, offsetSize);
            if (laneId >= i)
            {
                sum += readVal;
            }
        }
        return sum - val;
    }
    else 
    {
        int result = 0;
        int remaining = mask;
        while (remaining)
        {
            const int laneBit = remaining & -remaining;
            // Get the sourceLane 
            const int srcLane = __ffs(laneBit) - 1;
            // Broadcast (can also broadcast to self) 
            int readValue = __shfl_sync(mask, val, srcLane);
            // Only accumulate if srcLane is less than this lane
            if (srcLane < laneId)
            {
                result += readValue;
            }
            remaining &= ~laneBit;
        }
        return result;
    }
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */


/* Type that defines the uniform entry point params. The actual content of this type is dependent on the entry point parameters, and can be
found via reflection or defined such that it matches the shader appropriately.
*/
struct UniformEntryPointParams;
struct UniformState;
