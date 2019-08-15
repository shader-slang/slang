#ifndef SLANG_CPP_PRELUDE_H
#define SLANG_CPP_PRELUDE_H

/* --------------- START From slang.h  ----------------- */

#ifndef SLANG_COMPILER
#    define SLANG_COMPILER

/*
Compiler defines, see http://sourceforge.net/p/predef/wiki/Compilers/
NOTE that SLANG_VC holds the compiler version - not just 1 or 0
*/
#    if defined(_MSC_VER)
#        if _MSC_VER >= 1900
#            define SLANG_VC 14
#        elif _MSC_VER >= 1800
#            define SLANG_VC 12
#        elif _MSC_VER >= 1700
#            define SLANG_VC 11
#        elif _MSC_VER >= 1600
#            define SLANG_VC 10
#        elif _MSC_VER >= 1500
#            define SLANG_VC 9
#        else
#            error "unknown version of Visual C++ compiler"
#        endif
#    elif defined(__clang__)
#        define SLANG_CLANG 1
#    elif defined(__SNC__)
#        define SLANG_SNC 1
#    elif defined(__ghs__)
#        define SLANG_GHS 1
#    elif defined(__GNUC__) /* note: __clang__, __SNC__, or __ghs__ imply __GNUC__ */
#        define SLANG_GCC 1
#    else
#        error "unknown compiler"
#    endif
/*
Any compilers not detected by the above logic are now now explicitly zeroed out.
*/
#    ifndef SLANG_VC
#        define SLANG_VC 0
#    endif
#    ifndef SLANG_CLANG
#        define SLANG_CLANG 0
#    endif
#    ifndef SLANG_SNC
#        define SLANG_SNC 0
#    endif
#    ifndef SLANG_GHS
#        define SLANG_GHS 0
#    endif
#    ifndef SLANG_GCC
#        define SLANG_GCC 0
#    endif
#endif /* SLANG_COMPILER */

/*
The following section attempts to detect the target platform being compiled for.

If an application defines `SLANG_PLATFORM` before including this header,
they take responsibility for setting any compiler-dependent macros
used later in the file.

Most applications should not need to touch this section.
*/
#ifndef SLANG_PLATFORM
#    define SLANG_PLATFORM
/**
Operating system defines, see http://sourceforge.net/p/predef/wiki/OperatingSystems/
*/
#    if defined(WINAPI_FAMILY) && WINAPI_FAMILY == WINAPI_PARTITION_APP
#        define SLANG_WINRT 1 /* Windows Runtime, either on Windows RT or Windows 8 */
#    elif defined(XBOXONE)
#        define SLANG_XBOXONE 1
#    elif defined(_WIN64) /* note: XBOXONE implies _WIN64 */
#        define SLANG_WIN64 1
#    elif defined(_M_PPC)
#        define SLANG_X360 1
#    elif defined(_WIN32) /* note: _M_PPC implies _WIN32 */
#        define SLANG_WIN32 1
#    elif defined(__ANDROID__)
#        define SLANG_ANDROID 1
#    elif defined(__linux__) || defined(__CYGWIN__) /* note: __ANDROID__ implies __linux__ */
#        define SLANG_LINUX 1
#    elif defined(__APPLE__) && (defined(__arm__) || defined(__arm64__))
#        define SLANG_IOS 1
#    elif defined(__APPLE__)
#        define SLANG_OSX 1
#    elif defined(__CELLOS_LV2__)
#        define SLANG_PS3 1
#    elif defined(__ORBIS__)
#        define SLANG_PS4 1
#    elif defined(__SNC__) && defined(__arm__)
#        define SLANG_PSP2 1
#    elif defined(__ghs__)
#        define SLANG_WIIU 1
#    else
#        error "unknown target platform"
#    endif
/*
Any platforms not detected by the above logic are now now explicitly zeroed out.
*/
#    ifndef SLANG_WINRT
#        define SLANG_WINRT 0
#    endif
#    ifndef SLANG_XBOXONE
#        define SLANG_XBOXONE 0
#    endif
#    ifndef SLANG_WIN64
#        define SLANG_WIN64 0
#    endif
#    ifndef SLANG_X360
#        define SLANG_X360 0
#    endif
#    ifndef SLANG_WIN32
#        define SLANG_WIN32 0
#    endif
#    ifndef SLANG_ANDROID
#        define SLANG_ANDROID 0
#    endif
#    ifndef SLANG_LINUX
#        define SLANG_LINUX 0
#    endif
#    ifndef SLANG_IOS
#        define SLANG_IOS 0
#    endif
#    ifndef SLANG_OSX
#        define SLANG_OSX 0
#    endif
#    ifndef SLANG_PS3
#        define SLANG_PS3 0
#    endif
#    ifndef SLANG_PS4
#        define SLANG_PS4 0
#    endif
#    ifndef SLANG_PSP2
#        define SLANG_PSP2 0
#    endif
#    ifndef SLANG_WIIU
#        define SLANG_WIIU 0
#    endif
#endif /* SLANG_PLATFORM */

/* Shorthands for "families" of compilers/platforms */
#define SLANG_GCC_FAMILY (SLANG_CLANG || SLANG_SNC || SLANG_GHS || SLANG_GCC)
#define SLANG_WINDOWS_FAMILY (SLANG_WINRT || SLANG_WIN32 || SLANG_WIN64)
#define SLANG_MICROSOFT_FAMILY (SLANG_XBOXONE || SLANG_X360 || SLANG_WINDOWS_FAMILY)
#define SLANG_LINUX_FAMILY (SLANG_LINUX || SLANG_ANDROID)
#define SLANG_APPLE_FAMILY (SLANG_IOS || SLANG_OSX)                  /* equivalent to #if __APPLE__ */
#define SLANG_UNIX_FAMILY (SLANG_LINUX_FAMILY || SLANG_APPLE_FAMILY) /* shortcut for unix/posix platforms */

/* Macro for declaring if a method is no throw. Should be set before the return parameter. */
#ifndef SLANG_NO_THROW
#   if SLANG_WINDOWS_FAMILY && !defined(SLANG_DISABLE_EXCEPTIONS)
#       define SLANG_NO_THROW __declspec(nothrow)
#   endif
#endif
#ifndef SLANG_NO_THROW
#   define SLANG_NO_THROW
#endif

/* The `SLANG_STDCALL` and `SLANG_MCALL` defines are used to set the calling
convention for interface methods.
*/
#ifndef SLANG_STDCALL
#   if SLANG_MICROSOFT_FAMILY
#       define SLANG_STDCALL __stdcall
#   else
#       define SLANG_STDCALL
#   endif
#endif
#ifndef SLANG_MCALL
#   define SLANG_MCALL SLANG_STDCALL
#endif


#if !defined(SLANG_STATIC) && !defined(SLANG_STATIC)
    #define SLANG_DYNAMIC
#endif

#if defined(_MSC_VER)
#   define SLANG_DLL_EXPORT __declspec(dllexport)
#else
#   define SLANG_DLL_EXPORT __attribute__((__visibility__("default")))
#endif

#if defined(SLANG_DYNAMIC)
#   if defined(_MSC_VER)
#       ifdef SLANG_DYNAMIC_EXPORT
#           define SLANG_API SLANG_DLL_EXPORT
#       else
#           define SLANG_API __declspec(dllimport)
#       endif
#   else
        // TODO: need to consider compiler capabilities
//#     ifdef SLANG_DYNAMIC_EXPORT
#       define SLANG_API SLANG_DLL_EXPORT 
//#     endif
#   endif
#endif

#ifndef SLANG_API
#   define SLANG_API
#endif

// GCC Specific
#if SLANG_GCC_FAMILY
// This doesn't work on clang - because the typedef is seen as multiply defined, use the line numbered version defined later
#	if !defined(__clang__) && (defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 7)) || defined(__ORBIS__))
#		define SLANG_COMPILE_TIME_ASSERT(exp) typedef char SlangCompileTimeAssert_Dummy[(exp) ? 1 : -1] __attribute__((unused))
#	endif

#	define SLANG_NO_INLINE __attribute__((noinline))
#	define SLANG_FORCE_INLINE inline __attribute__((always_inline))
#   define SLANG_BREAKPOINT(id) __builtin_trap();
#	define SLANG_ALIGN_OF(T)	__alignof__(T)

// Use this macro instead of offsetof, because gcc produces warning if offsetof is used on a 
// non POD type, even though it produces the correct result
#   define SLANG_OFFSET_OF(T, ELEMENT) (size_t(&((T*)1)->ELEMENT) - 1)
#endif // SLANG_GCC_FAMILY

// Microsoft VC specific
#if SLANG_MICROSOFT_FAMILY
#	define SLANG_NO_INLINE __declspec(noinline)
#	define SLANG_FORCE_INLINE __forceinline
#	define SLANG_BREAKPOINT(id) __debugbreak();
#	define SLANG_ALIGN_OF(T) __alignof(T)

#   define SLANG_INT64(x) (x##i64)
#   define SLANG_UINT64(x) (x##ui64)
#endif // SLANG_MICROSOFT_FAMILY

#ifndef SLANG_FORCE_INLINE
#	define SLANG_FORCE_INLINE inline
#endif
#ifndef SLANG_NO_INLINE
#	define SLANG_NO_INLINE
#endif

#ifndef SLANG_COMPILE_TIME_ASSERT
#	define SLANG_COMPILE_TIME_ASSERT(exp) typedef char SLANG_CONCAT(SlangCompileTimeAssert,__LINE__)[(exp) ? 1 : -1]
#endif

#ifndef SLANG_OFFSET_OF
#	define SLANG_OFFSET_OF(X, Y) offsetof(X, Y)
#endif

#ifndef SLANG_BREAKPOINT
// Make it crash with a write to 0!
#   define SLANG_BREAKPOINT(id) (*((int*)0) = int(id));
#endif

// Use for getting the amount of members of a standard C array.
#define SLANG_COUNT_OF(x) (sizeof(x)/sizeof(x[0]))
/// SLANG_INLINE exists to have a way to inline consistent with SLANG_ALWAYS_INLINE
#define SLANG_INLINE inline

// Other defines
#define SLANG_STRINGIZE_HELPER(X) #X
#define SLANG_STRINGIZE(X) SLANG_STRINGIZE_HELPER(X)

#define SLANG_CONCAT_HELPER(X, Y) X##Y
#define SLANG_CONCAT(X, Y) SLANG_CONCAT_HELPER(X, Y)

#ifndef SLANG_UNUSED
#	define SLANG_UNUSED(v) (void)v;
#endif

// Used for doing constant literals
#ifndef SLANG_INT64
#	define SLANG_INT64(x) (x##ll)
#endif
#ifndef SLANG_UINT64
#	define SLANG_UINT64(x) (x##ull)
#endif


#ifdef __cplusplus
#   define SLANG_EXTERN_C extern "C"
#else
#   define SLANG_EXTERN_C
#endif

#ifdef __cplusplus
// C++ specific macros
// Clang
#if SLANG_CLANG
#    if (__clang_major__*10 + __clang_minor__) >= 33
#       define SLANG_HAS_MOVE_SEMANTICS 1
#       define SLANG_HAS_ENUM_CLASS 1
#       define SLANG_OVERRIDE override
#    endif
// Gcc
#elif SLANG_GCC_FAMILY
// Check for C++11
#		if (__cplusplus >= 201103L)
#			if (__GNUC__ * 100 + __GNUC_MINOR__) >= 405
#				define SLANG_HAS_MOVE_SEMANTICS 1
#			endif
#			if (__GNUC__ * 100 + __GNUC_MINOR__) >= 406
#				define SLANG_HAS_ENUM_CLASS 1
#			endif
#			if (__GNUC__ * 100 + __GNUC_MINOR__) >= 407
#				define SLANG_OVERRIDE override
#			endif
#		endif
#	endif // SLANG_GCC_FAMILY

// Visual Studio

#	if SLANG_VC
// C4481: nonstandard extension used: override specifier 'override'
#		if _MSC_VER < 1700
#			pragma warning(disable : 4481)
#		endif
#		define SLANG_OVERRIDE	override
#		if _MSC_VER >= 1600
#			define SLANG_HAS_MOVE_SEMANTICS 1
#		endif
#	    if _MSC_VER >= 1700
#		    define SLANG_HAS_ENUM_CLASS 1
#       endif

#   endif // SLANG_VC

// Set non set
#   ifndef SLANG_OVERRIDE
#	    define SLANG_OVERRIDE
#   endif
#   ifndef SLANG_HAS_ENUM_CLASS
#	    define SLANG_HAS_ENUM_CLASS 0
#   endif
#   ifndef SLANG_HAS_MOVE_SEMANTICS
#	    define SLANG_HAS_MOVE_SEMANTICS 0
#   endif

#endif // __cplusplus

/* Macros for detecting processor */
#if defined(_M_ARM) || defined(__ARM_EABI__)
// This is special case for nVidia tegra
#   define SLANG_PROCESSOR_ARM 1
#elif defined(__i386__) || defined(_M_IX86)
#   define SLANG_PROCESSOR_X86 1
#elif defined(_M_AMD64) || defined(_M_X64) || defined(__amd64) || defined(__x86_64)
#   define SLANG_PROCESSOR_X86_64 1
#elif defined(_PPC_) || defined(__ppc__) || defined(__POWERPC__) || defined(_M_PPC)
#   if defined(__powerpc64__) || defined(__ppc64__) || defined(__PPC64__) || defined(__64BIT__) || defined(_LP64) || defined(__LP64__)
#       define SLANG_PROCESSOR_POWER_PC_64 1
#   else
#       define SLANG_PROCESSOR_POWER_PC 1
#   endif
#elif defined(__arm__)
#   define SLANG_PROCESSOR_ARM 1
#elif defined(__aarch64__)
#   define SLANG_PROCESSOR_ARM_64 1
#endif 

#ifndef SLANG_PROCESSOR_ARM
#   define SLANG_PROCESSOR_ARM 0
#endif

#ifndef SLANG_PROCESSOR_ARM_64
#   define SLANG_PROCESSOR_ARM_64 0
#endif

#ifndef SLANG_PROCESSOR_X86
#   define SLANG_PROCESSOR_X86 0
#endif

#ifndef SLANG_PROCESSOR_X86_64
#   define SLANG_PROCESSOR_X86_64 0
#endif

#ifndef SLANG_PROCESSOR_POWER_PC
#   define SLANG_PROCESSOR_POWER_PC 0
#endif

#ifndef SLANG_PROCESSOR_POWER_PC_64
#   define SLANG_PROCESSOR_POWER_PC_64 0
#endif

// Processor families

#define SLANG_PROCESSOR_FAMILY_X86 (SLANG_PROCESSOR_X86_64 | SLANG_PROCESSOR_X86)
#define SLANG_PROCESSOR_FAMILY_ARM (SLANG_PROCESSOR_ARM | SLANG_PROCESSOR_ARM_64)
#define SLANG_PROCESSOR_FAMILY_POWER_PC (SLANG_PROCESSOR_POWER_PC_64 | SLANG_PROCESSOR_POWER_PC)

// Pointer size
#define SLANG_PTR_IS_64 (SLANG_PROCESSOR_ARM_64 | SLANG_PROCESSOR_X86_64 | SLANG_PROCESSOR_POWER_PC_64)
#define SLANG_PTR_IS_32 (SLANG_PTR_IS_64 ^ 1)

// Processor features
#if SLANG_PROCESSOR_FAMILY_X86
#   define SLANG_LITTLE_ENDIAN 1
#   define SLANG_UNALIGNED_ACCESS 1
#elif SLANG_PROCESSOR_FAMILY_ARM
#   if defined(__ARMEB__)
#       define SLANG_BIG_ENDIAN 1
#   else
#       define SLANG_LITTLE_ENDIAN 1
#   endif
#elif SLANG_PROCESSOR_FAMILY_POWER_PC
#       define SLANG_BIG_ENDIAN 1
#endif

#ifndef SLANG_LITTLE_ENDIAN
#   define SLANG_LITTLE_ENDIAN 0
#endif

#ifndef SLANG_BIG_ENDIAN
#   define SLANG_BIG_ENDIAN 0
#endif

#ifndef SLANG_UNALIGNED_ACCESS
#   define SLANG_UNALIGNED_ACCESS 0
#endif

// One endianess must be set
#if ((SLANG_BIG_ENDIAN | SLANG_LITTLE_ENDIAN) == 0)
#   error "Couldn't determine endianess"
#endif

#ifndef  SLANG_NO_INTTYPES
#include <inttypes.h>
#endif // ! SLANG_NO_INTTYPES

#ifndef  SLANG_NO_STDDEF
#include <stddef.h>
#endif // ! SLANG_NO_STDDEF

/* --------------- END From slang.h  ----------------- */

#include <math.h>
#include <assert.h>
#include <stdlib.h>

#ifndef SLANG_PRELUDE_PI
#   define SLANG_PRELUDE_PI           3.14159265358979323846
#endif

#if defined(_MSC_VER)
#   define SLANG_PRELUDE_SHARED_LIB_EXPORT __declspec(dllexport)
#else
#   define SLANG_PRELUDE_SHARED_LIB_EXPORT __attribute__((__visibility__("default")))
//#   define SLANG_PRELUDE_SHARED_LIB_EXPORT __attribute__ ((dllexport)) __attribute__((__visibility__("default")))
#endif    

#ifdef __cplusplus    
#   define SLANG_PRELUDE_EXTERN_C extern "C"
#else
#   define SLANG_PRELUDE_EXTERN_C 
#endif    

#define SLANG_PRELUDE_EXPORT SLANG_PRELUDE_EXTERN_C SLANG_PRELUDE_SHARED_LIB_EXPORT

#ifdef SLANG_PRELUDE_NAMESPACE
namespace SLANG_PRELUDE_NAMESPACE {
#endif


template <typename T, size_t SIZE>
struct FixedArray
{
    const T& operator[](size_t index) const { assert(index < SIZE); return m_data[index]; }
    T& operator[](size_t index) { assert(index < SIZE); return m_data[index]; }
    
    T m_data[SIZE];
};


// Hmm... I guess a constant buffer should be unwrapped to be just a struct passed in
/* template <typename T>
struct ConstantBuffer
{
}; */

template <typename T, int COUNT>
struct Vector;

template <typename T>
struct Vector<T, 1>
{
    T x;
};

template <typename T>
struct Vector<T, 2>
{
    T x, y;
};

template <typename T>
struct Vector<T, 3>
{
    T x, y, z;
};

template <typename T>
struct Vector<T, 4>
{
    T x, y, z, w;
};


typedef Vector<float, 2> float2;
typedef Vector<float, 3> float3;
typedef Vector<float, 4> float4;

typedef Vector<int32_t, 2> int2;
typedef Vector<int32_t, 3> int3;
typedef Vector<int32_t, 4> int4;

typedef Vector<uint32_t, 2> uint2;
typedef Vector<uint32_t, 3> uint3;
typedef Vector<uint32_t, 4> uint4;

template <typename T, int ROWS, int COLS>
struct Matrix
{
    Vector<T, COLS> rows[ROWS];
};

// ----------------------------- ResourceType -----------------------------------------

// https://docs.microsoft.com/en-us/windows/win32/direct3dhlsl/sm5-object-structuredbuffer-getdimensions
// Missing  Load(_In_  int  Location, _Out_ uint Status);

template <typename T>
struct RWStructuredBuffer
{
    T& operator[](size_t index) const { assert(index < count); return data[index]; }
    const T& Load(size_t index) const { assert(index < count); return data[index]; }  
    void GetDimensions(uint32_t& outNumStructs, uint32_t& outStride) { outNumStructs = uint32_t(count); outStride = uint32_t(sizeof(T)); }
  
    T* data;
    size_t count;
};

template <typename T>
struct StructuredBuffer
{
    const T& operator[](size_t index) const { assert(index < count); return data[index]; }
    const T& Load(size_t index) const { assert(index < count); return data[index]; }
    void GetDimensions(uint32_t& outNumStructs, uint32_t& outStride) { outNumStructs = uint32_t(count); outStride = uint32_t(sizeof(T)); }
    
    T* data;
    size_t count;
};

// Missing  Load(_In_  int  Location, _Out_ uint Status);
struct ByteAddressBuffer
{
    void GetDimensions(uint32_t& outDim) const { outDim = uint32_t(sizeInBytes); }
    uint32_t Load(size_t index) const 
    { 
        assert(index + 4 <= sizeInBytes && (index & 3) == 0); 
        return data[index >> 2]; 
    }
    uint2 Load2(size_t index) const 
    { 
        assert(index + 8 <= sizeInBytes && (index & 3) == 0); 
        const size_t dataIdx = index >> 2; 
        return uint2{data[dataIdx], data[dataIdx + 1]}; 
    }
    uint3 Load3(size_t index) const 
    { 
        assert(index + 12 <= sizeInBytes && (index & 3) == 0); 
        const size_t dataIdx = index >> 2; 
        return uint3{data[dataIdx], data[dataIdx + 1], data[dataIdx + 2]}; 
    }
    uint4 Load4(size_t index) const 
    { 
        assert(index + 16 <= sizeInBytes && (index & 3) == 0); 
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
    void GetDimensions(uint32_t& outDim) const { outDim = uint32_t(sizeInBytes); }
    
    uint32_t Load(size_t index) const 
    { 
        assert(index + 4 <= sizeInBytes && (index & 3) == 0); 
        return data[index >> 2]; 
    }
    uint2 Load2(size_t index) const 
    { 
        assert(index + 8 <= sizeInBytes && (index & 3) == 0); 
        const size_t dataIdx = index >> 2; 
        return uint2{data[dataIdx], data[dataIdx + 1]}; 
    }
    uint3 Load3(size_t index) const 
    { 
        assert(index + 12 <= sizeInBytes && (index & 3) == 0); 
        const size_t dataIdx = index >> 2; 
        return uint3{data[dataIdx], data[dataIdx + 1], data[dataIdx + 2]}; 
    }
    uint4 Load4(size_t index) const 
    { 
        assert(index + 16 <= sizeInBytes && (index & 3) == 0); 
        const size_t dataIdx = index >> 2; 
        return uint4{data[dataIdx], data[dataIdx + 1], data[dataIdx + 2], data[dataIdx + 3]}; 
    }
    
    void Store(size_t index, uint32_t v) const 
    { 
        assert(index + 4 <= sizeInBytes && (index & 3) == 0); 
        data[index >> 2] = v; 
    }
    void Store2(size_t index, uint2 v) const 
    { 
        assert(index + 8 <= sizeInBytes && (index & 3) == 0); 
        const size_t dataIdx = index >> 2; 
        data[dataIdx + 0] = v.x;
        data[dataIdx + 1] = v.y;
    }
    void Store3(size_t index, uint3 v) const 
    { 
        assert(index + 12 <= sizeInBytes && (index & 3) == 0); 
        const size_t dataIdx = index >> 2; 
        data[dataIdx + 0] = v.x;
        data[dataIdx + 1] = v.y;
        data[dataIdx + 2] = v.z;
    }
    void Store4(size_t index, uint4 v) const 
    { 
        assert(index + 16 <= sizeInBytes && (index & 3) == 0); 
        const size_t dataIdx = index >> 2; 
        data[dataIdx + 0] = v.x;
        data[dataIdx + 1] = v.y;
        data[dataIdx + 2] = v.z;
        data[dataIdx + 3] = v.w;
    }
    
    uint32_t* data;
    size_t sizeInBytes; //< Must be multiple of 4 
};

struct ISamplerState;
struct ISamplerComparisonState;

struct SamplerState
{
    ISamplerState* state;
};

struct SamplerComparisonState
{
    ISamplerComparisonState* state;
};

// Texture

struct ITexture2D
{
    virtual void Load(const int3& v, void* out) = 0;
    virtual void Sample(SamplerState samplerState, const float2& loc, void* out) = 0;
};

template <typename T>
struct Texture2D
{
    T Load(const int3& v) const { T out; texture->Load(v, &out); return out; }
    T Sample(SamplerState samplerState, const float2& v) const { T out; texture->Sample(samplerState, v, &out); return out; }
    
    ITexture2D* texture;              
};

/* Varing input for Compute */

struct ComputeVaryingInput
{
    uint3 groupID;
    uint3 groupThreadID;
};

// ----------------------------- F32 -----------------------------------------

union Union32 
{
    uint32_t u;
    int32_t i;
    float f;
};

// Helpers
SLANG_FORCE_INLINE float F32_calcSafeRadians(float radians)
{
	float a = radians * (1.0f /  float(SLANG_PRELUDE_PI));
	a = (a < 0.0f) ? (::ceilf(a) - a) : (a - ::floorf(a));
	return (a * float(SLANG_PRELUDE_PI));
}

// Unary 
SLANG_FORCE_INLINE float F32_ceil(float f) { return ::ceilf(f); }
SLANG_FORCE_INLINE float F32_floor(float f) { return ::floorf(f); }
SLANG_FORCE_INLINE float F32_sin(float f) { return ::sinf(F32_calcSafeRadians(f)); }
SLANG_FORCE_INLINE float F32_cos(float f) { return ::cosf(F32_calcSafeRadians(f)); }
SLANG_FORCE_INLINE float F32_tan(float f) { return ::tanf(f); }
SLANG_FORCE_INLINE float F32_asin(float f) { return ::asinf(f); }
SLANG_FORCE_INLINE float F32_acos(float f) { return ::acosf(f); }
SLANG_FORCE_INLINE float F32_atan(float f) { return ::atanf(f); }
SLANG_FORCE_INLINE float F32_log2(float f) { return ::log2f(f); }
SLANG_FORCE_INLINE float F32_exp2(float f) { return ::exp2f(f); }
SLANG_FORCE_INLINE float F32_exp(float f) { return ::expf(f); }
SLANG_FORCE_INLINE float F32_abs(float f) { return ::fabsf(f); }
SLANG_FORCE_INLINE float F32_trunc(float f) { return ::truncf(f); }
SLANG_FORCE_INLINE float F32_sqrt(float f) { return ::sqrtf(f); }
SLANG_FORCE_INLINE float F32_rsqrt(float f) { return 1.0f / F32_sqrt(f); }
SLANG_FORCE_INLINE float F32_rcp(float f) { return 1.0f / f; }
SLANG_FORCE_INLINE float F32_sign(float f) { return ( f == 0.0f) ? f : (( f < 0.0f) ? -1.0f : 1.0f); } 
SLANG_FORCE_INLINE float F32_saturate(float f) { return (f < 0.0f) ? 0.0f : (f > 1.0f) ? 1.0f : f; }
SLANG_FORCE_INLINE float F32_frac(float f) { return f - F32_floor(f); }
SLANG_FORCE_INLINE float F32_radians(float f) { return f * 0.01745329222f; }

// Binary
SLANG_FORCE_INLINE float F32_min(float a, float b) { return a < b ? a : b; }
SLANG_FORCE_INLINE float F32_max(float a, float b) { return a > b ? a : b; }
SLANG_FORCE_INLINE float F32_pow(float a, float b) { return ::powf(a, b); }
SLANG_FORCE_INLINE float F32_fmod(float a, float b) { return ::fmodf(a, b); }
SLANG_FORCE_INLINE float F32_step(float a, float b) { return float(a >= b); }
SLANG_FORCE_INLINE float F32_atan2(float a, float b) { return float(atan2(a, b)); }

// Ternary 
SLANG_FORCE_INLINE float F32_smoothstep(float min, float max, float x) { return x < min ? min : ((x > max) ? max : x / (max - min)); }
SLANG_FORCE_INLINE float F32_lerp(float x, float y, float s) { return x + s * (y - x); }
SLANG_FORCE_INLINE float F32_clamp(float x, float min, float max) { return ( x < min) ? min : ((x > max) ? max : x); }
SLANG_FORCE_INLINE void F32_sincos(float f, float& outSin, float& outCos) { outSin = F32_sin(f); outCos = F32_cos(f); }

SLANG_FORCE_INLINE uint32_t F32_asuint(float f) { Union32 u; u.f = f; return u.u; }
SLANG_FORCE_INLINE int32_t F32_asint(float f) { Union32 u; u.f = f; return u.i; }

// ----------------------------- F64 -----------------------------------------

SLANG_FORCE_INLINE double F64_calcSafeRadians(double radians)
{
    double a = radians * (1.0 / SLANG_PRELUDE_PI);
    a = (a < 0.0) ? (::ceil(a) - a) : (a - ::floor(a));
    return (a * SLANG_PRELUDE_PI);
}

// Unary 
SLANG_FORCE_INLINE double F64_ceil(double f) { return ::ceil(f); }
SLANG_FORCE_INLINE double F64_floor(double f) { return ::floor(f); }
SLANG_FORCE_INLINE double F64_sin(double f) { return ::sin(F64_calcSafeRadians(f)); }
SLANG_FORCE_INLINE double F64_cos(double f) { return ::cos(F64_calcSafeRadians(f)); }
SLANG_FORCE_INLINE double F64_tan(double f) { return ::tan(f); }
SLANG_FORCE_INLINE double F64_asin(double f) { return ::asin(f); }
SLANG_FORCE_INLINE double F64_acos(double f) { return ::acos(f); }
SLANG_FORCE_INLINE double F64_atan(double f) { return ::atan(f); }
SLANG_FORCE_INLINE double F64_log2(double f) { return ::log2(f); }
SLANG_FORCE_INLINE double F64_exp2(double f) { return ::exp2(f); }
SLANG_FORCE_INLINE double F64_exp(double f) { return ::exp(f); }
SLANG_FORCE_INLINE double F64_abs(double f) { return ::fabs(f); }
SLANG_FORCE_INLINE double F64_trunc(double f) { return ::trunc(f); }
SLANG_FORCE_INLINE double F64_sqrt(double f) { return ::sqrt(f); }
SLANG_FORCE_INLINE double F64_rsqrt(double f) { return 1.0 / F64_sqrt(f); }
SLANG_FORCE_INLINE double F64_rcp(double f) { return 1.0 / f; }
SLANG_FORCE_INLINE double F64_sign(double f) { return (f == 0.0) ? f : ((f < 0.0) ? -1.0 : 1.0); }
SLANG_FORCE_INLINE double F64_saturate(double f) { return (f < 0.0) ? 0.0 : (f > 1.0) ? 1.0 : f; }
SLANG_FORCE_INLINE double F64_frac(double f) { return f - F64_floor(f); }
SLANG_FORCE_INLINE double F64_radians(double f) { return f * 0.01745329222; }

// Binary
SLANG_FORCE_INLINE double F64_min(double a, double b) { return a < b ? a : b; }
SLANG_FORCE_INLINE double F64_max(double a, double b) { return a > b ? a : b; }
SLANG_FORCE_INLINE double F64_pow(double a, double b) { return ::pow(a, b); }
SLANG_FORCE_INLINE double F64_fmod(double a, double b) { return ::fmod(a, b); }
SLANG_FORCE_INLINE double F64_step(double a, double b) { return double(a >= b); }
SLANG_FORCE_INLINE double F64_atan2(double a, double b) { return atan2(a, b); }

// Ternary 
SLANG_FORCE_INLINE double F64_smoothstep(double min, double max, double x) { return x < min ? min : ((x > max) ? max : x / (max - min)); }
SLANG_FORCE_INLINE double F64_lerp(double x, double y, double s) { return x + s * (y - x); }
SLANG_FORCE_INLINE double F64_clamp(double x, double min, double max) { return (x < min) ? min : ((x > max) ? max : x); }
SLANG_FORCE_INLINE void F64_sincos(double f, double& outSin, double& outCos) { outSin = F64_sin(f); outCos = F64_cos(f); }

// TODO!
//uint32_t F64_asuint(float f);
//int32_t F64_asint(float f);

// ----------------------------- I32 -----------------------------------------

SLANG_FORCE_INLINE int32_t I32_abs(int32_t f) { return (f < 0) ? -f : f; }

SLANG_FORCE_INLINE int32_t I32_min(int32_t a, int32_t b) { return a < b ? a : b; }
SLANG_FORCE_INLINE int32_t I32_max(int32_t a, int32_t b) { return a > b ? a : b; }

SLANG_FORCE_INLINE int32_t I32_clamp(int32_t x, int32_t min, int32_t max) { return ( x < min) ? min : ((x > max) ? max : x); }

SLANG_FORCE_INLINE float I32_asfloat(int32_t x) { Union32 u; u.i = x; return u.f; }

// ----------------------------- U32 -----------------------------------------

SLANG_FORCE_INLINE uint32_t U32_abs(uint32_t f) { return f; }

SLANG_FORCE_INLINE uint32_t U32_min(uint32_t a, uint32_t b) { return a < b ? a : b; }
SLANG_FORCE_INLINE uint32_t U32_max(uint32_t a, uint32_t b) { return a > b ? a : b; }

SLANG_FORCE_INLINE uint32_t U32_clamp(uint32_t x, uint32_t min, uint32_t max) { return ( x < min) ? min : ((x > max) ? max : x); }

SLANG_FORCE_INLINE float U32_asfloat(uint32_t x) { Union32 u; u.u = x; return u.f; }

#ifdef SLANG_PRELUDE_NAMESPACE
} 
#endif

#endif
