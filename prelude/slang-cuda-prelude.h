
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

// TODO(JS): NOTE! These functions only work across all lanes. 
// Special handling will be needed if only some lanes are active.

__inline__ __device__ int _waveOr(int mask, int val) 
{
    for (int offset = SLANG_CUDA_WARP_SIZE / 2; offset > 0; offset /= 2) 
    {
        val = val | __shfl_xor_sync( mask, val, offset);
    }
    return val;
}

__inline__ __device__ int _waveAnd(int mask, int val) 
{
    for (int offset = SLANG_CUDA_WARP_SIZE / 2; offset > 0; offset /= 2) 
    {
        val = val & __shfl_xor_sync( mask, val, offset);
    }
    return val;
}

__inline__ __device__ int _waveXor(int mask, int val) 
{
    for (int offset = SLANG_CUDA_WARP_SIZE / 2; offset > 0; offset /= 2) 
    {
        val = val ^ __shfl_xor_sync( mask, val, offset);
    }
    return val;
}

__inline__ __device__ int _waveMin(int mask, int val) 
{    
    for (int offset = SLANG_CUDA_WARP_SIZE / 2; offset > 0; offset /= 2) 
    {
        int newVal = __shfl_xor_sync( mask, val, offset);
        val = (newVal < val) ? newVal : val;
    }
}

__inline__ __device__ int _waveMax(int mask, int val) 
{
    for (int offset = SLANG_CUDA_WARP_SIZE / 2; offset > 0; offset /= 2) 
    {
        int newVal = __shfl_xor_sync( mask, val, offset);
        val = (newVal > val) ? newVal : val;
    }
    return val;
}

__inline__ __device__ int _waveProduct(int mask, int val) 
{
    const int offsetSize = _waveCalcPow2Offset(mask);
    if (offsetSize > 0)
    {
        for (int offset = offsetSize >> 1; offset > 0; offset >>= 2) 
        {
            val *= __shfl_xor_sync( mask, val, offset);
        }
        return val;        
    }
    else if ((mask & (mask - 1)) == 0)
    {
        return val;
    }

    // Special case across arbitrary set bits
    return -1;
}

__inline__ __device__ int _waveSum(int mask, int val) 
{
    for (int offset = SLANG_CUDA_WARP_SIZE / 2; offset > 0; offset /= 2) 
    {
        val += __shfl_xor_sync( mask, val, offset);
    }
    return val;
}

__inline__ __device__ bool _waveAllEqual(int mask, int val) 
{
    int pred;
    __match_all_sync(mask, val, &pred);
    return pred != 0;
}

__inline__ __device__ int _waveReadFirst(int val) 
{
    const int mask = __activemask();
    const int lowestLaneId = __ffs(mask) - 1;
    return __shfl_sync(mask, val, lowestLaneId);   
}

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

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */


/* Type that defines the uniform entry point params. The actual content of this type is dependent on the entry point parameters, and can be
found via reflection or defined such that it matches the shader appropriately.
*/
struct UniformEntryPointParams;
struct UniformState;
