
#include <inttypes.h>
#include <math.h>
#include <inttypes.h>
#include <math.h>
#include <assert.h>

#ifndef M_PI
#   define M_PI           3.14159265358979323846
#endif

template <typename T, size_t SIZE>
struct Array
{
    const T& operator[](size_t index) const { assert(index < SIZE); return m_data[index]; }
    T& operator[](size_t index) { assert(index < SIZE); return m_data[index]; }
    
    T m_data[SIZE];
};

template <typename T>
struct RWStructuredBuffer
{
    T& operator[](size_t index) const { return data[index]; }
    
    T* data;
    size_t count;
};

// ----------------------------- F32 -----------------------------------------

union Union32 
{
    uint32_t u;
    int32_t i;
    float f;
};

// Helpers
float F32_calcSafeRadians(float radians)
{
	float a = radians * (1.0f /  M_PI);
	a = (a < 0.0f) ? (::ceilf(a) - a) : (a - ::floorf(a));
	return (a * M_PI);
}

// Unary 
float F32_ceil(float f) { return ::ceilf(f); }
float F32_floor(float f) { return ::floorf(f); }
float F32_sin(float f) { return ::sinf(F32_calcSafeRadians(f)); }
float F32_cos(float f) { return ::cosf(F32_calcSafeRadians(f)); }
float F32_tan(float f) { return ::tanf(f); }
float F32_asin(float f) { return ::asinf(f); }
float F32_acos(float f) { return ::acosf(f); }
float F32_atan(float f) { return ::atanf(f); }
float F32_log2(float f) { return ::log2f(f); }
float F32_exp2(float f) { return ::exp2f(f); }
float F32_exp(float f) { return ::expf(f); }
float F32_abs(float f) { return ::fabsf(f); }
float F32_trunc(float f) { return ::truncf(f); }
float F32_sqrt(float f) { return ::sqrtf(f); }
float F32_rsqrt(float f) { return 1.0f / F32_sqrt(f); }
float F32_rcp(float f) { return 1.0f / f; }
float F32_sign(float f) { return ( f == 0.0f) ? f : (( f < 0.0f) ? -1.0f : 1.0f); } 
float F32_saturate(float f) { return (f < 0.0f) ? 0.0f : (f > 1.0f) ? 1.0f : f; }
float F32_frac(float f) { return f - F32_floor(f); }
float F32_radians(float f) { return f * 0.01745329222f; }

// Binary
float F32_min(float a, float b) { return a < b ? a : b; }
float F32_max(float a, float b) { return a > b ? a : b; }
float F32_pow(float a, float b) { return ::powf(a, b); }
float F32_fmod(float a, float b) { return ::fmodf(a, b); }
float F32_step(float a, float b) { return float(a >= b); }
float F32_atan2(float a, float b) { return float(atan2(a, b)); }

// Ternary 
float F32_smoothstep(float min, float max, float x) { return x < min ? min : ((x > max) ? max : x / (max - min)); }
float F32_lerp(float x, float y, float s) { return x + s * (y - x); }
float F32_clamp(float x, float min, float max) { return ( x < min) ? min : ((x > max) ? max : x); }
void F32_sincos(float f, float& outSin, float& outCos) { outSin = F32_sin(f); outCos = F32_cos(f); }

uint32_t F32_asuint(float f) { Union32 u; u.f = f; return u.u; }
int32_t F32_asint(float f) { Union32 u; u.f = f; return u.i; }

// ----------------------------- I32 -----------------------------------------

int32_t I32_abs(int32_t f) { return (f < 0) ? -f : f; }

int32_t I32_min(int32_t a, int32_t b) { return a < b ? a : b; }
int32_t I32_max(int32_t a, int32_t b) { return a > b ? a : b; }

int32_t I32_clamp(int32_t x, int32_t min, int32_t max) { return ( x < min) ? min : ((x > max) ? max : x); }

float I32_asfloat(int32_t x) { Union32 u; u.i = x; return u.f; }

// ----------------------------- U32 -----------------------------------------

uint32_t U32_abs(uint32_t f) { return (f < 0) ? -f : f; }

uint32_t U32_min(uint32_t a, uint32_t b) { return a < b ? a : b; }
uint32_t U32_max(uint32_t a, uint32_t b) { return a > b ? a : b; }

uint32_t U32_clamp(uint32_t x, uint32_t min, uint32_t max) { return ( x < min) ? min : ((x > max) ? max : x); }

float U32_asfloat(uint32_t x) { Union32 u; u.u = x; return u.f; }
