#ifndef SLANG_PRELUDE_SCALAR_INTRINSICS_H
#define SLANG_PRELUDE_SCALAR_INTRINSICS_H

#include "../slang.h"

#if SLANG_PROCESSOR_X86_64 && SLANG_VC
// If we have visual studio and 64 bit processor, we can assume we have popcnt, and can include x86 intrinsics
#   include <intrin.h>
#endif

#ifdef SLANG_PRELUDE_NAMESPACE
namespace SLANG_PRELUDE_NAMESPACE {
#endif

#ifndef SLANG_PRELUDE_PI
#   define SLANG_PRELUDE_PI           3.14159265358979323846
#endif

// ----------------------------- F32 -----------------------------------------

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

// Helpers
SLANG_FORCE_INLINE float F32_calcSafeRadians(float radians)
{
    // Put 0 to 2pi cycles to cycle around 0 to 1 
	float a = radians * (1.0f /  float(SLANG_PRELUDE_PI * 2));
    // Get truncated fraction, as value in  0 - 1 range
    a = a - ::floorf(a);
    // Convert back to 0 - 2pi range
	return (a * float(SLANG_PRELUDE_PI * 2));
}

// Unary 
SLANG_FORCE_INLINE float F32_ceil(float f) { return ::ceilf(f); }
SLANG_FORCE_INLINE float F32_floor(float f) { return ::floorf(f); }
SLANG_FORCE_INLINE float F32_round(float f) { return ::roundf(f); }
SLANG_FORCE_INLINE float F32_sin(float f) { return ::sinf(f); }
SLANG_FORCE_INLINE float F32_cos(float f) { return ::cosf(f); }
SLANG_FORCE_INLINE float F32_tan(float f) { return ::tanf(f); }
SLANG_FORCE_INLINE float F32_asin(float f) { return ::asinf(f); }
SLANG_FORCE_INLINE float F32_acos(float f) { return ::acosf(f); }
SLANG_FORCE_INLINE float F32_atan(float f) { return ::atanf(f); }
SLANG_FORCE_INLINE float F32_sinh(float f) { return ::sinhf(f); }
SLANG_FORCE_INLINE float F32_cosh(float f) { return ::coshf(f); }
SLANG_FORCE_INLINE float F32_tanh(float f) { return ::tanhf(f); }
SLANG_FORCE_INLINE float F32_log2(float f) { return ::log2f(f); }
SLANG_FORCE_INLINE float F32_log(float f) { return ::logf(f); }
SLANG_FORCE_INLINE float F32_log10(float f) { return ::log10f(f); }
SLANG_FORCE_INLINE float F32_exp2(float f) { return ::exp2f(f); }
SLANG_FORCE_INLINE float F32_exp(float f) { return ::expf(f); }
SLANG_FORCE_INLINE float F32_abs(float f) { return ::fabsf(f); }
SLANG_FORCE_INLINE float F32_trunc(float f) { return ::truncf(f); }
SLANG_FORCE_INLINE float F32_sqrt(float f) { return ::sqrtf(f); }
SLANG_FORCE_INLINE float F32_rsqrt(float f) { return 1.0f / F32_sqrt(f); }
SLANG_FORCE_INLINE float F32_sign(float f) { return ( f == 0.0f) ? f : (( f < 0.0f) ? -1.0f : 1.0f); } 
SLANG_FORCE_INLINE float F32_frac(float f) { return f - F32_floor(f); }

SLANG_FORCE_INLINE bool F32_isnan(float f) { return SLANG_PRELUDE_STD isnan(f); }
SLANG_FORCE_INLINE bool F32_isfinite(float f) { return SLANG_PRELUDE_STD isfinite(f); }
SLANG_FORCE_INLINE bool F32_isinf(float f) { return SLANG_PRELUDE_STD isinf(f); }

// Binary
SLANG_FORCE_INLINE float F32_min(float a, float b) { return ::fminf(a, b); }
SLANG_FORCE_INLINE float F32_max(float a, float b) { return ::fmaxf(a, b); }
SLANG_FORCE_INLINE float F32_pow(float a, float b) { return ::powf(a, b); }
SLANG_FORCE_INLINE float F32_fmod(float a, float b) { return ::fmodf(a, b); }
SLANG_FORCE_INLINE float F32_remainder(float a, float b) { return ::remainderf(a, b); }
SLANG_FORCE_INLINE float F32_atan2(float a, float b) { return float(::atan2(a, b)); }

SLANG_FORCE_INLINE float F32_frexp(float x, float* e)
{
    int ei;
    float m = ::frexpf(x, &ei);
    *e = float(ei);
    return m;
}
SLANG_FORCE_INLINE float F32_modf(float x, float* ip)
{
    return ::modff(x, ip);
}

SLANG_FORCE_INLINE uint32_t F32_asuint(float f) { Union32 u; u.f = f; return u.u; }
SLANG_FORCE_INLINE int32_t F32_asint(float f) { Union32 u; u.f = f; return u.i; }

// Ternary
SLANG_FORCE_INLINE float F32_fma(float a, float b, float c) { return ::fmaf(a, b, c); }

// ----------------------------- F64 -----------------------------------------

SLANG_FORCE_INLINE double F64_calcSafeRadians(double radians)
{
    // Put 0 to 2pi cycles to cycle around 0 to 1 
	double a = radians * (1.0f /  (SLANG_PRELUDE_PI * 2));
    // Get truncated fraction, as value in  0 - 1 range
    a = a - ::floor(a);
    // Convert back to 0 - 2pi range
	return (a * (SLANG_PRELUDE_PI * 2));
}

// Unary 
SLANG_FORCE_INLINE double F64_ceil(double f) { return ::ceil(f); }
SLANG_FORCE_INLINE double F64_floor(double f) { return ::floor(f); }
SLANG_FORCE_INLINE double F64_round(double f) { return ::round(f); }
SLANG_FORCE_INLINE double F64_sin(double f) { return ::sin(f); }
SLANG_FORCE_INLINE double F64_cos(double f) { return ::cos(f); }
SLANG_FORCE_INLINE double F64_tan(double f) { return ::tan(f); }
SLANG_FORCE_INLINE double F64_asin(double f) { return ::asin(f); }
SLANG_FORCE_INLINE double F64_acos(double f) { return ::acos(f); }
SLANG_FORCE_INLINE double F64_atan(double f) { return ::atan(f); }
SLANG_FORCE_INLINE double F64_sinh(double f) { return ::sinh(f); }
SLANG_FORCE_INLINE double F64_cosh(double f) { return ::cosh(f); }
SLANG_FORCE_INLINE double F64_tanh(double f) { return ::tanh(f); }
SLANG_FORCE_INLINE double F64_log2(double f) { return ::log2(f); }
SLANG_FORCE_INLINE double F64_log(double f) { return ::log(f); }
SLANG_FORCE_INLINE double F64_log10(float f) { return ::log10(f); }
SLANG_FORCE_INLINE double F64_exp2(double f) { return ::exp2(f); }
SLANG_FORCE_INLINE double F64_exp(double f) { return ::exp(f); }
SLANG_FORCE_INLINE double F64_abs(double f) { return ::fabs(f); }
SLANG_FORCE_INLINE double F64_trunc(double f) { return ::trunc(f); }
SLANG_FORCE_INLINE double F64_sqrt(double f) { return ::sqrt(f); }
SLANG_FORCE_INLINE double F64_rsqrt(double f) { return 1.0 / F64_sqrt(f); }
SLANG_FORCE_INLINE double F64_sign(double f) { return (f == 0.0) ? f : ((f < 0.0) ? -1.0 : 1.0); }
SLANG_FORCE_INLINE double F64_frac(double f) { return f - F64_floor(f); }

SLANG_FORCE_INLINE bool F64_isnan(double f) { return SLANG_PRELUDE_STD isnan(f); }
SLANG_FORCE_INLINE bool F64_isfinite(double f) { return SLANG_PRELUDE_STD isfinite(f); }
SLANG_FORCE_INLINE bool F64_isinf(double f) { return SLANG_PRELUDE_STD isinf(f); }

// Binary
SLANG_FORCE_INLINE double F64_min(double a, double b) { return ::fmin(a, b); }
SLANG_FORCE_INLINE double F64_max(double a, double b) { return ::fmax(a, b); }
SLANG_FORCE_INLINE double F64_pow(double a, double b) { return ::pow(a, b); }
SLANG_FORCE_INLINE double F64_fmod(double a, double b) { return ::fmod(a, b); }
SLANG_FORCE_INLINE double F64_remainder(double a, double b) { return ::remainder(a, b); }
SLANG_FORCE_INLINE double F64_atan2(double a, double b) { return ::atan2(a, b); }

SLANG_FORCE_INLINE double F64_frexp(double x, double* e)
{
    int ei;
    double m = ::frexp(x, &ei);
    *e = float(ei);
    return m;
}

SLANG_FORCE_INLINE double F64_modf(double x, double* ip)
{
    return ::modf(x, ip);
}

SLANG_FORCE_INLINE void F64_asuint(double d, uint32_t* low, uint32_t* hi)
{
    Union64 u;
    u.d = d;
    *low = uint32_t(u.u);
    *hi = uint32_t(u.u >> 32);
}

SLANG_FORCE_INLINE void F64_asint(double d, int32_t* low, int32_t* hi)
{
    Union64 u;
    u.d = d;
    *low = int32_t(u.u);
    *hi = int32_t(u.u >> 32);
}

// Ternary
SLANG_FORCE_INLINE double F64_fma(double a, double b, double c) { return ::fma(a, b, c); }

// ----------------------------- I32 -----------------------------------------

SLANG_FORCE_INLINE int32_t I32_abs(int32_t f) { return (f < 0) ? -f : f; }

SLANG_FORCE_INLINE int32_t I32_min(int32_t a, int32_t b) { return a < b ? a : b; }
SLANG_FORCE_INLINE int32_t I32_max(int32_t a, int32_t b) { return a > b ? a : b; }

SLANG_FORCE_INLINE float I32_asfloat(int32_t x) { Union32 u; u.i = x; return u.f; }
SLANG_FORCE_INLINE uint32_t I32_asuint(int32_t x) { return uint32_t(x); }
SLANG_FORCE_INLINE double I32_asdouble(int32_t low, int32_t hi )
{
    Union64 u;
    u.u = (uint64_t(hi) << 32) | uint32_t(low);
    return u.d;
}

// ----------------------------- U32 -----------------------------------------

SLANG_FORCE_INLINE uint32_t U32_abs(uint32_t f) { return f; }

SLANG_FORCE_INLINE uint32_t U32_min(uint32_t a, uint32_t b) { return a < b ? a : b; }
SLANG_FORCE_INLINE uint32_t U32_max(uint32_t a, uint32_t b) { return a > b ? a : b; }

SLANG_FORCE_INLINE float U32_asfloat(uint32_t x) { Union32 u; u.u = x; return u.f; }
SLANG_FORCE_INLINE uint32_t U32_asint(int32_t x) { return uint32_t(x); } 

SLANG_FORCE_INLINE double U32_asdouble(uint32_t low, uint32_t hi)
{
    Union64 u;
    u.u = (uint64_t(hi) << 32) | low;
    return u.d;
}


SLANG_FORCE_INLINE uint32_t U32_countbits(uint32_t v)
{
#if SLANG_GCC_FAMILY    
    return __builtin_popcount(v);
#elif SLANG_PROCESSOR_X86_64 && SLANG_VC
    return __popcnt(v);
#else     
    uint32_t c = 0;
    while (v)
    {
        c++;
        v &= v - 1;
    }
    return c;
#endif
}

// ----------------------------- U64 -----------------------------------------

SLANG_FORCE_INLINE uint64_t U64_abs(uint64_t f) { return f; }

SLANG_FORCE_INLINE uint64_t U64_min(uint64_t a, uint64_t b) { return a < b ? a : b; }
SLANG_FORCE_INLINE uint64_t U64_max(uint64_t a, uint64_t b) { return a > b ? a : b; }

// TODO(JS): We don't define countbits for 64bit in stdlib currently.
// It's not clear from documentation if it should return 32 or 64 bits, if it exists. 
// 32 bits can always hold the result, and will be implicitly promoted. 
SLANG_FORCE_INLINE uint32_t U64_countbits(uint64_t v)
{
#if SLANG_GCC_FAMILY    
    return uint32_t(__builtin_popcountl(v));
#elif SLANG_PROCESSOR_X86_64 && SLANG_VC
    return uint32_t(__popcnt64(v));
#else     
    uint32_t c = 0;
    while (v)
    {
        c++;
        v &= v - 1;
    }
    return c;
#endif
}

// ----------------------------- I64 -----------------------------------------

SLANG_FORCE_INLINE int64_t I64_abs(int64_t f) { return (f < 0) ? -f : f; }

SLANG_FORCE_INLINE int64_t I64_min(int64_t a, int64_t b) { return a < b ? a : b; }
SLANG_FORCE_INLINE int64_t I64_max(int64_t a, int64_t b) { return a > b ? a : b; }


// ----------------------------- Interlocked ---------------------------------
#ifdef _WIN32
#include <intrin.h>
#endif

void InterlockedAdd(uint32_t* dest, uint32_t value, uint32_t* oldValue)
{
#ifdef _WIN32
    *oldValue = _InterlockedExchangeAdd((long*)dest, (long)value);
#else
    *oldValue = __sync_fetch_and_add(dest, value);
#endif
}
#ifdef SLANG_PRELUDE_NAMESPACE
} 
#endif

#endif
