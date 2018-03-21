#ifndef SLANG_DEFINES_H
#define SLANG_DEFINES_H

/*
The following preprocessor identifiers specify compiler, OS, and architecture.
All definitions have a value of 1 or 0, use '#if' instead of '#ifdef'.
*/

#ifndef SLANG_COMPILER
#	define SLANG_COMPILER

/*
Compiler defines, see http://sourceforge.net/p/predef/wiki/Compilers/
NOTE that SLANG_VC holds the compiler version - not just 1 or 0
*/
#	if defined(_MSC_VER)
#		if _MSC_VER >= 1900
#			define SLANG_VC 14
#		elif _MSC_VER >= 1800
#			define SLANG_VC 12
#		elif _MSC_VER >= 1700
#			define SLANG_VC 11
#		elif _MSC_VER >= 1600
#			define SLANG_VC 10
#		elif _MSC_VER >= 1500
#			define SLANG_VC 9
#		else
#			error "Unknown VC version"
#		endif
#	elif defined(__clang__)
#		define SLANG_CLANG 1
#	elif defined(__SNC__)
#		define SLANG_SNC 1
#	elif defined(__ghs__)
#		define SLANG_GHS 1
#	elif defined(__GNUC__) // note: __clang__, __SNC__, or __ghs__ imply __GNUC__
#		define SLANG_GCC 1
#	else
#		error "Unknown compiler"
#	endif

// Zero unset
#	ifndef SLANG_VC
#		define SLANG_VC 0
#	endif
#	ifndef SLANG_CLANG
#		define SLANG_CLANG 0
#	endif
#	ifndef SLANG_SNC
#		define SLANG_SNC 0
#	endif
#	ifndef SLANG_GHS
#		define SLANG_GHS 0
#	endif
#	ifndef SLANG_GCC
#		define SLANG_GCC 0
#	endif

#endif // SLANG_COMPILER

#ifndef SLANG_PLATFORM
#	define SLANG_PLATFORM

/**
Operating system defines, see http://sourceforge.net/p/predef/wiki/OperatingSystems/
*/
#	if defined(WINAPI_FAMILY) && WINAPI_FAMILY == WINAPI_PARTITION_APP
#		define SLANG_WINRT 1 // Windows Runtime, either on Windows RT or Windows 8
#	elif defined(XBOXONE)
#		define SLANG_XBOXONE 1
#	elif defined(_WIN64) // note: XBOXONE implies _WIN64
#		define SLANG_WIN64 1
#	elif defined(_M_PPC)
#		define SLANG_X360 1
#	elif defined(_WIN32) // note: _M_PPC implies _WIN32
#		define SLANG_WIN32 1
#	elif defined(__ANDROID__)
#		define SLANG_ANDROID 1
#	elif defined(__linux__) || defined(__CYGWIN__) // note: __ANDROID__ implies __linux__
#		define SLANG_LINUX 1
#	elif defined(__APPLE__) && (defined(__arm__) || defined(__arm64__))
#		define SLANG_IOS 1
#	elif defined(__APPLE__)
#		define SLANG_OSX 1
#	elif defined(__CELLOS_LV2__)
#		define SLANG_PS3 1
#	elif defined(__ORBIS__)
#		define SLANG_PS4 1
#	elif defined(__SNC__) && defined(__arm__)
#		define SLANG_PSP2 1
#	elif defined(__ghs__)
#		define SLANG_WIIU 1
#	else
#		error "Unknown operating system"
#	endif

// zero unset
#	ifndef SLANG_WINRT
#		define SLANG_WINRT 0
#	endif
#	ifndef SLANG_XBOXONE
#		define SLANG_XBOXONE 0
#	endif
#	ifndef SLANG_WIN64
#		define SLANG_WIN64 0
#	endif
#	ifndef SLANG_X360
#		define SLANG_X360 0
#	endif
#	ifndef SLANG_WIN32
#		define SLANG_WIN32 0
#	endif
#	ifndef SLANG_ANDROID
#		define SLANG_ANDROID 0
#	endif
#	ifndef SLANG_LINUX
#		define SLANG_LINUX 0
#	endif
#	ifndef SLANG_IOS
#		define SLANG_IOS 0
#	endif
#	ifndef SLANG_OSX
#		define SLANG_OSX 0
#	endif
#	ifndef SLANG_PS3
#		define SLANG_PS3 0
#	endif
#	ifndef SLANG_PS4
#		define SLANG_PS4 0
#	endif
#	ifndef SLANG_PSP2
#		define SLANG_PSP2 0
#	endif
#	ifndef SLANG_WIIU
#		define SLANG_WIIU 0
#	endif

#endif // SLANG_PLATFORM

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
// compiler
#define SLANG_GCC_FAMILY (SLANG_CLANG || SLANG_SNC || SLANG_GHS || SLANG_GCC)

// os
#define SLANG_WINDOWS_FAMILY (SLANG_WINRT || SLANG_WIN32 || SLANG_WIN64)
#define SLANG_MICROSOFT_FAMILY (SLANG_XBOXONE || SLANG_X360 || SLANG_WINDOWS_FAMILY)
#define SLANG_LINUX_FAMILY (SLANG_LINUX || SLANG_ANDROID)
#define SLANG_APPLE_FAMILY (SLANG_IOS || SLANG_OSX)                  // equivalent to #if __APPLE__
#define SLANG_UNIX_FAMILY (SLANG_LINUX_FAMILY || SLANG_APPLE_FAMILY) // shortcut for unix/posix platforms
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

#	define SLANG_STDCALL __stdcall
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

#ifndef SLANG_STDCALL
#	define SLANG_STDCALL 
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

// Macro for declaring if a method is no throw. Should be set before the return parameter. 
#if SLANG_WINDOWS_FAMILY && !defined(SLANG_DISABLE_EXCEPTIONS)
#	define SLANG_NO_THROW __declspec(nothrow) 
#endif

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

#define SLANG_MCALL SLANG_STDCALL

#include <new>		// For placement new

#endif // __cplusplus


#endif // SLANG_DEFINES_H
