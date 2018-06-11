#ifndef SLANG_DEFINES_H
#define SLANG_DEFINES_H

/*
Some of our `#define`s are needed in the public API header as well, so
we will go ahead and include that here so that we can share the definitions.
*/
#include "../../slang.h"

/*
The following preprocessor identifiers specify compiler, OS, and architecture.
All definitions have a value of 1 or 0, use '#if' instead of '#ifdef'.
*/

#ifndef SLANG_PROCESSOR
#	define SLANG_PROCESSOR

/* Architecture defines, see http://sourceforge.net/p/predef/wiki/Architectures/ */
#	if defined(__x86_64__) || defined(_M_X64) // ps4 compiler defines _M_X64 without value
#		define SLANG_X64 1
#	elif defined(__i386__) || defined(_M_IX86)
#		define SLANG_X86 1
#	elif defined(__arm64__) || defined(__aarch64__)
#		define SLANG_A64 1
#	elif defined(__arm__) || defined(_M_ARM)
#		define SLANG_ARM 1
#	elif defined(__SPU__)
#		define SLANG_SPU 1
#	elif defined(__ppc__) || defined(_M_PPC) || defined(__CELLOS_LV2__)
#		define SLANG_PPC 1
#	else
#		error "Unknown architecture"
#	endif

/**
SIMD defines
*/
#	if defined(__i386__) || defined(_M_IX86) || defined(__x86_64__) || defined(_M_X64)
#		define SLANG_SSE2 1
#	endif
#	if defined(_M_ARM) || defined(__ARM_NEON__)
#		define SLANG_NEON 1
#	endif
#	if defined(_M_PPC) || defined(__CELLOS_LV2__)
#		define SLANG_VMX 1
#	endif

// Zero unset
#	ifndef SLANG_X64
#		define SLANG_X64 0
#	endif
#	ifndef SLANG_X86
#		define SLANG_X86 0
#	endif
#	ifndef SLANG_A64
#		define SLANG_A64 0
#	endif
#	ifndef SLANG_ARM
#		define SLANG_ARM 0
#	endif
#	ifndef SLANG_SPU
#		define SLANG_SPU 0
#	endif
#	ifndef SLANG_PPC
#		define SLANG_PPC 0
#	endif
#	ifndef SLANG_SSE2
#		define SLANG_SSE2 0
#	endif
#	ifndef SLANG_NEON
#		define SLANG_NEON 0
#	endif
#	ifndef SLANG_VMX
#		define SLANG_VMX 0
#	endif

#endif // SLANG_PROCESSOR

/*
define anything not defined through the command line to 0
*/
#ifndef SLANG_DEBUG
#	define SLANG_DEBUG 0
#endif
#ifndef SLANG_CHECKED
#	define SLANG_CHECKED 0
#endif
#ifndef SLANG_PROFILE
#	define SLANG_PROFILE 0
#endif

/**
family shortcuts
*/
// architecture
#define SLANG_INTEL_FAMILY (SLANG_X64 || SLANG_X86)					// Intel x86 family
#define SLANG_ARM_FAMILY (SLANG_ARM || SLANG_A64)
#define SLANG_PPC_FAMILY (SLANG_PPC)

#define SLANG_P64_FAMILY (SLANG_X64 || SLANG_A64) // shortcut for 64-bit architectures

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

/**
General defines
*/

// GCC Specific
#if SLANG_GCC_FAMILY
#	define SLANG_NO_INLINE __attribute__((noinline))

// This doesn't work on clang - because the typedef is seen as multiply defined, use the line numbered version defined later
#	if !defined(__clang__) && (defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 7)) || defined(__ORBIS__))
#		define SLANG_COMPILE_TIME_ASSERT(exp) typedef char SlangCompileTimeAssert_Dummy[(exp) ? 1 : -1] __attribute__((unused))
#	endif

#	if !SLANG_SNC && !SLANG_GHS
#		define SLANG_OFFSET_OF(X, Y) __builtin_offsetof(X, Y)
#	endif

//#	if !SLANG_LINUX // Workaround; Fedora Core 3 do not agree with force inline
#	define SLANG_FORCE_INLINE inline __attribute__((always_inline))
//#	endif

#   define SLANG_BREAKPOINT(id) __builtin_trap();

#	if (__cplusplus >= 201103L) && ((__GNUC__ > 4) || (__GNUC__ ==4 && __GNUC_MINOR__ >= 6))
#		define SLANG_NULL	nullptr
#	else
#		define SLANG_NULL	__null
#	endif

#	define SLANG_ALIGN_OF(T)	__alignof__(T)

#	define SLANG_FUNCTION_SIG	__PRETTY_FUNCTION__

#endif // SLANG_GCC_FAMILY

// Microsoft VC specific
#if SLANG_MICROSOFT_FAMILY

#	pragma inline_depth(255)

#	pragma warning(disable : 4324 )	// C4324: structure was padded due to alignment specifier
#	pragma warning(disable : 4514 )	// 'function' : unreferenced inline function has been removed
#	pragma warning(disable : 4710 )	// 'function' : function not inlined
#	pragma warning(disable : 4711 )	// function 'function' selected for inline expansion
#	pragma warning(disable : 4127 )	// C4127: conditional expression is constant

#	define SLANG_NO_ALIAS __declspec(noalias)
#	define SLANG_NO_INLINE __declspec(noinline)
#	define SLANG_FORCE_INLINE __forceinline
#	define SLANG_PUSH_PACK_DEFAULT __pragma(pack(push, 8))
#	define SLANG_POP_PACK __pragma(pack(pop))

#	define SLANG_BREAKPOINT(id) __debugbreak();

#	ifdef __cplusplus
#		define SLANG_NULL	nullptr
#	endif

#	define SLANG_ALIGN_OF(T) __alignof(T)

#	define SLANG_FUNCTION_SIG	__FUNCSIG__

#	define SLANG_INT64(x) (x##i64)
#	define SLANG_UINT64(x) (x##ui64)

#	define SLANG_CALL_CONV __cdecl

#endif // SLANG_MICROSOFT_FAMILY

// Set defaults if not set

// Used for doing constant literals
#ifndef SLANG_INT64
#	define SLANG_INT64(x) (x##ll)
#endif
#ifndef SLANG_UINT64
#	define SLANG_UINT64(x) (x##ull)
#endif

#ifndef SLANG_NULL
#	if defined(NULL)
#		define SLANG_NULL NULL
#	else
#		define SLANG_NULL 0
#	endif
#endif

#ifndef SLANG_FORCE_INLINE
#	define SLANG_FORCE_INLINE inline
#endif
#ifndef SLANG_NO_INLINE
#	define SLANG_NO_INLINE
#endif
#ifndef SLANG_NO_ALIAS
#	define SLANG_NO_ALIAS
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

#ifndef SLANG_FUNCTION_NAME
#	define SLANG_FUNCTION_NAME	__FUNCTION__
#endif
#ifndef SLANG_FUNCTION_SIG
#	define SLANG_FUNCTION_SIG SLANG_FUNCTION_NAME
#endif

#ifndef SLANG_CALL_CONV
#	define SLANG_CALL_CONV
#endif

//! casting the null ptr takes a special-case code path, which we don't want
#define SLANG_OFFSETOF_BASE 0x100
#define SLANG_OFFSET_OF_RT(Class, Member)                                                                                 \
	(reinterpret_cast<size_t>(&reinterpret_cast<Class*>(SLANG_OFFSETOF_BASE)->Member) - size_t(SLANG_OFFSETOF_BASE))

#ifdef __CUDACC__
#	define SLANG_CUDA_CALLABLE __host__ __device__
#else
#	define SLANG_CUDA_CALLABLE
#endif

#if SLANG_GCC_FAMILY && !SLANG_GHS
#	define SLANG_WEAK_SYMBOL __attribute__((weak)) // this is to support SIMD constant merging in template specialization
#else
#	define SLANG_WEAK_SYMBOL
#endif

// C++ Symbols
#ifdef __cplusplus

// C++ specific macros

// Gcc
#	if SLANG_GCC_FAMILY
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

// Clang specific
#	if SLANG_CLANG
#	endif // SLANG_CLANG

// Set non set

#ifndef SLANG_NO_THROW
#	define SLANG_NO_THROW
#endif
#ifndef SLANG_OVERRIDE
#	define SLANG_OVERRIDE
#endif
#ifndef SLANG_HAS_ENUM_CLASS
#	define SLANG_HAS_ENUM_CLASS 0
#endif
#ifndef SLANG_HAS_MOVE_SEMANTICS
#	define SLANG_HAS_MOVE_SEMANTICS 0
#endif

#include <new>		// For placement new

#endif // __cplusplus


#endif // SLANG_DEFINES_H
