
// For now we'll disable any asserts in this prelude
#define SLANG_PRELUDE_ASSERT(x) 

//
#define SLANG_FORCE_INLINE inline

#define SLANG_CUDA_CALL __device__ 

#define SLANG_FORCE_INLINE inline
#define SLANG_INLINE inline

template <typename T, size_t SIZE>
struct FixedArray
{
    SLANG_CUDA_CALL const T& operator[](size_t index) const { SLANG_PRELUDE_ASSERT(index < SIZE); return m_data[index]; }
    SLANG_CUDA_CALL T& operator[](size_t index) { SLANG_PRELUDE_ASSERT(index < SIZE); return m_data[index]; }
    
    T m_data[SIZE];
};

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

// Binary
SLANG_CUDA_CALL float F32_min(float a, float b) { return a < b ? a : b; }
SLANG_CUDA_CALL float F32_max(float a, float b) { return a > b ? a : b; }
SLANG_CUDA_CALL float F32_step(float a, float b) { return float(a >= b); }

// Ternary 
SLANG_CUDA_CALL float F32_lerp(float x, float y, float s) { return x + s * (y - x); }
SLANG_CUDA_CALL void F32_sincos(float f, float& outSin, float& outCos) { sincosf(f, &outSin, &outCos); }
SLANG_CUDA_CALL float F32_smoothstep(float min, float max, float x) { return x < min ? min : ((x > max) ? max : x / (max - min)); }
SLANG_CUDA_CALL float F32_clamp(float x, float min, float max) { return ( x < min) ? min : ((x > max) ? max : x); }

SLANG_CUDA_CALL uint32_t F32_asuint(float f) { Union32 u; u.f = f; return u.u; }
SLANG_CUDA_CALL int32_t F32_asint(float f) { Union32 u; u.f = f; return u.i; }

// ----------------------------- F64 -----------------------------------------

// Unary 
SLANG_CUDA_CALL double F64_rcp(double f) { return 1.0 / f; }
SLANG_CUDA_CALL double F64_sign(double f) { return (f == 0.0) ? f : ((f < 0.0) ? -1.0 : 1.0); }
SLANG_CUDA_CALL double F64_saturate(double f) { return (f < 0.0) ? 0.0 : (f > 1.0) ? 1.0 : f; }
SLANG_CUDA_CALL double F64_frac(double f) { return f - floor(f); }

// Binary
SLANG_CUDA_CALL double F64_min(double a, double b) { return a < b ? a : b; }
SLANG_CUDA_CALL double F64_max(double a, double b) { return a > b ? a : b; }
SLANG_CUDA_CALL double F64_step(double a, double b) { return double(a >= b); }

// Ternary 
SLANG_CUDA_CALL double F64_lerp(double x, double y, double s) { return x + s * (y - x); }
SLANG_CUDA_CALL void F64_sincos(double f, double& outSin, double& outCos) { sincos(f, &outSin, &outCos); }
SLANG_CUDA_CALL double F64_smoothstep(double min, double max, double x) { return x < min ? min : ((x > max) ? max : x / (max - min)); }
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

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */


/* Type that defines the uniform entry point params. The actual content of this type is dependent on the entry point parameters, and can be
found via reflection or defined such that it matches the shader appropriately.
*/
struct UniformEntryPointParams;
struct UniformState;
