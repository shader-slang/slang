#ifndef SLANG_CPP_HOST_PRELUDE_H
#define SLANG_CPP_HOST_PRELUDE_H

#include <cstdio>
#include <cmath>
#include <cstring>

#ifndef SLANG_CORE_STRING_H
#define SLANG_CORE_STRING_H

#include <string.h>
#include <cstdlib>
#include <stdio.h>

#ifndef SLANG_CORE_SMART_POINTER_H
#define SLANG_CORE_SMART_POINTER_H

#ifndef SLANG_CORE_COMMON_H
#define SLANG_CORE_COMMON_H

#ifndef SLANG_H
#define SLANG_H

/** \file slang.h

The Slang API provides services to compile, reflect, and specialize code
written in the Slang shading language.
*/

/*
The following section attempts to detect the compiler and version in use.

If an application defines `SLANG_COMPILER` before including this header,
they take responsibility for setting any compiler-dependent macros
used later in the file.

Most applications should not need to touch this section.
*/
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
#   if 0 && __GNUC__ >= 4
// Didn't work on latest gcc on linux.. so disable for now
// https://gcc.gnu.org/wiki/Visibility
#       define SLANG_DLL_EXPORT __attribute__ ((dllexport))
#   else
#       define SLANG_DLL_EXPORT __attribute__((__visibility__("default")))
#   endif
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

// If explicilty disabled and not set, set to not available
#if !defined(SLANG_HAS_EXCEPTIONS) && defined(SLANG_DISABLE_EXCEPTIONS)
#   define SLANG_HAS_EXCEPTIONS 0
#endif

// If not set, the default is exceptions are available
#ifndef SLANG_HAS_EXCEPTIONS
#   define SLANG_HAS_EXCEPTIONS 1
#endif

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

// TODO(JS): Not used in previous code. Left here as may be useful on some other version. 
// #define SLANG_RETURN_NEVER __attribute__((__noreturn__))

#       define SLANG_RETURN_NEVER [[noreturn]]

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

#   define SLANG_RETURN_NEVER __declspec(noreturn)

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

#ifndef SLANG_RETURN_NEVER
#   define SLANG_RETURN_NEVER /* empty */
#endif // SLANG_RETURN_NEVER

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
#elif defined(_M_ARM64) || defined(__aarch64__)
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

#ifdef __cplusplus
extern "C"
{
#endif
    /*!
    @mainpage Introduction

    API Reference: slang.h

    @file slang.h
    */

    typedef uint32_t    SlangUInt32;
    typedef int32_t     SlangInt32;

    // Use SLANG_PTR_ macros to determine SlangInt/SlangUInt types.
    // This is used over say using size_t/ptrdiff_t/intptr_t/uintptr_t, because on some targets, these types are distinct from
    // their uint_t/int_t equivalents and so produce ambiguity with function overloading.
    //
    // SlangSizeT is helpful as on some compilers size_t is distinct from a regular integer type and so overloading doesn't work.
    // Casting to SlangSizeT works around this.
#if SLANG_PTR_IS_64
    typedef int64_t    SlangInt;
    typedef uint64_t   SlangUInt;

    typedef uint64_t   SlangSizeT;
#else
    typedef int32_t    SlangInt;
    typedef uint32_t   SlangUInt;

    typedef uint32_t   SlangSizeT;
#endif

    typedef bool SlangBool;

    
    /*!
    @brief Severity of a diagnostic generated by the compiler.
    Values come from the enum below, with higher values representing more severe
    conditions, and all values >= SLANG_SEVERITY_ERROR indicating compilation
    failure.
    */
    typedef int SlangSeverity;
    enum
    {
        SLANG_SEVERITY_NOTE = 0,    /**< An informative message. */
        SLANG_SEVERITY_WARNING,     /**< A warning, which indicates a possible proble. */
        SLANG_SEVERITY_ERROR,       /**< An error, indicating that compilation failed. */
        SLANG_SEVERITY_FATAL,       /**< An unrecoverable error, which forced compilation to abort. */
        SLANG_SEVERITY_INTERNAL,    /**< An internal error, indicating a logic error in the compiler. */
    };

    typedef int SlangBindableResourceType;
    enum
    {
        SLANG_NON_BINDABLE = 0,
        SLANG_TEXTURE,
        SLANG_SAMPLER,
        SLANG_UNIFORM_BUFFER,
        SLANG_STORAGE_BUFFER,
    };

    typedef int SlangCompileTarget;
    enum
    {
        SLANG_TARGET_UNKNOWN,
        SLANG_TARGET_NONE,
        SLANG_GLSL,
        SLANG_GLSL_VULKAN,          //< deprecated: just use `SLANG_GLSL`
        SLANG_GLSL_VULKAN_ONE_DESC, //< deprecated
        SLANG_HLSL,
        SLANG_SPIRV,
        SLANG_SPIRV_ASM,
        SLANG_DXBC,
        SLANG_DXBC_ASM,
        SLANG_DXIL,
        SLANG_DXIL_ASM,
        SLANG_C_SOURCE,             ///< The C language
        SLANG_CPP_SOURCE,           ///< The C++ language
        SLANG_EXECUTABLE,           ///< Executable (for hosting CPU/OS)
        SLANG_SHARED_LIBRARY,       ///< A shared library/Dll (for hosting CPU/OS)
        SLANG_HOST_CALLABLE,        ///< A CPU target that makes the compiled code available to be run immediately
        SLANG_CUDA_SOURCE,          ///< Cuda source
        SLANG_PTX,                  ///< PTX
        SLANG_OBJECT_CODE,          ///< Object code that can be used for later linking
        SLANG_TARGET_COUNT_OF,
    };

    /* A "container format" describes the way that the outputs
    for multiple files, entry points, targets, etc. should be
    combined into a single artifact for output. */
    typedef int SlangContainerFormat;
    enum
    {
        /* Don't generate a container. */
        SLANG_CONTAINER_FORMAT_NONE,

        /* Generate a container in the `.slang-module` format,
        which includes reflection information, compiled kernels, etc. */
        SLANG_CONTAINER_FORMAT_SLANG_MODULE,
    };

    typedef int SlangPassThroughIntegral;
    enum SlangPassThrough : SlangPassThroughIntegral
    {
        SLANG_PASS_THROUGH_NONE,
        SLANG_PASS_THROUGH_FXC,
        SLANG_PASS_THROUGH_DXC,
        SLANG_PASS_THROUGH_GLSLANG,
        SLANG_PASS_THROUGH_CLANG,                   ///< Clang C/C++ compiler 
        SLANG_PASS_THROUGH_VISUAL_STUDIO,           ///< Visual studio C/C++ compiler
        SLANG_PASS_THROUGH_GCC,                     ///< GCC C/C++ compiler
        SLANG_PASS_THROUGH_GENERIC_C_CPP,           ///< Generic C or C++ compiler, which is decided by the source type
        SLANG_PASS_THROUGH_NVRTC,                   ///< NVRTC Cuda compiler
        SLANG_PASS_THROUGH_LLVM,                    ///< LLVM 'compiler' - includes LLVM and Clang
        SLANG_PASS_THROUGH_COUNT_OF,
    };

    /* Defines an archive type used to holds a 'file system' type structure. */
    typedef int SlangArchiveTypeIntegral;
    enum SlangArchiveType : SlangArchiveTypeIntegral
    {
        SLANG_ARCHIVE_TYPE_UNDEFINED,
        SLANG_ARCHIVE_TYPE_ZIP,
        SLANG_ARCHIVE_TYPE_RIFF,                ///< Riff container with no compression
        SLANG_ARCHIVE_TYPE_RIFF_DEFLATE,
        SLANG_ARCHIVE_TYPE_RIFF_LZ4,
        SLANG_ARCHIVE_TYPE_COUNT_OF,
    };

    /*!
    Flags to control compilation behavior.
    */
    typedef unsigned int SlangCompileFlags;
    enum
    {
        /* Do as little mangling of names as possible, to try to preserve original names */
        SLANG_COMPILE_FLAG_NO_MANGLING          = 1 << 3,

        /* Skip code generation step, just check the code and generate layout */
        SLANG_COMPILE_FLAG_NO_CODEGEN           = 1 << 4,

        /* Obfuscate shader names on release products */
        SLANG_COMPILE_FLAG_OBFUSCATE = 1 << 5,

        /* Deprecated flags: kept around to allow existing applications to
        compile. Note that the relevant features will still be left in
        their default state. */
        SLANG_COMPILE_FLAG_NO_CHECKING          = 0,
        SLANG_COMPILE_FLAG_SPLIT_MIXED_TYPES    = 0,
    };

    /*!
    @brief Flags to control code generation behavior of a compilation target */
    typedef unsigned int SlangTargetFlags;
    enum
    {
        /* When compiling for a D3D Shader Model 5.1 or higher target, allocate
           distinct register spaces for parameter blocks.

           @deprecated This behavior is now enabled unconditionally.
        */
        SLANG_TARGET_FLAG_PARAMETER_BLOCKS_USE_REGISTER_SPACES = 1 << 4,

        /* When set, will generate target code that contains all entrypoints defined
           in the input source or specified via the `spAddEntryPoint` function in a
           single output module (library/source file).
        */
        SLANG_TARGET_FLAG_GENERATE_WHOLE_PROGRAM = 1 << 8,

        /* When set, will dump out the IR between intermediate compilation steps.*/
        SLANG_TARGET_FLAG_DUMP_IR = 1 << 9,

        /* When set, will generate SPIRV directly instead of going through glslang. */
        SLANG_TARGET_FLAG_GENERATE_SPIRV_DIRECTLY = 1 << 10,
    };

    /*!
    @brief Options to control floating-point precision guarantees for a target.
    */
    typedef unsigned int SlangFloatingPointMode;
    enum
    {
        SLANG_FLOATING_POINT_MODE_DEFAULT = 0,
        SLANG_FLOATING_POINT_MODE_FAST,
        SLANG_FLOATING_POINT_MODE_PRECISE,
    };

    /*!
    @brief Options to control emission of `#line` directives
    */
    typedef unsigned int SlangLineDirectiveMode;
    enum
    {
        SLANG_LINE_DIRECTIVE_MODE_DEFAULT = 0,  /**< Default behavior: pick behavior base on target. */
        SLANG_LINE_DIRECTIVE_MODE_NONE,         /**< Don't emit line directives at all. */
        SLANG_LINE_DIRECTIVE_MODE_STANDARD,     /**< Emit standard C-style `#line` directives. */
        SLANG_LINE_DIRECTIVE_MODE_GLSL,         /**< Emit GLSL-style directives with file *number* instead of name */
    };

    typedef int SlangSourceLanguageIntegral;
    enum SlangSourceLanguage : SlangSourceLanguageIntegral
    {
        SLANG_SOURCE_LANGUAGE_UNKNOWN,
        SLANG_SOURCE_LANGUAGE_SLANG,
        SLANG_SOURCE_LANGUAGE_HLSL,
        SLANG_SOURCE_LANGUAGE_GLSL,
        SLANG_SOURCE_LANGUAGE_C,
        SLANG_SOURCE_LANGUAGE_CPP,
        SLANG_SOURCE_LANGUAGE_CUDA,
        SLANG_SOURCE_LANGUAGE_COUNT_OF,
    };

    typedef unsigned int SlangProfileID;
    enum
    {
        SLANG_PROFILE_UNKNOWN,
    };

    typedef SlangInt32 SlangCapabilityID;
    enum
    {
        SLANG_CAPABILITY_UNKNOWN = 0,
    };

    typedef unsigned int SlangMatrixLayoutMode;
    enum
    {
        SLANG_MATRIX_LAYOUT_MODE_UNKNOWN = 0,
        SLANG_MATRIX_LAYOUT_ROW_MAJOR,
        SLANG_MATRIX_LAYOUT_COLUMN_MAJOR,
    };

    typedef SlangUInt32 SlangStage;
    enum
    {
        SLANG_STAGE_NONE,
        SLANG_STAGE_VERTEX,
        SLANG_STAGE_HULL,
        SLANG_STAGE_DOMAIN,
        SLANG_STAGE_GEOMETRY,
        SLANG_STAGE_FRAGMENT,
        SLANG_STAGE_COMPUTE,
        SLANG_STAGE_RAY_GENERATION,
        SLANG_STAGE_INTERSECTION,
        SLANG_STAGE_ANY_HIT,
        SLANG_STAGE_CLOSEST_HIT,
        SLANG_STAGE_MISS,
        SLANG_STAGE_CALLABLE,
        SLANG_STAGE_MESH,
        SLANG_STAGE_AMPLIFICATION,

        // alias:
        SLANG_STAGE_PIXEL = SLANG_STAGE_FRAGMENT,
    };

    typedef SlangUInt32 SlangDebugInfoLevel;
    enum
    {
        SLANG_DEBUG_INFO_LEVEL_NONE = 0,    /**< Don't emit debug information at all. */
        SLANG_DEBUG_INFO_LEVEL_MINIMAL,     /**< Emit as little debug information as possible, while still supporting stack trackes. */
        SLANG_DEBUG_INFO_LEVEL_STANDARD,    /**< Emit whatever is the standard level of debug information for each target. */
        SLANG_DEBUG_INFO_LEVEL_MAXIMAL,     /**< Emit as much debug infromation as possible for each target. */
        
    };

    typedef SlangUInt32 SlangOptimizationLevel;
    enum
    {
        SLANG_OPTIMIZATION_LEVEL_NONE = 0,  /**< Don't optimize at all. */
        SLANG_OPTIMIZATION_LEVEL_DEFAULT,   /**< Default optimization level: balance code quality and compilation time. */
        SLANG_OPTIMIZATION_LEVEL_HIGH,      /**< Optimize aggressively. */
        SLANG_OPTIMIZATION_LEVEL_MAXIMAL,   /**< Include optimizations that may take a very long time, or may involve severe space-vs-speed tradeoffs */
    };

    /** A result code for a Slang API operation.

    This type is generally compatible with the Windows API `HRESULT` type. In particular, negative values indicate
    failure results, while zero or positive results indicate success.

    In general, Slang APIs always return a zero result on success, unless documented otherwise. Strictly speaking
    a negative value indicates an error, a positive (or 0) value indicates success. This can be tested for with the macros
    SLANG_SUCCEEDED(x) or SLANG_FAILED(x).

    It can represent if the call was successful or not. It can also specify in an extensible manner what facility
    produced the result (as the integral 'facility') as well as what caused it (as an integral 'code').
    Under the covers SlangResult is represented as a int32_t.

    SlangResult is designed to be compatible with COM HRESULT.

    It's layout in bits is as follows

    Severity | Facility | Code
    ---------|----------|-----
    31       |    30-16 | 15-0

    Severity - 1 fail, 0 is success - as SlangResult is signed 32 bits, means negative number indicates failure.
    Facility is where the error originated from. Code is the code specific to the facility.

    Result codes have the following styles,
    1) SLANG_name
    2) SLANG_s_f_name
    3) SLANG_s_name

    where s is S for success, E for error
    f is the short version of the facility name

    Style 1 is reserved for SLANG_OK and SLANG_FAIL as they are so commonly used.

    It is acceptable to expand 'f' to a longer name to differentiate a name or drop if unique without it.
    ie for a facility 'DRIVER' it might make sense to have an error of the form SLANG_E_DRIVER_OUT_OF_MEMORY
    */

    typedef int32_t SlangResult;

    //! Use to test if a result was failure. Never use result != SLANG_OK to test for failure, as there may be successful codes != SLANG_OK.
#define SLANG_FAILED(status) ((status) < 0)
    //! Use to test if a result succeeded. Never use result == SLANG_OK to test for success, as will detect other successful codes as a failure.
#define SLANG_SUCCEEDED(status) ((status) >= 0)

    //! Get the facility the result is associated with
#define SLANG_GET_RESULT_FACILITY(r)    ((int32_t)(((r) >> 16) & 0x7fff))
    //! Get the result code for the facility
#define SLANG_GET_RESULT_CODE(r)        ((int32_t)((r) & 0xffff))

#define SLANG_MAKE_ERROR(fac, code)        ((((int32_t)(fac)) << 16) | ((int32_t)(code)) | int32_t(0x80000000))
#define SLANG_MAKE_SUCCESS(fac, code)    ((((int32_t)(fac)) << 16) | ((int32_t)(code)))

    /*************************** Facilities ************************************/

    //! Facilities compatible with windows COM - only use if known code is compatible
#define SLANG_FACILITY_WIN_GENERAL      0
#define SLANG_FACILITY_WIN_INTERFACE    4
#define SLANG_FACILITY_WIN_API          7

    //! Base facility -> so as to not clash with HRESULT values (values in 0x200 range do not appear used)
#define SLANG_FACILITY_BASE         0x200

    /*! Facilities numbers must be unique across a project to make the resulting result a unique number.
    It can be useful to have a consistent short name for a facility, as used in the name prefix */
#define SLANG_FACILITY_CORE             SLANG_FACILITY_BASE
    /* Facility for codes, that are not uniquely defined/protected. Can be used to pass back a specific error without requiring system wide facility uniqueness. Codes
    should never be part of a public API. */
#define SLANG_FACILITY_INTERNAL         SLANG_FACILITY_BASE + 1

    /// Base for external facilities. Facilities should be unique across modules.
#define SLANG_FACILITY_EXTERNAL_BASE 0x210

    /* ************************ Win COM compatible Results ******************************/
    // https://msdn.microsoft.com/en-us/library/windows/desktop/aa378137(v=vs.85).aspx

    //! SLANG_OK indicates success, and is equivalent to SLANG_MAKE_SUCCESS(SLANG_FACILITY_WIN_GENERAL, 0)
#define SLANG_OK                          0
    //! SLANG_FAIL is the generic failure code - meaning a serious error occurred and the call couldn't complete
#define SLANG_FAIL                          SLANG_MAKE_ERROR(SLANG_FACILITY_WIN_GENERAL, 0x4005)

#define SLANG_MAKE_WIN_GENERAL_ERROR(code)  SLANG_MAKE_ERROR(SLANG_FACILITY_WIN_GENERAL, code)

    //! Functionality is not implemented
#define SLANG_E_NOT_IMPLEMENTED             SLANG_MAKE_WIN_GENERAL_ERROR(0x4001)
    //! Interface not be found
#define SLANG_E_NO_INTERFACE                SLANG_MAKE_WIN_GENERAL_ERROR(0x4002)
    //! Operation was aborted (did not correctly complete)
#define SLANG_E_ABORT                       SLANG_MAKE_WIN_GENERAL_ERROR(0x4004) 

    //! Indicates that a handle passed in as parameter to a method is invalid.
#define SLANG_E_INVALID_HANDLE              SLANG_MAKE_ERROR(SLANG_FACILITY_WIN_API, 6)
    //! Indicates that an argument passed in as parameter to a method is invalid.
#define SLANG_E_INVALID_ARG                 SLANG_MAKE_ERROR(SLANG_FACILITY_WIN_API, 0x57)
    //! Operation could not complete - ran out of memory
#define SLANG_E_OUT_OF_MEMORY               SLANG_MAKE_ERROR(SLANG_FACILITY_WIN_API, 0xe)

    /* *************************** other Results **************************************/

#define SLANG_MAKE_CORE_ERROR(code)         SLANG_MAKE_ERROR(SLANG_FACILITY_CORE, code)

    // Supplied buffer is too small to be able to complete
#define SLANG_E_BUFFER_TOO_SMALL            SLANG_MAKE_CORE_ERROR(1)
    //! Used to identify a Result that has yet to be initialized.
    //! It defaults to failure such that if used incorrectly will fail, as similar in concept to using an uninitialized variable.
#define SLANG_E_UNINITIALIZED               SLANG_MAKE_CORE_ERROR(2)
    //! Returned from an async method meaning the output is invalid (thus an error), but a result for the request is pending, and will be returned on a subsequent call with the async handle.
#define SLANG_E_PENDING                     SLANG_MAKE_CORE_ERROR(3)
    //! Indicates a file/resource could not be opened
#define SLANG_E_CANNOT_OPEN                 SLANG_MAKE_CORE_ERROR(4)
    //! Indicates a file/resource could not be found
#define SLANG_E_NOT_FOUND                   SLANG_MAKE_CORE_ERROR(5)
    //! An unhandled internal failure (typically from unhandled exception)
#define SLANG_E_INTERNAL_FAIL               SLANG_MAKE_CORE_ERROR(6)
    //! Could not complete because some underlying feature (hardware or software) was not available 
#define SLANG_E_NOT_AVAILABLE               SLANG_MAKE_CORE_ERROR(7)
        //! Could not complete because the operation times out. 
#define SLANG_E_TIME_OUT                    SLANG_MAKE_CORE_ERROR(8)

    /** A "Universally Unique Identifier" (UUID)

    The Slang API uses UUIDs to identify interfaces when
    using `queryInterface`.

    This type is compatible with the `GUID` type defined
    by the Component Object Model (COM), but Slang is
    not dependent on COM.
    */
    struct SlangUUID
    {
        uint32_t data1;
        uint16_t data2;
        uint16_t data3;
        uint8_t  data4[8];
    };

// Place at the start of an interface with the guid.
// Guid should be specified as SLANG_COM_INTERFACE(0x00000000, 0x0000, 0x0000, { 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46 })
// NOTE: it's the typical guid struct definition, without the surrounding {}
// It is not necessary to use the multiple parameters (we can wrap in parens), but this is simple.
#define SLANG_COM_INTERFACE(a, b, c, d0, d1, d2, d3, d4, d5, d6, d7) \
    public: \
    SLANG_FORCE_INLINE static const SlangUUID& getTypeGuid() \
    { \
        static const SlangUUID guid = { a, b, c, d0, d1, d2, d3, d4, d5, d6, d7 }; \
        return guid; \
    }

// Sometimes it's useful to associate a guid with a class to identify it. This macro can used for this,
// and the guid extracted via the getTypeGuid() function defined in the type
#define SLANG_CLASS_GUID(a, b, c, d0, d1, d2, d3, d4, d5, d6, d7) \
    SLANG_FORCE_INLINE static const SlangUUID& getTypeGuid() \
    { \
        static const SlangUUID guid = { a, b, c, d0, d1, d2, d3, d4, d5, d6, d7 }; \
        return guid; \
    }

    /** Base interface for components exchanged through the API.

    This interface definition is compatible with the COM `IUnknown`,
    and uses the same UUID, but Slang does not require applications
    to use or initialize COM.
    */
    struct ISlangUnknown
    {
        SLANG_COM_INTERFACE(0x00000000, 0x0000, 0x0000, { 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46 })

        virtual SLANG_NO_THROW SlangResult SLANG_MCALL queryInterface(SlangUUID const& uuid, void** outObject) = 0;
        virtual SLANG_NO_THROW uint32_t SLANG_MCALL addRef() = 0;
        virtual SLANG_NO_THROW uint32_t SLANG_MCALL release() = 0;

        /*
        Inline methods are provided to allow the above operations to be called
        using their traditional COM names/signatures:
        */
        SlangResult QueryInterface(struct _GUID const& uuid, void** outObject) { return queryInterface(*(SlangUUID const*)&uuid, outObject); }
        uint32_t AddRef() { return addRef(); }
        uint32_t Release() { return release(); }
    };
    #define SLANG_UUID_ISlangUnknown ISlangUnknown::getTypeGuid()

    /** A "blob" of binary data.

    This interface definition is compatible with the `ID3DBlob` and `ID3D10Blob` interfaces.
    */
    struct ISlangBlob : public ISlangUnknown
    {
        SLANG_COM_INTERFACE(0x8BA5FB08, 0x5195, 0x40e2, { 0xAC, 0x58, 0x0D, 0x98, 0x9C, 0x3A, 0x01, 0x02 })

        virtual SLANG_NO_THROW void const* SLANG_MCALL getBufferPointer() = 0;
        virtual SLANG_NO_THROW size_t SLANG_MCALL getBufferSize() = 0;
    };
    #define SLANG_UUID_ISlangBlob ISlangBlob::getTypeGuid()

    /** A (real or virtual) file system.

    Slang can make use of this interface whenever it would otherwise try to load files
    from disk, allowing applications to hook and/or override filesystem access from
    the compiler.

    It is the responsibility of 
    the caller of any method that returns a ISlangBlob to release the blob when it is no 
    longer used (using 'release').
    */

    struct ISlangFileSystem : public ISlangUnknown
    {
        SLANG_COM_INTERFACE(0x003A09FC, 0x3A4D, 0x4BA0, { 0xAD, 0x60, 0x1F, 0xD8, 0x63, 0xA9, 0x15, 0xAB })

        /** Load a file from `path` and return a blob of its contents
        @param path The path to load from, as a null-terminated UTF-8 string.
        @param outBlob A destination pointer to receive the blob of the file contents.
        @returns A `SlangResult` to indicate success or failure in loading the file.

        NOTE! This is a *binary* load - the blob should contain the exact same bytes
        as are found in the backing file. 

        If load is successful, the implementation should create a blob to hold
        the file's content, store it to `outBlob`, and return 0.
        If the load fails, the implementation should return a failure status
        (any negative value will do).
        */
        virtual SLANG_NO_THROW SlangResult SLANG_MCALL loadFile(
            char const*     path,
            ISlangBlob** outBlob) = 0;
    };
    #define SLANG_UUID_ISlangFileSystem ISlangFileSystem::getTypeGuid()


    typedef void(*SlangFuncPtr)(void);

    /** An interface that can be used to encapsulate access to a shared library. An implementation 
    does not have to implement the library as a shared library. 
    */
    struct ISlangSharedLibrary: public ISlangUnknown
    {
        SLANG_COM_INTERFACE( 0x9c9d5bc5, 0xeb61, 0x496f,{ 0x80, 0xd7, 0xd1, 0x47, 0xc4, 0xa2, 0x37, 0x30 })

            /** Get a function by name. If the library is unloaded will only return nullptr. 
            @param name The name of the function 
            @return The function pointer related to the name or nullptr if not found 
            */
        inline SlangFuncPtr SLANG_MCALL findFuncByName(char const* name)
        {
            return reinterpret_cast<SlangFuncPtr>(findSymbolAddressByName(name));
        }
            /** Get a symbol by name. If the library is unloaded will only return nullptr. 
            @param name The name of the symbol 
            @return The pointer related to the name or nullptr if not found 
            */
        virtual SLANG_NO_THROW void* SLANG_MCALL findSymbolAddressByName(char const* name) = 0;
    };
    #define SLANG_UUID_ISlangSharedLibrary ISlangSharedLibrary::getTypeGuid()

    struct ISlangSharedLibraryLoader: public ISlangUnknown
    {
        SLANG_COM_INTERFACE(0x6264ab2b, 0xa3e8, 0x4a06, { 0x97, 0xf1, 0x49, 0xbc, 0x2d, 0x2a, 0xb1, 0x4d })

            /** Load a shared library. In typical usage the library name should *not* contain any platform
            specific elements. For example on windows a dll name should *not* be passed with a '.dll' extension,
            and similarly on linux a shared library should *not* be passed with the 'lib' prefix and '.so' extension
            @path path The unadorned filename and/or path for the shared library
            @ param sharedLibraryOut Holds the shared library if successfully loaded */
        virtual SLANG_NO_THROW SlangResult SLANG_MCALL loadSharedLibrary(
            const char*     path,
            ISlangSharedLibrary** sharedLibraryOut) = 0;
    };
    #define SLANG_UUID_ISlangSharedLibraryLoader ISlangSharedLibraryLoader::getTypeGuid()
    
    /* Type that identifies how a path should be interpreted */
    typedef unsigned int SlangPathType;
    enum
    {
        SLANG_PATH_TYPE_DIRECTORY,      /**< Path specified specifies a directory. */
        SLANG_PATH_TYPE_FILE,           /**< Path specified is to a file. */
    };

    /* Callback to enumerate the contents of of a directory in a ISlangFileSystemExt.
    The name is the name of a file system object (directory/file) in the specified path (ie it is without a path) */
    typedef void (*FileSystemContentsCallBack)(SlangPathType pathType, const char* name, void* userData);

    /** An extended file system abstraction.
    
    Implementing and using this interface over ISlangFileSystem gives much more control over how paths
    are managed, as well as how it is determined if two files 'are the same'.

    All paths as input char*, or output as ISlangBlobs are always encoded as UTF-8 strings.
    Blobs that contain strings are always zero terminated.
    */
    struct ISlangFileSystemExt : public ISlangFileSystem
    {
        SLANG_COM_INTERFACE(0x5fb632d2, 0x979d, 0x4481, { 0x9f, 0xee, 0x66, 0x3c, 0x3f, 0x14, 0x49, 0xe1 })

        /** Get a uniqueIdentity which uniquely identifies an object of the file system.
           
        Given a path, returns a 'uniqueIdentity' which ideally is the same value for the same object on the file system.

        The uniqueIdentity is used to compare if two paths are the same - which amongst other things allows Slang to
        cache source contents internally. It is also used for #pragma once functionality.

        A *requirement* is for any implementation is that two paths can only return the same uniqueIdentity if the
        contents of the two files are *identical*h. If an implementation breaks this constraint it can produce incorrect compilation.
        If an implementation cannot *strictly* identify *the same* files, this will only have an effect on #pragma once behavior.

        The string for the uniqueIdentity is held zero terminated in the ISlangBlob of outUniqueIdentity.
   
        Note that there are many ways a uniqueIdentity may be generated for a file. For example it could be the
        'canonical path' - assuming it is available and unambiguous for a file system. Another possible mechanism
        could be to store the filename combined with the file date time to uniquely identify it.
     
        The client must ensure the blob be released when no longer used, otherwise memory will leak.

        NOTE! Ideally this method would be called 'getPathUniqueIdentity' but for historical reasons and
        backward compatibility it's name remains with 'File' even though an implementation should be made to work
        with directories too.

        @param path
        @param outUniqueIdentity
        @returns A `SlangResult` to indicate success or failure getting the uniqueIdentity.
        */
        virtual SLANG_NO_THROW SlangResult SLANG_MCALL getFileUniqueIdentity(
            const char* path,
            ISlangBlob** outUniqueIdentity) = 0;

        /** Calculate a path combining the 'fromPath' with 'path'

        The client must ensure the blob be released when no longer used, otherwise memory will leak.

        @param fromPathType How to interpret the from path - as a file or a directory.
        @param fromPath The from path. 
        @param path Path to be determined relative to the fromPath
        @param pathOut Holds the string which is the relative path. The string is held in the blob zero terminated.  
        @returns A `SlangResult` to indicate success or failure in loading the file.
        */
        virtual SLANG_NO_THROW SlangResult SLANG_MCALL calcCombinedPath(
            SlangPathType fromPathType,
            const char* fromPath,
            const char* path,
            ISlangBlob** pathOut) = 0;          
            
        /** Gets the type of path that path is on the file system. 
        @param path
        @param pathTypeOut
        @returns SLANG_OK if located and type is known, else an error. SLANG_E_NOT_FOUND if not found.
        */
        virtual SLANG_NO_THROW SlangResult SLANG_MCALL getPathType(
            const char* path, 
            SlangPathType* pathTypeOut) = 0;

        /** Get a simplified path. 
        Given a path, returns a simplified version of that path - typically removing '..' and/or '.'. A simplified
        path must point to the same object as the original. 
       
        This method is optional, if not implemented return SLANG_E_NOT_IMPLEMENTED.

        @param path
        @param outSimplifiedPath
        @returns SLANG_OK if successfully simplified the path (SLANG_E_NOT_IMPLEMENTED if not implemented, or some other error code)
        */
        virtual SLANG_NO_THROW SlangResult SLANG_MCALL getSimplifiedPath(
            const char* path,
            ISlangBlob** outSimplifiedPath) = 0;

        /** Get a canonical path identifies an object of the file system.

        Given a path, returns a 'canonicalPath' to the file. This may be a file system 'canonical path' to
        show where a file was read from. If the file system is say a zip file - it might include the path to the zip
        container as well as the absolute path to the specific file. The main purpose of the method is to be able
        to display to uses unambiguously where a file was read from.

        This method is optional, if not implemented return SLANG_E_NOT_IMPLEMENTED.

        @param path
        @param outCanonicalPath
        @returns SLANG_OK if successfully canonicalized the path (SLANG_E_NOT_IMPLEMENTED if not implemented, or some other error code)
        */
        virtual SLANG_NO_THROW SlangResult SLANG_MCALL getCanonicalPath(
            const char* path,
            ISlangBlob** outCanonicalPath) = 0;

        /** Clears any cached information */
        virtual SLANG_NO_THROW void SLANG_MCALL clearCache() = 0;

        /** Enumerate the contents of the path
        
        Note that for normal Slang operation it isn't necessary to enumerate contents this can return SLANG_E_NOT_IMPLEMENTED.
        
        @param The path to enumerate
        @param callback This callback is called for each entry in the path. 
        @param userData This is passed to the callback
        @returns SLANG_OK if successful 
        */
        virtual SLANG_NO_THROW SlangResult SLANG_MCALL enumeratePathContents(
            const char* path,
            FileSystemContentsCallBack callback,
            void* userData) = 0;
    };

    #define SLANG_UUID_ISlangFileSystemExt ISlangFileSystemExt::getTypeGuid()

    struct ISlangMutableFileSystem : public ISlangFileSystemExt
    {
        SLANG_COM_INTERFACE(0xa058675c, 0x1d65, 0x452a, { 0x84, 0x58, 0xcc, 0xde, 0xd1, 0x42, 0x71, 0x5 })

        /** Write the data specified with data and size to the specified path.

        @param path The path for data to be saved to
        @param data The data to be saved
        @param size The size of the data
        @returns SLANG_OK if successful (SLANG_E_NOT_IMPLEMENTED if not implemented, or some other error code)
        */
        virtual SLANG_NO_THROW SlangResult SLANG_MCALL saveFile(
            const char* path,
            const void* data,
            size_t size) = 0;

        /** Remove the entry in the path (directory of file). Will only delete an empty directory, if not empty
        will return an error.

        @param path The path to remove 
        @returns SLANG_OK if successful 
        */
        virtual SLANG_NO_THROW SlangResult SLANG_MCALL remove(
            const char* path) = 0;

        /** Create a directory.

        The path to the directory must exist

        @param path To the directory to create. The parent path *must* exist otherwise will return an error.
        @returns SLANG_OK if successful 
        */
        virtual SLANG_NO_THROW SlangResult SLANG_MCALL createDirectory(
            const char* path) = 0;
    };

    #define SLANG_UUID_ISlangMutableFileSystem ISlangMutableFileSystem::getTypeGuid()

    /* Identifies different types of writer target*/
    typedef unsigned int SlangWriterChannel;
    enum
    {
        SLANG_WRITER_CHANNEL_DIAGNOSTIC,
        SLANG_WRITER_CHANNEL_STD_OUTPUT,
        SLANG_WRITER_CHANNEL_STD_ERROR,
        SLANG_WRITER_CHANNEL_COUNT_OF,
    };

    typedef unsigned int SlangWriterMode;
    enum 
    {
        SLANG_WRITER_MODE_TEXT,
        SLANG_WRITER_MODE_BINARY,
    };

    /** A stream typically of text, used for outputting diagnostic as well as other information.
    */
    struct ISlangWriter : public ISlangUnknown
    {
        SLANG_COM_INTERFACE(0xec457f0e, 0x9add, 0x4e6b,{ 0x85, 0x1c, 0xd7, 0xfa, 0x71, 0x6d, 0x15, 0xfd })

            /** Begin an append buffer.
            NOTE! Only one append buffer can be active at any time.
            @param maxNumChars The maximum of chars that will be appended
            @returns The start of the buffer for appending to. */    
        virtual SLANG_NO_THROW char* SLANG_MCALL beginAppendBuffer(size_t maxNumChars) = 0;
            /** Ends the append buffer, and is equivalent to a write of the append buffer.
            NOTE! That an endAppendBuffer is not necessary if there are no characters to write.
            @param buffer is the start of the data to append and must be identical to last value returned from beginAppendBuffer
            @param numChars must be a value less than or equal to what was returned from last call to beginAppendBuffer
            @returns Result, will be SLANG_OK on success */
        virtual SLANG_NO_THROW SlangResult SLANG_MCALL endAppendBuffer(char* buffer, size_t numChars) = 0;
            /** Write text to the writer
            @param chars The characters to write out
            @param numChars The amount of characters
            @returns SLANG_OK on success */
        virtual SLANG_NO_THROW SlangResult SLANG_MCALL write(const char* chars, size_t numChars) = 0;
            /** Flushes any content to the output */
        virtual SLANG_NO_THROW void SLANG_MCALL flush() = 0;
            /** Determines if the writer stream is to the console, and can be used to alter the output 
            @returns Returns true if is a console writer */
        virtual SLANG_NO_THROW SlangBool SLANG_MCALL isConsole() = 0;
            /** Set the mode for the writer to use
            @param mode The mode to use
            @returns SLANG_OK on success */
        virtual SLANG_NO_THROW SlangResult SLANG_MCALL setMode(SlangWriterMode mode) = 0;
    };
    
    #define SLANG_UUID_ISlangWriter ISlangWriter::getTypeGuid()

    namespace slang {
    struct IGlobalSession;
    struct ICompileRequest;
    } // namespace slang

    /*!
    @brief An instance of the Slang library.
    */
    typedef slang::IGlobalSession SlangSession;
    

    typedef struct SlangProgramLayout SlangProgramLayout;

    /*!
    @brief A request for one or more compilation actions to be performed.
    */
    typedef struct slang::ICompileRequest SlangCompileRequest;


    /*!
    @brief Initialize an instance of the Slang library.
    */
    SLANG_API SlangSession* spCreateSession(const char* deprecated = 0);

    /*!
    @brief Clean up after an instance of the Slang library.
    */
    SLANG_API void spDestroySession(
        SlangSession*   session);

    /** @see slang::IGlobalSession::setSharedLibraryLoader
    */
    SLANG_API void spSessionSetSharedLibraryLoader(
        SlangSession*               session,
        ISlangSharedLibraryLoader*  loader);

    /** @see slang::IGlobalSession::getSharedLibraryLoader
    */
    SLANG_API ISlangSharedLibraryLoader* spSessionGetSharedLibraryLoader(
        SlangSession*   session);

    /** @see slang::IGlobalSession::checkCompileTargetSupport
    */
    SLANG_API SlangResult spSessionCheckCompileTargetSupport(
        SlangSession*       session,
        SlangCompileTarget  target);

    /** @see slang::IGlobalSession::checkPassThroughSupport
    */
    SLANG_API SlangResult spSessionCheckPassThroughSupport(
        SlangSession*       session,
        SlangPassThrough    passThrough
    );

    /** @see slang::IGlobalSession::addBuiltins
    */
    SLANG_API void spAddBuiltins(
        SlangSession*   session,
        char const*     sourcePath,
        char const*     sourceString);

        /*!
    @brief Callback type used for diagnostic output. 
    */
    typedef void(*SlangDiagnosticCallback)(
        char const* message,
        void*       userData);

    /*!
    @brief Get the build version 'tag' string. The string is the same as produced via `git describe --tags`
    for the project. If Slang is built separately from the automated build scripts
    the contents will by default be 'unknown'. Any string can be set by changing the
    contents of 'slang-tag-version.h' file and recompiling the project.

    This function will return exactly the same result as the method getBuildTag string on IGlobalSession.

    An advantage of using this function over the method is that doing so does not require the creation of
    a session, which can be a fairly costly operation.

    @return The build tag string
    */
    SLANG_API const char* spGetBuildTagString();

    /* @see slang::IGlobalSession::createCompileRequest
    */
    SLANG_API SlangCompileRequest* spCreateCompileRequest(
        SlangSession* session);

    /*!
    @brief Destroy a compile request.
    Note a request is a COM object and can be destroyed via 'Release'.
    */
    SLANG_API void spDestroyCompileRequest(
        SlangCompileRequest*    request);

    /*! @see slang::ICompileRequest::setFileSystem */
    SLANG_API void spSetFileSystem(
        SlangCompileRequest*    request,
        ISlangFileSystem*       fileSystem);

    /*! @see slang::ICompileRequest::setCompileFlags */
    SLANG_API void spSetCompileFlags(
        SlangCompileRequest*    request,
        SlangCompileFlags       flags);

    /*! @see slang::ICompileRequest::setDumpIntermediates */
    SLANG_API void spSetDumpIntermediates(
        SlangCompileRequest*    request,
        int                     enable);

    /*! @see slang::ICompileRequest::setDumpIntermediatePrefix */
    SLANG_API void spSetDumpIntermediatePrefix(
        SlangCompileRequest*    request,
        const char* prefix);

    /*! DEPRECATED: use `spSetTargetLineDirectiveMode` instead.
        @see slang::ICompileRequest::setLineDirectiveMode */
    SLANG_API void spSetLineDirectiveMode(
        SlangCompileRequest*    request,
        SlangLineDirectiveMode  mode);
        
    /*! @see slang::ICompileRequest::setTargetLineDirectiveMode */
    SLANG_API void spSetTargetLineDirectiveMode(
        SlangCompileRequest*    request,
        int targetIndex,
        SlangLineDirectiveMode  mode);

    /*! @see slang::ICompileRequest::setTargetLineDirectiveMode */
    SLANG_API void spSetTargetForceGLSLScalarBufferLayout(
        SlangCompileRequest*    request,
        int targetIndex,
        bool forceScalarLayout);

    /*! @see slang::ICompileRequest::setCodeGenTarget */
    SLANG_API void spSetCodeGenTarget(
        SlangCompileRequest*    request,
        SlangCompileTarget target);

    /*! @see slang::ICompileRequest::addCodeGenTarget */
    SLANG_API int spAddCodeGenTarget(
        SlangCompileRequest*    request,
        SlangCompileTarget      target);

    /*! @see slang::ICompileRequest::setTargetProfile */
    SLANG_API void spSetTargetProfile(
        SlangCompileRequest*    request,
        int                     targetIndex,
        SlangProfileID          profile);

    /*! @see slang::ICompileRequest::setTargetFlags */
    SLANG_API void spSetTargetFlags(
        SlangCompileRequest*    request,
        int                     targetIndex,
        SlangTargetFlags        flags);



    /*! @see slang::ICompileRequest::setTargetFloatingPointMode */
    SLANG_API void spSetTargetFloatingPointMode(
        SlangCompileRequest*    request,
        int                     targetIndex,
        SlangFloatingPointMode  mode);

    /*! @see slang::ICompileRequest::addTargetCapability */
    SLANG_API void spAddTargetCapability(
        slang::ICompileRequest* request,
        int                     targetIndex,
        SlangCapabilityID       capability);

    /* DEPRECATED: use `spSetMatrixLayoutMode` instead. */
    SLANG_API void spSetTargetMatrixLayoutMode(
        SlangCompileRequest*    request,
        int                     targetIndex,
        SlangMatrixLayoutMode   mode);

    /*! @see slang::ICompileRequest::setMatrixLayoutMode */
    SLANG_API void spSetMatrixLayoutMode(
        SlangCompileRequest*    request,
        SlangMatrixLayoutMode   mode);

    /*! @see slang::ICompileRequest::setDebugInfoLevel */
    SLANG_API void spSetDebugInfoLevel(
        SlangCompileRequest*    request,
        SlangDebugInfoLevel     level);

    /*! @see slang::ICompileRequest::setOptimizationLevel */
    SLANG_API void spSetOptimizationLevel(
        SlangCompileRequest*    request,
        SlangOptimizationLevel  level);


    
    /*! @see slang::ICompileRequest::setOutputContainerFormat */
    SLANG_API void spSetOutputContainerFormat(
        SlangCompileRequest*    request,
        SlangContainerFormat    format);

    /*! @see slang::ICompileRequest::setPassThrough */
    SLANG_API void spSetPassThrough(
        SlangCompileRequest*    request,
        SlangPassThrough        passThrough);

     /*! @see slang::ICompileRequest::setDiagnosticCallback */
    SLANG_API void spSetDiagnosticCallback(
        SlangCompileRequest*    request,
        SlangDiagnosticCallback callback,
        void const*             userData);

    /*! @see slang::ICompileRequest::setWriter */
    SLANG_API void spSetWriter(
        SlangCompileRequest*    request,
        SlangWriterChannel      channel, 
        ISlangWriter*           writer);

    /*! @see slang::ICompileRequest::getWriter */
    SLANG_API ISlangWriter* spGetWriter(
        SlangCompileRequest*    request,
        SlangWriterChannel      channel);

    /*! @see slang::ICompileRequest::addSearchPath */
    SLANG_API void spAddSearchPath(
        SlangCompileRequest*    request,
        const char*             searchDir);

   /*! @see slang::ICompileRequest::addPreprocessorDefine */
    SLANG_API void spAddPreprocessorDefine(
        SlangCompileRequest*    request,
        const char*             key,
        const char*             value);

    /*! @see slang::ICompileRequest::processCommandLineArguments */
    SLANG_API SlangResult spProcessCommandLineArguments(
        SlangCompileRequest*    request,
        char const* const*      args,
        int                     argCount);

    /*! @see slang::ICompileRequest::addTranslationUnit */
    SLANG_API int spAddTranslationUnit(
        SlangCompileRequest*    request,
        SlangSourceLanguage     language,
        char const*             name);

    
    /*! @see slang::ICompileRequest::setDefaultModuleName */
    SLANG_API void spSetDefaultModuleName(
        SlangCompileRequest*    request,
        const char* defaultModuleName);

    /*! @see slang::ICompileRequest::addPreprocessorDefine */
    SLANG_API void spTranslationUnit_addPreprocessorDefine(
        SlangCompileRequest*    request,
        int                     translationUnitIndex,
        const char*             key,
        const char*             value);


    /*! @see slang::ICompileRequest::addTranslationUnitSourceFile */
    SLANG_API void spAddTranslationUnitSourceFile(
        SlangCompileRequest*    request,
        int                     translationUnitIndex,
        char const*             path);

    /*! @see slang::ICompileRequest::addTranslationUnitSourceString */
    SLANG_API void spAddTranslationUnitSourceString(
        SlangCompileRequest*    request,
        int                     translationUnitIndex,
        char const*             path,
        char const*             source);


    /*! @see slang::ICompileRequest::addLibraryReference */
    SLANG_API SlangResult spAddLibraryReference(
        SlangCompileRequest*    request,
        const void* libData,
        size_t libDataSize);

    /*! @see slang::ICompileRequest::addTranslationUnitSourceStringSpan */
    SLANG_API void spAddTranslationUnitSourceStringSpan(
        SlangCompileRequest*    request,
        int                     translationUnitIndex,
        char const*             path,
        char const*             sourceBegin,
        char const*             sourceEnd);

    /*! @see slang::ICompileRequest::addTranslationUnitSourceBlob */
    SLANG_API void spAddTranslationUnitSourceBlob(
        SlangCompileRequest*    request,
        int                     translationUnitIndex,
        char const*             path,
        ISlangBlob*             sourceBlob);

    /*! @see slang::IGlobalSession::findProfile */
    SLANG_API SlangProfileID spFindProfile(
        SlangSession*   session,
        char const*     name);

    /*! @see slang::IGlobalSession::findCapability */
    SLANG_API SlangCapabilityID spFindCapability(
        SlangSession*   session,
        char const*     name);

    /*! @see slang::ICompileRequest::addEntryPoint */
    SLANG_API int spAddEntryPoint(
        SlangCompileRequest*    request,
        int                     translationUnitIndex,
        char const*             name,
        SlangStage              stage);

    /*! @see slang::ICompileRequest::addEntryPointEx */
    SLANG_API int spAddEntryPointEx(
        SlangCompileRequest*    request,
        int                     translationUnitIndex,
        char const*             name,
        SlangStage              stage,
        int                     genericArgCount,
        char const**            genericArgs);

    /*! @see slang::ICompileRequest::setGlobalGenericArgs */
    SLANG_API SlangResult spSetGlobalGenericArgs(
        SlangCompileRequest*    request,
        int                     genericArgCount,
        char const**            genericArgs);

    /*! @see slang::ICompileRequest::setTypeNameForGlobalExistentialTypeParam */
    SLANG_API SlangResult spSetTypeNameForGlobalExistentialTypeParam(
        SlangCompileRequest*    request,
        int                     slotIndex,
        char const*             typeName);

    /*! @see slang::ICompileRequest::setTypeNameForEntryPointExistentialTypeParam */
    SLANG_API SlangResult spSetTypeNameForEntryPointExistentialTypeParam(
        SlangCompileRequest*    request,
        int                     entryPointIndex,
        int                     slotIndex,
        char const*             typeName);

    /*! @see slang::ICompileRequest::compile */
    SLANG_API SlangResult spCompile(
        SlangCompileRequest*    request);


    /*! @see slang::ICompileRequest::getDiagnosticOutput */
    SLANG_API char const* spGetDiagnosticOutput(
        SlangCompileRequest*    request);

    /*! @see slang::ICompileRequest::getDiagnosticOutputBlob */
    SLANG_API SlangResult spGetDiagnosticOutputBlob(
        SlangCompileRequest*    request,
        ISlangBlob**            outBlob);


    /*! @see slang::ICompileRequest::getDependencyFileCount */
    SLANG_API int
    spGetDependencyFileCount(
        SlangCompileRequest*    request);

    /*! @see slang::ICompileRequest::getDependencyFilePath */
    SLANG_API char const*
    spGetDependencyFilePath(
        SlangCompileRequest*    request,
        int                     index);

    /*! @see slang::ICompileRequest::getTranslationUnitCount */
    SLANG_API int
    spGetTranslationUnitCount(
        SlangCompileRequest*    request);

    /*! @see slang::ICompileRequest::getEntryPointSource */
    SLANG_API char const* spGetEntryPointSource(
        SlangCompileRequest*    request,
        int                     entryPointIndex);

    /*! @see slang::ICompileRequest::getEntryPointCode */
    SLANG_API void const* spGetEntryPointCode(
        SlangCompileRequest*    request,
        int                     entryPointIndex,
        size_t*                 outSize);

    /*! @see slang::ICompileRequest::getEntryPointCodeBlob */
    SLANG_API SlangResult spGetEntryPointCodeBlob(
        SlangCompileRequest*    request,
        int                     entryPointIndex,
        int                     targetIndex,
        ISlangBlob**            outBlob);

    /*! @see slang::ICompileRequest::getEntryPointHostCallable */
    SLANG_API SlangResult spGetEntryPointHostCallable(
        SlangCompileRequest*    request,
        int                     entryPointIndex,
        int                     targetIndex,
        ISlangSharedLibrary**   outSharedLibrary);

    /*! @see slang::ICompileRequest::getTargetCodeBlob */
    SLANG_API SlangResult spGetTargetCodeBlob(
        SlangCompileRequest*    request,
        int                     targetIndex,
        ISlangBlob**            outBlob);

    /*! @see slang::ICompileRequest::getTargetHostCallable */
    SLANG_API SlangResult spGetTargetHostCallable(
        SlangCompileRequest*    request,
        int                     targetIndex,
        ISlangSharedLibrary**   outSharedLibrary);

    /*! @see slang::ICompileRequest::getCompileRequestCode */
    SLANG_API void const* spGetCompileRequestCode(
        SlangCompileRequest*    request,
        size_t*                 outSize);

    /*! @see slang::ICompileRequest::getContainerCode */
    SLANG_API SlangResult spGetContainerCode(
        SlangCompileRequest*    request,
        ISlangBlob**            outBlob);

    /*! @see slang::ICompileRequest::loadRepro */
    SLANG_API SlangResult spLoadRepro(
        SlangCompileRequest* request,
        ISlangFileSystem* fileSystem,
        const void* data,
        size_t size);

    /*! @see slang::ICompileRequest::saveRepro */
    SLANG_API SlangResult spSaveRepro(
        SlangCompileRequest* request,
        ISlangBlob** outBlob
    );

    /*! @see slang::ICompileRequest::enableReproCapture */
    SLANG_API SlangResult spEnableReproCapture(
        SlangCompileRequest* request);


    /** Extract contents of a repro.

    Writes the contained files and manifest with their 'unique' names into fileSystem. For more details read the
    docs/repro.md documentation. 

    @param session          The slang session
    @param reproData        Holds the repro data
    @param reproDataSize    The size of the repro data
    @param fileSystem       File system that the contents of the repro will be written to
    @returns                A `SlangResult` to indicate success or failure.
    */
    SLANG_API SlangResult spExtractRepro(
        SlangSession* session,
        const void* reproData,
        size_t reproDataSize,
        ISlangMutableFileSystem* fileSystem);

    /* Turns a repro into a file system.

    Makes the contents of the repro available as a file system - that is able to access the files with the same
    paths as were used on the original repro file system. 

    @param session          The slang session
    @param reproData        The repro data
    @param reproDataSize    The size of the repro data
    @param replaceFileSystem  Will attempt to load by unique names from this file system before using contents of the repro. Optional.
    @param outFileSystem    The file system that can be used to access contents
    @returns                A `SlangResult` to indicate success or failure.
    */
    SLANG_API SlangResult spLoadReproAsFileSystem(
        SlangSession* session,
        const void* reproData,
        size_t reproDataSize,
        ISlangFileSystem* replaceFileSystem,
        ISlangFileSystemExt** outFileSystem);
    
    /*
    Forward declarations of types used in the reflection interface;
    */

    typedef struct SlangProgramLayout SlangProgramLayout;
    typedef struct SlangEntryPoint SlangEntryPoint;
    typedef struct SlangEntryPointLayout SlangEntryPointLayout;

    typedef struct SlangReflectionModifier          SlangReflectionModifier;
    typedef struct SlangReflectionType              SlangReflectionType;
    typedef struct SlangReflectionTypeLayout        SlangReflectionTypeLayout;
    typedef struct SlangReflectionVariable          SlangReflectionVariable;
    typedef struct SlangReflectionVariableLayout    SlangReflectionVariableLayout;
    typedef struct SlangReflectionTypeParameter     SlangReflectionTypeParameter;
    typedef struct SlangReflectionUserAttribute     SlangReflectionUserAttribute;

    /*
    Type aliases to maintain backward compatibility.
    */
    typedef SlangProgramLayout SlangReflection;
    typedef SlangEntryPointLayout SlangReflectionEntryPoint;

    // get reflection data from a compilation request
    SLANG_API SlangReflection* spGetReflection(
        SlangCompileRequest*    request);

    // type reflection

    typedef unsigned int SlangTypeKind;
    enum
    {
        SLANG_TYPE_KIND_NONE,
        SLANG_TYPE_KIND_STRUCT,
        SLANG_TYPE_KIND_ARRAY,
        SLANG_TYPE_KIND_MATRIX,
        SLANG_TYPE_KIND_VECTOR,
        SLANG_TYPE_KIND_SCALAR,
        SLANG_TYPE_KIND_CONSTANT_BUFFER,
        SLANG_TYPE_KIND_RESOURCE,
        SLANG_TYPE_KIND_SAMPLER_STATE,
        SLANG_TYPE_KIND_TEXTURE_BUFFER,
        SLANG_TYPE_KIND_SHADER_STORAGE_BUFFER,
        SLANG_TYPE_KIND_PARAMETER_BLOCK,
        SLANG_TYPE_KIND_GENERIC_TYPE_PARAMETER,
        SLANG_TYPE_KIND_INTERFACE,
        SLANG_TYPE_KIND_OUTPUT_STREAM,
        SLANG_TYPE_KIND_SPECIALIZED,
        SLANG_TYPE_KIND_FEEDBACK,
        SLANG_TYPE_KIND_COUNT,
    };

    typedef unsigned int SlangScalarType;
    enum
    {
        SLANG_SCALAR_TYPE_NONE,
        SLANG_SCALAR_TYPE_VOID,
        SLANG_SCALAR_TYPE_BOOL,
        SLANG_SCALAR_TYPE_INT32,
        SLANG_SCALAR_TYPE_UINT32,
        SLANG_SCALAR_TYPE_INT64,
        SLANG_SCALAR_TYPE_UINT64,
        SLANG_SCALAR_TYPE_FLOAT16,
        SLANG_SCALAR_TYPE_FLOAT32,
        SLANG_SCALAR_TYPE_FLOAT64,
        SLANG_SCALAR_TYPE_INT8,
        SLANG_SCALAR_TYPE_UINT8,
        SLANG_SCALAR_TYPE_INT16,
        SLANG_SCALAR_TYPE_UINT16,
    };

#ifndef SLANG_RESOURCE_SHAPE
#    define SLANG_RESOURCE_SHAPE
    typedef unsigned int SlangResourceShape;
    enum
    {
        SLANG_RESOURCE_BASE_SHAPE_MASK      = 0x0F,

        SLANG_RESOURCE_NONE                 = 0x00,

        SLANG_TEXTURE_1D                    = 0x01,
        SLANG_TEXTURE_2D                    = 0x02,
        SLANG_TEXTURE_3D                    = 0x03,
        SLANG_TEXTURE_CUBE                  = 0x04,
        SLANG_TEXTURE_BUFFER                = 0x05,

        SLANG_STRUCTURED_BUFFER             = 0x06,
        SLANG_BYTE_ADDRESS_BUFFER           = 0x07,
        SLANG_RESOURCE_UNKNOWN              = 0x08,
        SLANG_ACCELERATION_STRUCTURE        = 0x09,

        SLANG_RESOURCE_EXT_SHAPE_MASK       = 0xF0,

        SLANG_TEXTURE_FEEDBACK_FLAG         = 0x10,
        SLANG_TEXTURE_ARRAY_FLAG            = 0x40,
        SLANG_TEXTURE_MULTISAMPLE_FLAG      = 0x80,

        SLANG_TEXTURE_1D_ARRAY              = SLANG_TEXTURE_1D   | SLANG_TEXTURE_ARRAY_FLAG,
        SLANG_TEXTURE_2D_ARRAY              = SLANG_TEXTURE_2D   | SLANG_TEXTURE_ARRAY_FLAG,
        SLANG_TEXTURE_CUBE_ARRAY            = SLANG_TEXTURE_CUBE | SLANG_TEXTURE_ARRAY_FLAG,

        SLANG_TEXTURE_2D_MULTISAMPLE        = SLANG_TEXTURE_2D | SLANG_TEXTURE_MULTISAMPLE_FLAG,
        SLANG_TEXTURE_2D_MULTISAMPLE_ARRAY  = SLANG_TEXTURE_2D | SLANG_TEXTURE_MULTISAMPLE_FLAG | SLANG_TEXTURE_ARRAY_FLAG,
    };
#endif
    typedef unsigned int SlangResourceAccess;
    enum
    {
        SLANG_RESOURCE_ACCESS_NONE,
        SLANG_RESOURCE_ACCESS_READ,
        SLANG_RESOURCE_ACCESS_READ_WRITE,
        SLANG_RESOURCE_ACCESS_RASTER_ORDERED,
        SLANG_RESOURCE_ACCESS_APPEND,
        SLANG_RESOURCE_ACCESS_CONSUME,
        SLANG_RESOURCE_ACCESS_WRITE,
    };

    typedef unsigned int SlangParameterCategory;
    enum
    {
        SLANG_PARAMETER_CATEGORY_NONE,
        SLANG_PARAMETER_CATEGORY_MIXED,
        SLANG_PARAMETER_CATEGORY_CONSTANT_BUFFER,
        SLANG_PARAMETER_CATEGORY_SHADER_RESOURCE,
        SLANG_PARAMETER_CATEGORY_UNORDERED_ACCESS,
        SLANG_PARAMETER_CATEGORY_VARYING_INPUT,
        SLANG_PARAMETER_CATEGORY_VARYING_OUTPUT,
        SLANG_PARAMETER_CATEGORY_SAMPLER_STATE,
        SLANG_PARAMETER_CATEGORY_UNIFORM,
        SLANG_PARAMETER_CATEGORY_DESCRIPTOR_TABLE_SLOT,
        SLANG_PARAMETER_CATEGORY_SPECIALIZATION_CONSTANT,
        SLANG_PARAMETER_CATEGORY_PUSH_CONSTANT_BUFFER,

        // HLSL register `space`, Vulkan GLSL `set`
        SLANG_PARAMETER_CATEGORY_REGISTER_SPACE,

        // A parameter whose type is to be specialized by a global generic type argument
        SLANG_PARAMETER_CATEGORY_GENERIC,

        SLANG_PARAMETER_CATEGORY_RAY_PAYLOAD,
        SLANG_PARAMETER_CATEGORY_HIT_ATTRIBUTES,
        SLANG_PARAMETER_CATEGORY_CALLABLE_PAYLOAD,
        SLANG_PARAMETER_CATEGORY_SHADER_RECORD,

        // An existential type parameter represents a "hole" that
        // needs to be filled with a concrete type to enable
        // generation of specialized code.
        //
        // Consider this example:
        //
        //      struct MyParams
        //      {
        //          IMaterial material;
        //          ILight lights[3];
        //      };
        //
        // This `MyParams` type introduces two existential type parameters:
        // one for `material` and one for `lights`. Even though `lights`
        // is an array, it only introduces one type parameter, because
        // we need to hae a *single* concrete type for all the array
        // elements to be able to generate specialized code.
        //
        SLANG_PARAMETER_CATEGORY_EXISTENTIAL_TYPE_PARAM,

        // An existential object parameter represents a value
        // that needs to be passed in to provide data for some
        // interface-type shader paameter.
        //
        // Consider this example:
        //
        //      struct MyParams
        //      {
        //          IMaterial material;
        //          ILight lights[3];
        //      };
        //
        // This `MyParams` type introduces four existential object parameters:
        // one for `material` and three for `lights` (one for each array
        // element). This is consistent with the number of interface-type
        // "objects" that are being passed through to the shader.
        //
        SLANG_PARAMETER_CATEGORY_EXISTENTIAL_OBJECT_PARAM,

        //
        SLANG_PARAMETER_CATEGORY_COUNT,


        // DEPRECATED:
        SLANG_PARAMETER_CATEGORY_VERTEX_INPUT = SLANG_PARAMETER_CATEGORY_VARYING_INPUT,
        SLANG_PARAMETER_CATEGORY_FRAGMENT_OUTPUT = SLANG_PARAMETER_CATEGORY_VARYING_OUTPUT,
    };

    /** Types of API-managed bindings that a parameter might use.
    
    `SlangBindingType` represents the distinct types of binding ranges that might be
    understood by an underlying graphics API or cross-API abstraction layer.
    Several of the enumeration cases here correspond to cases of `VkDescriptorType`
    defined by the Vulkan API. Note however that the values of this enumeration
    are not the same as those of any particular API.

    The `SlangBindingType` enumeration is distinct from `SlangParameterCategory`
    because `SlangParameterCategory` differentiates the types of parameters for
    the purposes of layout, where the layout rules of some targets will treat
    parameters of different types as occupying the same binding space for layout
    (e.g., in SPIR-V both a `Texture2D` and `SamplerState` use the same space of
    `binding` indices, and are not allowed to overlap), while those same types
    map to different types of bindingsin the API (e.g., both textures and samplers
    use different `VkDescriptorType` values).

    When you want to answer "what register/binding did this parameter use?" you
    should use `SlangParameterCategory`.

    When you wnat to answer "what type of descriptor range should this parameter use?"
    you should use `SlangBindingType`.
    */
    typedef SlangUInt32 SlangBindingType;
    enum
    {
        SLANG_BINDING_TYPE_UNKNOWN = 0,

        SLANG_BINDING_TYPE_SAMPLER,
        SLANG_BINDING_TYPE_TEXTURE,
        SLANG_BINDING_TYPE_CONSTANT_BUFFER,
        SLANG_BINDING_TYPE_PARAMETER_BLOCK,
        SLANG_BINDING_TYPE_TYPED_BUFFER,
        SLANG_BINDING_TYPE_RAW_BUFFER,
        SLANG_BINDING_TYPE_COMBINED_TEXTURE_SAMPLER,
        SLANG_BINDING_TYPE_INPUT_RENDER_TARGET,
        SLANG_BINDING_TYPE_INLINE_UNIFORM_DATA,
        SLANG_BINDING_TYPE_RAY_TRACTING_ACCELERATION_STRUCTURE,

        SLANG_BINDING_TYPE_VARYING_INPUT,
        SLANG_BINDING_TYPE_VARYING_OUTPUT,

        SLANG_BINDING_TYPE_EXISTENTIAL_VALUE,
        SLANG_BINDING_TYPE_PUSH_CONSTANT,

        SLANG_BINDING_TYPE_MUTABLE_FLAG = 0x100,

        SLANG_BINDING_TYPE_MUTABLE_TETURE = SLANG_BINDING_TYPE_TEXTURE | SLANG_BINDING_TYPE_MUTABLE_FLAG,
        SLANG_BINDING_TYPE_MUTABLE_TYPED_BUFFER = SLANG_BINDING_TYPE_TYPED_BUFFER | SLANG_BINDING_TYPE_MUTABLE_FLAG,
        SLANG_BINDING_TYPE_MUTABLE_RAW_BUFFER = SLANG_BINDING_TYPE_RAW_BUFFER | SLANG_BINDING_TYPE_MUTABLE_FLAG,

        SLANG_BINDING_TYPE_BASE_MASK = 0x00FF,
        SLANG_BINDING_TYPE_EXT_MASK  = 0xFF00,
    };

    typedef SlangUInt32 SlangLayoutRules;
    enum
    {
        SLANG_LAYOUT_RULES_DEFAULT,
    };

    typedef SlangUInt32 SlangModifierID;
    enum
    {
        SLANG_MODIFIER_SHARED,
    };

    // User Attribute
    SLANG_API char const* spReflectionUserAttribute_GetName(SlangReflectionUserAttribute* attrib);
    SLANG_API unsigned int spReflectionUserAttribute_GetArgumentCount(SlangReflectionUserAttribute* attrib);
    SLANG_API SlangReflectionType* spReflectionUserAttribute_GetArgumentType(SlangReflectionUserAttribute* attrib, unsigned int index);
    SLANG_API SlangResult spReflectionUserAttribute_GetArgumentValueInt(SlangReflectionUserAttribute* attrib, unsigned int index, int * rs);
    SLANG_API SlangResult spReflectionUserAttribute_GetArgumentValueFloat(SlangReflectionUserAttribute* attrib, unsigned int index, float * rs);

    /** Returns the string-typed value of a user attribute argument
        The string returned is not null-terminated. The length of the string is returned via `outSize`.
        If index of out of range, or if the specified argument is not a string, the function will return nullptr.
    */
    SLANG_API const char* spReflectionUserAttribute_GetArgumentValueString(SlangReflectionUserAttribute* attrib, unsigned int index, size_t * outSize);

    // Type Reflection

    SLANG_API SlangTypeKind spReflectionType_GetKind(SlangReflectionType* type);
    SLANG_API unsigned int spReflectionType_GetUserAttributeCount(SlangReflectionType* type);
    SLANG_API SlangReflectionUserAttribute* spReflectionType_GetUserAttribute(SlangReflectionType* type, unsigned int index);
    SLANG_API SlangReflectionUserAttribute* spReflectionType_FindUserAttributeByName(SlangReflectionType* type, char const* name);

    SLANG_API unsigned int spReflectionType_GetFieldCount(SlangReflectionType* type);
    SLANG_API SlangReflectionVariable* spReflectionType_GetFieldByIndex(SlangReflectionType* type, unsigned index);

        /** Returns the number of elements in the given type.

        This operation is valid for vector and array types. For other types it returns zero.

        When invoked on an unbounded-size array it will return `SLANG_UNBOUNDED_SIZE`,
        which is defined to be `~size_t(0)`.

        If the size of a type cannot be statically computed, perhaps because it depends on
        a generic parameter that has not been bound to a specific value, this function returns zero.
        */
    SLANG_API size_t spReflectionType_GetElementCount(SlangReflectionType* type);

    #define SLANG_UNBOUNDED_SIZE (~size_t(0))

    SLANG_API SlangReflectionType* spReflectionType_GetElementType(SlangReflectionType* type);

    SLANG_API unsigned int spReflectionType_GetRowCount(SlangReflectionType* type);
    SLANG_API unsigned int spReflectionType_GetColumnCount(SlangReflectionType* type);
    SLANG_API SlangScalarType spReflectionType_GetScalarType(SlangReflectionType* type);

    SLANG_API SlangResourceShape spReflectionType_GetResourceShape(SlangReflectionType* type);
    SLANG_API SlangResourceAccess spReflectionType_GetResourceAccess(SlangReflectionType* type);
    SLANG_API SlangReflectionType* spReflectionType_GetResourceResultType(SlangReflectionType* type);

    SLANG_API char const* spReflectionType_GetName(SlangReflectionType* type);

    // Type Layout Reflection

    SLANG_API SlangReflectionType* spReflectionTypeLayout_GetType(SlangReflectionTypeLayout* type);
    SLANG_API SlangTypeKind spReflectionTypeLayout_getKind(SlangReflectionTypeLayout* type);
    SLANG_API size_t spReflectionTypeLayout_GetSize(SlangReflectionTypeLayout* type, SlangParameterCategory category);
    SLANG_API size_t spReflectionTypeLayout_GetStride(SlangReflectionTypeLayout* type, SlangParameterCategory category);
    SLANG_API int32_t spReflectionTypeLayout_getAlignment(SlangReflectionTypeLayout* type, SlangParameterCategory category);

    SLANG_API SlangReflectionVariableLayout* spReflectionTypeLayout_GetFieldByIndex(SlangReflectionTypeLayout* type, unsigned index);

    SLANG_API SlangInt spReflectionTypeLayout_findFieldIndexByName(SlangReflectionTypeLayout* typeLayout, const char* nameBegin, const char* nameEnd);

    SLANG_API size_t spReflectionTypeLayout_GetElementStride(SlangReflectionTypeLayout* type, SlangParameterCategory category);
    SLANG_API SlangReflectionTypeLayout* spReflectionTypeLayout_GetElementTypeLayout(SlangReflectionTypeLayout* type);
    SLANG_API SlangReflectionVariableLayout* spReflectionTypeLayout_GetElementVarLayout(SlangReflectionTypeLayout* type);
    SLANG_API SlangReflectionVariableLayout* spReflectionTypeLayout_getContainerVarLayout(SlangReflectionTypeLayout* type);

    SLANG_API SlangParameterCategory spReflectionTypeLayout_GetParameterCategory(SlangReflectionTypeLayout* type);

    SLANG_API unsigned spReflectionTypeLayout_GetCategoryCount(SlangReflectionTypeLayout* type);
    SLANG_API SlangParameterCategory spReflectionTypeLayout_GetCategoryByIndex(SlangReflectionTypeLayout* type, unsigned index);

    SLANG_API SlangMatrixLayoutMode spReflectionTypeLayout_GetMatrixLayoutMode(SlangReflectionTypeLayout* type);

    SLANG_API int spReflectionTypeLayout_getGenericParamIndex(SlangReflectionTypeLayout* type);

    SLANG_API SlangReflectionTypeLayout* spReflectionTypeLayout_getPendingDataTypeLayout(SlangReflectionTypeLayout* type);

    SLANG_API SlangReflectionVariableLayout* spReflectionTypeLayout_getSpecializedTypePendingDataVarLayout(SlangReflectionTypeLayout* type);
    SLANG_API SlangInt spReflectionType_getSpecializedTypeArgCount(SlangReflectionType* type);
    SLANG_API SlangReflectionType* spReflectionType_getSpecializedTypeArgType(SlangReflectionType* type, SlangInt index);

    SLANG_API SlangInt spReflectionTypeLayout_getBindingRangeCount(SlangReflectionTypeLayout* typeLayout);
    SLANG_API SlangBindingType spReflectionTypeLayout_getBindingRangeType(SlangReflectionTypeLayout* typeLayout, SlangInt index);
    SLANG_API SlangInt spReflectionTypeLayout_getBindingRangeBindingCount(SlangReflectionTypeLayout* typeLayout, SlangInt index);
    SLANG_API SlangReflectionTypeLayout* spReflectionTypeLayout_getBindingRangeLeafTypeLayout(SlangReflectionTypeLayout* typeLayout, SlangInt index);
    SLANG_API SlangReflectionVariable* spReflectionTypeLayout_getBindingRangeLeafVariable(SlangReflectionTypeLayout* typeLayout, SlangInt index);
    SLANG_API SlangInt spReflectionTypeLayout_getFieldBindingRangeOffset(SlangReflectionTypeLayout* typeLayout, SlangInt fieldIndex);

    SLANG_API SlangInt spReflectionTypeLayout_getBindingRangeDescriptorSetIndex(SlangReflectionTypeLayout* typeLayout, SlangInt index);
    SLANG_API SlangInt spReflectionTypeLayout_getBindingRangeFirstDescriptorRangeIndex(SlangReflectionTypeLayout* typeLayout, SlangInt index);
    SLANG_API SlangInt spReflectionTypeLayout_getBindingRangeDescriptorRangeCount(SlangReflectionTypeLayout* typeLayout, SlangInt index);

    SLANG_API SlangInt spReflectionTypeLayout_getDescriptorSetCount(SlangReflectionTypeLayout* typeLayout);
    SLANG_API SlangInt spReflectionTypeLayout_getDescriptorSetSpaceOffset(SlangReflectionTypeLayout* typeLayout, SlangInt setIndex);
    SLANG_API SlangInt spReflectionTypeLayout_getDescriptorSetDescriptorRangeCount(SlangReflectionTypeLayout* typeLayout, SlangInt setIndex);
    SLANG_API SlangInt spReflectionTypeLayout_getDescriptorSetDescriptorRangeIndexOffset(SlangReflectionTypeLayout* typeLayout, SlangInt setIndex, SlangInt rangeIndex);
    SLANG_API SlangInt spReflectionTypeLayout_getDescriptorSetDescriptorRangeDescriptorCount(SlangReflectionTypeLayout* typeLayout, SlangInt setIndex, SlangInt rangeIndex);
    SLANG_API SlangBindingType spReflectionTypeLayout_getDescriptorSetDescriptorRangeType(SlangReflectionTypeLayout* typeLayout, SlangInt setIndex, SlangInt rangeIndex);
    SLANG_API SlangParameterCategory spReflectionTypeLayout_getDescriptorSetDescriptorRangeCategory(SlangReflectionTypeLayout* typeLayout, SlangInt setIndex, SlangInt rangeIndex);

    SLANG_API SlangInt spReflectionTypeLayout_getSubObjectRangeCount(SlangReflectionTypeLayout* typeLayout);
    SLANG_API SlangInt spReflectionTypeLayout_getSubObjectRangeBindingRangeIndex(SlangReflectionTypeLayout* typeLayout, SlangInt subObjectRangeIndex);
    SLANG_API SlangInt spReflectionTypeLayout_getSubObjectRangeSpaceOffset(SlangReflectionTypeLayout* typeLayout, SlangInt subObjectRangeIndex);
    SLANG_API SlangReflectionVariableLayout* spReflectionTypeLayout_getSubObjectRangeOffset(SlangReflectionTypeLayout* typeLayout, SlangInt subObjectRangeIndex);

#if 0
    SLANG_API SlangInt spReflectionTypeLayout_getSubObjectRangeCount(SlangReflectionTypeLayout* typeLayout);
    SLANG_API SlangInt spReflectionTypeLayout_getSubObjectRangeObjectCount(SlangReflectionTypeLayout* typeLayout, SlangInt index);
    SLANG_API SlangInt spReflectionTypeLayout_getSubObjectRangeBindingRangeIndex(SlangReflectionTypeLayout* typeLayout, SlangInt index);
    SLANG_API SlangReflectionTypeLayout* spReflectionTypeLayout_getSubObjectRangeTypeLayout(SlangReflectionTypeLayout* typeLayout, SlangInt index);

    SLANG_API SlangInt spReflectionTypeLayout_getSubObjectRangeDescriptorRangeCount(SlangReflectionTypeLayout* typeLayout, SlangInt subObjectRangeIndex);
    SLANG_API SlangBindingType spReflectionTypeLayout_getSubObjectRangeDescriptorRangeBindingType(SlangReflectionTypeLayout* typeLayout, SlangInt subObjectRangeIndex, SlangInt bindingRangeIndexInSubObject);
    SLANG_API SlangInt spReflectionTypeLayout_getSubObjectRangeDescriptorRangeBindingCount(SlangReflectionTypeLayout* typeLayout, SlangInt subObjectRangeIndex, SlangInt bindingRangeIndexInSubObject);
    SLANG_API SlangInt spReflectionTypeLayout_getSubObjectRangeDescriptorRangeIndexOffset(SlangReflectionTypeLayout* typeLayout, SlangInt subObjectRangeIndex, SlangInt bindingRangeIndexInSubObject);
    SLANG_API SlangInt spReflectionTypeLayout_getSubObjectRangeDescriptorRangeSpaceOffset(SlangReflectionTypeLayout* typeLayout, SlangInt subObjectRangeIndex, SlangInt bindingRangeIndexInSubObject);
#endif

    // Variable Reflection

    SLANG_API char const* spReflectionVariable_GetName(SlangReflectionVariable* var);
    SLANG_API SlangReflectionType* spReflectionVariable_GetType(SlangReflectionVariable* var);
    SLANG_API SlangReflectionModifier* spReflectionVariable_FindModifier(SlangReflectionVariable* var, SlangModifierID modifierID);
    SLANG_API unsigned int spReflectionVariable_GetUserAttributeCount(SlangReflectionVariable* var);
    SLANG_API SlangReflectionUserAttribute* spReflectionVariable_GetUserAttribute(SlangReflectionVariable* var, unsigned int index);
    SLANG_API SlangReflectionUserAttribute* spReflectionVariable_FindUserAttributeByName(SlangReflectionVariable* var, SlangSession * session, char const* name);

    // Variable Layout Reflection

    SLANG_API SlangReflectionVariable* spReflectionVariableLayout_GetVariable(SlangReflectionVariableLayout* var);

    SLANG_API SlangReflectionTypeLayout* spReflectionVariableLayout_GetTypeLayout(SlangReflectionVariableLayout* var);

    SLANG_API size_t spReflectionVariableLayout_GetOffset(SlangReflectionVariableLayout* var, SlangParameterCategory category);
    SLANG_API size_t spReflectionVariableLayout_GetSpace(SlangReflectionVariableLayout* var, SlangParameterCategory category);

    SLANG_API char const* spReflectionVariableLayout_GetSemanticName(SlangReflectionVariableLayout* var);
    SLANG_API size_t spReflectionVariableLayout_GetSemanticIndex(SlangReflectionVariableLayout* var);

    /** Get the stage that a variable belongs to (if any).

    A variable "belongs" to a specific stage when it is a varying input/output
    parameter either defined as part of the parameter list for an entry
    point *or* at the global scope of a stage-specific GLSL code file (e.g.,
    an `in` parameter in a GLSL `.vs` file belongs to the vertex stage).
    */
    SLANG_API SlangStage spReflectionVariableLayout_getStage(
        SlangReflectionVariableLayout* var);


    SLANG_API SlangReflectionVariableLayout* spReflectionVariableLayout_getPendingDataLayout(SlangReflectionVariableLayout* var);

    // Shader Parameter Reflection

    typedef SlangReflectionVariableLayout SlangReflectionParameter;

    SLANG_API unsigned spReflectionParameter_GetBindingIndex(SlangReflectionParameter* parameter);
    SLANG_API unsigned spReflectionParameter_GetBindingSpace(SlangReflectionParameter* parameter);

    // Entry Point Reflection

    SLANG_API char const* spReflectionEntryPoint_getName(
        SlangReflectionEntryPoint* entryPoint);

    SLANG_API char const* spReflectionEntryPoint_getNameOverride(
        SlangReflectionEntryPoint* entryPoint);

    SLANG_API unsigned spReflectionEntryPoint_getParameterCount(
        SlangReflectionEntryPoint* entryPoint);

    SLANG_API SlangReflectionVariableLayout* spReflectionEntryPoint_getParameterByIndex(
        SlangReflectionEntryPoint*  entryPoint,
        unsigned                    index);

    SLANG_API SlangStage spReflectionEntryPoint_getStage(SlangReflectionEntryPoint* entryPoint);

    SLANG_API void spReflectionEntryPoint_getComputeThreadGroupSize(
        SlangReflectionEntryPoint*  entryPoint,
        SlangUInt                   axisCount,
        SlangUInt*                  outSizeAlongAxis);

    SLANG_API int spReflectionEntryPoint_usesAnySampleRateInput(
        SlangReflectionEntryPoint* entryPoint);

    SLANG_API SlangReflectionVariableLayout* spReflectionEntryPoint_getVarLayout(
        SlangReflectionEntryPoint* entryPoint);

    SLANG_API SlangReflectionVariableLayout* spReflectionEntryPoint_getResultVarLayout(
        SlangReflectionEntryPoint* entryPoint);

    SLANG_API int spReflectionEntryPoint_hasDefaultConstantBuffer(
        SlangReflectionEntryPoint* entryPoint);

    // SlangReflectionTypeParameter
    SLANG_API char const* spReflectionTypeParameter_GetName(SlangReflectionTypeParameter* typeParam);
    SLANG_API unsigned spReflectionTypeParameter_GetIndex(SlangReflectionTypeParameter* typeParam);
    SLANG_API unsigned spReflectionTypeParameter_GetConstraintCount(SlangReflectionTypeParameter* typeParam);
    SLANG_API SlangReflectionType* spReflectionTypeParameter_GetConstraintByIndex(SlangReflectionTypeParameter* typeParam, unsigned int index);

    // Shader Reflection

    SLANG_API unsigned spReflection_GetParameterCount(SlangReflection* reflection);
    SLANG_API SlangReflectionParameter* spReflection_GetParameterByIndex(SlangReflection* reflection, unsigned index);

    SLANG_API unsigned int spReflection_GetTypeParameterCount(SlangReflection* reflection);
    SLANG_API SlangReflectionTypeParameter* spReflection_GetTypeParameterByIndex(SlangReflection* reflection, unsigned int index);
    SLANG_API SlangReflectionTypeParameter* spReflection_FindTypeParameter(SlangReflection* reflection, char const* name);

    SLANG_API SlangReflectionType* spReflection_FindTypeByName(SlangReflection* reflection, char const* name);
    SLANG_API SlangReflectionTypeLayout* spReflection_GetTypeLayout(SlangReflection* reflection, SlangReflectionType* reflectionType, SlangLayoutRules rules);

    SLANG_API SlangUInt spReflection_getEntryPointCount(SlangReflection* reflection);
    SLANG_API SlangReflectionEntryPoint* spReflection_getEntryPointByIndex(SlangReflection* reflection, SlangUInt index);
    SLANG_API SlangReflectionEntryPoint* spReflection_findEntryPointByName(SlangReflection* reflection, char const* name);

    SLANG_API SlangUInt spReflection_getGlobalConstantBufferBinding(SlangReflection* reflection);
    SLANG_API size_t spReflection_getGlobalConstantBufferSize(SlangReflection* reflection);

    SLANG_API  SlangReflectionType* spReflection_specializeType(
        SlangReflection*            reflection,
        SlangReflectionType*        type,
        SlangInt                    specializationArgCount,
        SlangReflectionType* const* specializationArgs,
        ISlangBlob**                outDiagnostics);

        /// Get the number of hashed strings
    SLANG_API SlangUInt spReflection_getHashedStringCount(
        SlangReflection*  reflection);

        /// Get a hashed string. The number of chars is written in outCount.
        /// The count does *NOT* including terminating 0. The returned string will be 0 terminated. 
    SLANG_API const char* spReflection_getHashedString(
        SlangReflection*  reflection,
        SlangUInt index,
        size_t* outCount);

        /// Compute a string hash.
        /// Count should *NOT* include terminating zero.
    SLANG_API int spComputeStringHash(const char* chars, size_t count);

        /// Get a type layout representing reflection information for the global-scope prameters.
    SLANG_API SlangReflectionTypeLayout* spReflection_getGlobalParamsTypeLayout(
        SlangReflection* reflection);

        /// Get a variable layout representing reflection information for the global-scope prameters.
    SLANG_API SlangReflectionVariableLayout* spReflection_getGlobalParamsVarLayout(
        SlangReflection* reflection);

#ifdef __cplusplus
}

/* Helper interfaces for C++ users */
namespace slang
{
    struct BufferReflection;
    struct TypeLayoutReflection;
    struct TypeReflection;
    struct VariableLayoutReflection;
    struct VariableReflection;
    
    struct UserAttribute
    {
        char const* getName()
        {
            return spReflectionUserAttribute_GetName((SlangReflectionUserAttribute*)this);
        }
        uint32_t getArgumentCount()
        {
            return (uint32_t)spReflectionUserAttribute_GetArgumentCount((SlangReflectionUserAttribute*)this);
        }
        TypeReflection* getArgumentType(uint32_t index)
        {
            return (TypeReflection*)spReflectionUserAttribute_GetArgumentType((SlangReflectionUserAttribute*)this, index);
        }
        SlangResult getArgumentValueInt(uint32_t index, int * value)
        {
            return spReflectionUserAttribute_GetArgumentValueInt((SlangReflectionUserAttribute*)this, index, value);
        }
        SlangResult getArgumentValueFloat(uint32_t index, float * value)
        {
            return spReflectionUserAttribute_GetArgumentValueFloat((SlangReflectionUserAttribute*)this, index, value);
        }
        const char* getArgumentValueString(uint32_t index, size_t * outSize)
        {
            return spReflectionUserAttribute_GetArgumentValueString((SlangReflectionUserAttribute*)this, index, outSize);
        }
    };

    struct TypeReflection
    {
        enum class Kind
        {
            None    = SLANG_TYPE_KIND_NONE,
            Struct  = SLANG_TYPE_KIND_STRUCT,
            Array   = SLANG_TYPE_KIND_ARRAY,
            Matrix  = SLANG_TYPE_KIND_MATRIX,
            Vector  = SLANG_TYPE_KIND_VECTOR,
            Scalar  = SLANG_TYPE_KIND_SCALAR,
            ConstantBuffer = SLANG_TYPE_KIND_CONSTANT_BUFFER,
            Resource = SLANG_TYPE_KIND_RESOURCE,
            SamplerState = SLANG_TYPE_KIND_SAMPLER_STATE,
            TextureBuffer = SLANG_TYPE_KIND_TEXTURE_BUFFER,
            ShaderStorageBuffer = SLANG_TYPE_KIND_SHADER_STORAGE_BUFFER,
            ParameterBlock = SLANG_TYPE_KIND_PARAMETER_BLOCK,
            GenericTypeParameter = SLANG_TYPE_KIND_GENERIC_TYPE_PARAMETER,
            Interface = SLANG_TYPE_KIND_INTERFACE,
            OutputStream = SLANG_TYPE_KIND_OUTPUT_STREAM,
            Specialized = SLANG_TYPE_KIND_SPECIALIZED,
            Feedback = SLANG_TYPE_KIND_FEEDBACK,
        };

        enum ScalarType : SlangScalarType
        {
            None    = SLANG_SCALAR_TYPE_NONE,
            Void    = SLANG_SCALAR_TYPE_VOID,
            Bool    = SLANG_SCALAR_TYPE_BOOL,
            Int32   = SLANG_SCALAR_TYPE_INT32,
            UInt32  = SLANG_SCALAR_TYPE_UINT32,
            Int64   = SLANG_SCALAR_TYPE_INT64,
            UInt64  = SLANG_SCALAR_TYPE_UINT64,
            Float16 = SLANG_SCALAR_TYPE_FLOAT16,
            Float32 = SLANG_SCALAR_TYPE_FLOAT32,
            Float64 = SLANG_SCALAR_TYPE_FLOAT64,
            Int8    = SLANG_SCALAR_TYPE_INT8,
            UInt8   = SLANG_SCALAR_TYPE_UINT8,
            Int16   = SLANG_SCALAR_TYPE_INT16,
            UInt16  = SLANG_SCALAR_TYPE_UINT16,
        };

        Kind getKind()
        {
            return (Kind) spReflectionType_GetKind((SlangReflectionType*) this);
        }

        // only useful if `getKind() == Kind::Struct`
        unsigned int getFieldCount()
        {
            return spReflectionType_GetFieldCount((SlangReflectionType*) this);
        }

        VariableReflection* getFieldByIndex(unsigned int index)
        {
            return (VariableReflection*) spReflectionType_GetFieldByIndex((SlangReflectionType*) this, index);
        }

        bool isArray() { return getKind() == TypeReflection::Kind::Array; }

        TypeReflection* unwrapArray()
        {
            TypeReflection* type = this;
            while( type->isArray() )
            {
                type = type->getElementType();
            }
            return type;
        }

        // only useful if `getKind() == Kind::Array`
        size_t getElementCount()
        {
            return spReflectionType_GetElementCount((SlangReflectionType*) this);
        }

        size_t getTotalArrayElementCount()
        {
            if(!isArray()) return 0;
            size_t result = 1;
            TypeReflection* type = this;
            for(;;)
            {
                if(!type->isArray())
                    return result;

                result *= type->getElementCount();
                type = type->getElementType();
            }
        }

        TypeReflection* getElementType()
        {
            return (TypeReflection*) spReflectionType_GetElementType((SlangReflectionType*) this);
        }

        unsigned getRowCount()
        {
            return spReflectionType_GetRowCount((SlangReflectionType*) this);
        }

        unsigned getColumnCount()
        {
            return spReflectionType_GetColumnCount((SlangReflectionType*) this);
        }

        ScalarType getScalarType()
        {
            return (ScalarType) spReflectionType_GetScalarType((SlangReflectionType*) this);
        }

        TypeReflection* getResourceResultType()
        {
            return (TypeReflection*) spReflectionType_GetResourceResultType((SlangReflectionType*) this);
        }

        SlangResourceShape getResourceShape()
        {
            return spReflectionType_GetResourceShape((SlangReflectionType*) this);
        }

        SlangResourceAccess getResourceAccess()
        {
            return spReflectionType_GetResourceAccess((SlangReflectionType*) this);
        }

        char const* getName()
        {
            return spReflectionType_GetName((SlangReflectionType*) this);
        }

        unsigned int getUserAttributeCount()
        {
            return spReflectionType_GetUserAttributeCount((SlangReflectionType*)this);
        }
        UserAttribute* getUserAttributeByIndex(unsigned int index)
        {
            return (UserAttribute*)spReflectionType_GetUserAttribute((SlangReflectionType*)this, index);
        }
        UserAttribute* findUserAttributeByName(char const* name)
        {
            return (UserAttribute*)spReflectionType_FindUserAttributeByName((SlangReflectionType*)this, name);
        }
    };

    enum ParameterCategory : SlangParameterCategory
    {
        // TODO: these aren't scoped...
        None = SLANG_PARAMETER_CATEGORY_NONE,
        Mixed = SLANG_PARAMETER_CATEGORY_MIXED,
        ConstantBuffer = SLANG_PARAMETER_CATEGORY_CONSTANT_BUFFER,
        ShaderResource = SLANG_PARAMETER_CATEGORY_SHADER_RESOURCE,
        UnorderedAccess = SLANG_PARAMETER_CATEGORY_UNORDERED_ACCESS,
        VaryingInput = SLANG_PARAMETER_CATEGORY_VARYING_INPUT,
        VaryingOutput = SLANG_PARAMETER_CATEGORY_VARYING_OUTPUT,
        SamplerState = SLANG_PARAMETER_CATEGORY_SAMPLER_STATE,
        Uniform = SLANG_PARAMETER_CATEGORY_UNIFORM,
        DescriptorTableSlot = SLANG_PARAMETER_CATEGORY_DESCRIPTOR_TABLE_SLOT,
        SpecializationConstant = SLANG_PARAMETER_CATEGORY_SPECIALIZATION_CONSTANT,
        PushConstantBuffer = SLANG_PARAMETER_CATEGORY_PUSH_CONSTANT_BUFFER,
        RegisterSpace = SLANG_PARAMETER_CATEGORY_REGISTER_SPACE,
        GenericResource = SLANG_PARAMETER_CATEGORY_GENERIC,

        RayPayload = SLANG_PARAMETER_CATEGORY_RAY_PAYLOAD,
        HitAttributes = SLANG_PARAMETER_CATEGORY_HIT_ATTRIBUTES,
        CallablePayload = SLANG_PARAMETER_CATEGORY_CALLABLE_PAYLOAD,

        ShaderRecord = SLANG_PARAMETER_CATEGORY_SHADER_RECORD,

        ExistentialTypeParam = SLANG_PARAMETER_CATEGORY_EXISTENTIAL_TYPE_PARAM,
        ExistentialObjectParam = SLANG_PARAMETER_CATEGORY_EXISTENTIAL_OBJECT_PARAM,

        // DEPRECATED:
        VertexInput = SLANG_PARAMETER_CATEGORY_VERTEX_INPUT,
        FragmentOutput = SLANG_PARAMETER_CATEGORY_FRAGMENT_OUTPUT,
    };

    enum class BindingType : SlangBindingType
    {
        Unknown                             = SLANG_BINDING_TYPE_UNKNOWN,

        Sampler                             = SLANG_BINDING_TYPE_SAMPLER,
        Texture                             = SLANG_BINDING_TYPE_TEXTURE,
        ConstantBuffer                      = SLANG_BINDING_TYPE_CONSTANT_BUFFER,
        ParameterBlock                      = SLANG_BINDING_TYPE_PARAMETER_BLOCK,
        TypedBuffer                         = SLANG_BINDING_TYPE_TYPED_BUFFER,
        RawBuffer                           = SLANG_BINDING_TYPE_RAW_BUFFER,
        CombinedTextureSampler              = SLANG_BINDING_TYPE_COMBINED_TEXTURE_SAMPLER,
        InputRenderTarget                   = SLANG_BINDING_TYPE_INPUT_RENDER_TARGET,
        InlineUniformData                   = SLANG_BINDING_TYPE_INLINE_UNIFORM_DATA,
        RayTracingAccelerationStructure     = SLANG_BINDING_TYPE_RAY_TRACTING_ACCELERATION_STRUCTURE,
        VaryingInput                        = SLANG_BINDING_TYPE_VARYING_INPUT,
        VaryingOutput                       = SLANG_BINDING_TYPE_VARYING_OUTPUT,
        ExistentialValue                    = SLANG_BINDING_TYPE_EXISTENTIAL_VALUE,
        PushConstant                        = SLANG_BINDING_TYPE_PUSH_CONSTANT,

        MutableFlag                         = SLANG_BINDING_TYPE_MUTABLE_FLAG,

        MutableTexture                      = SLANG_BINDING_TYPE_MUTABLE_TETURE,
        MutableTypedBuffer                  = SLANG_BINDING_TYPE_MUTABLE_TYPED_BUFFER,
        MutableRawBuffer                    = SLANG_BINDING_TYPE_MUTABLE_RAW_BUFFER,

        BaseMask                            = SLANG_BINDING_TYPE_BASE_MASK,
        ExtMask                             = SLANG_BINDING_TYPE_EXT_MASK,
    };

    struct TypeLayoutReflection
    {
        TypeReflection* getType()
        {
            return (TypeReflection*) spReflectionTypeLayout_GetType((SlangReflectionTypeLayout*) this);
        }

        TypeReflection::Kind getKind()
        {
            return (TypeReflection::Kind) spReflectionTypeLayout_getKind((SlangReflectionTypeLayout*) this);
        }

        size_t getSize(SlangParameterCategory category = SLANG_PARAMETER_CATEGORY_UNIFORM)
        {
            return spReflectionTypeLayout_GetSize((SlangReflectionTypeLayout*) this, category);
        }

        size_t getStride(SlangParameterCategory category = SLANG_PARAMETER_CATEGORY_UNIFORM)
        {
            return spReflectionTypeLayout_GetStride((SlangReflectionTypeLayout*) this, category);
        }

        int32_t getAlignment(SlangParameterCategory category = SLANG_PARAMETER_CATEGORY_UNIFORM)
        {
            return spReflectionTypeLayout_getAlignment((SlangReflectionTypeLayout*) this, category);
        }

        unsigned int getFieldCount()
        {
            return getType()->getFieldCount();
        }

        VariableLayoutReflection* getFieldByIndex(unsigned int index)
        {
            return (VariableLayoutReflection*) spReflectionTypeLayout_GetFieldByIndex((SlangReflectionTypeLayout*) this, index);
        }

        SlangInt findFieldIndexByName(char const* nameBegin, char const* nameEnd = nullptr)
        {
            return spReflectionTypeLayout_findFieldIndexByName((SlangReflectionTypeLayout*) this, nameBegin, nameEnd);
        }

        bool isArray() { return getType()->isArray(); }

        TypeLayoutReflection* unwrapArray()
        {
            TypeLayoutReflection* typeLayout = this;
            while( typeLayout->isArray() )
            {
                typeLayout = typeLayout->getElementTypeLayout();
            }
            return typeLayout;
        }

        // only useful if `getKind() == Kind::Array`
        size_t getElementCount()
        {
            return getType()->getElementCount();
        }

        size_t getTotalArrayElementCount()
        {
            return getType()->getTotalArrayElementCount();
        }

        size_t getElementStride(SlangParameterCategory category)
        {
            return spReflectionTypeLayout_GetElementStride((SlangReflectionTypeLayout*) this, category);
        }

        TypeLayoutReflection* getElementTypeLayout()
        {
            return (TypeLayoutReflection*) spReflectionTypeLayout_GetElementTypeLayout((SlangReflectionTypeLayout*) this);
        }

        VariableLayoutReflection* getElementVarLayout()
        {
            return (VariableLayoutReflection*)spReflectionTypeLayout_GetElementVarLayout((SlangReflectionTypeLayout*) this);
        }

        VariableLayoutReflection* getContainerVarLayout()
        {
            return (VariableLayoutReflection*)spReflectionTypeLayout_getContainerVarLayout((SlangReflectionTypeLayout*) this);
        }

        // How is this type supposed to be bound?
        ParameterCategory getParameterCategory()
        {
            return (ParameterCategory) spReflectionTypeLayout_GetParameterCategory((SlangReflectionTypeLayout*) this);
        }

        unsigned int getCategoryCount()
        {
            return spReflectionTypeLayout_GetCategoryCount((SlangReflectionTypeLayout*) this);
        }

        ParameterCategory getCategoryByIndex(unsigned int index)
        {
            return (ParameterCategory) spReflectionTypeLayout_GetCategoryByIndex((SlangReflectionTypeLayout*) this, index);
        }

        unsigned getRowCount()
        {
            return getType()->getRowCount();
        }

        unsigned getColumnCount()
        {
            return getType()->getColumnCount();
        }

        TypeReflection::ScalarType getScalarType()
        {
            return getType()->getScalarType();
        }

        TypeReflection* getResourceResultType()
        {
            return getType()->getResourceResultType();
        }

        SlangResourceShape getResourceShape()
        {
            return getType()->getResourceShape();
        }

        SlangResourceAccess getResourceAccess()
        {
            return getType()->getResourceAccess();
        }

        char const* getName()
        {
            return getType()->getName();
        }

        SlangMatrixLayoutMode getMatrixLayoutMode()
        {
            return spReflectionTypeLayout_GetMatrixLayoutMode((SlangReflectionTypeLayout*) this);
        }

        int getGenericParamIndex()
        {
            return spReflectionTypeLayout_getGenericParamIndex(
                (SlangReflectionTypeLayout*) this);
        }

        TypeLayoutReflection* getPendingDataTypeLayout()
        {
            return (TypeLayoutReflection*) spReflectionTypeLayout_getPendingDataTypeLayout(
                (SlangReflectionTypeLayout*) this);
        }

        VariableLayoutReflection* getSpecializedTypePendingDataVarLayout()
        {
            return (VariableLayoutReflection*) spReflectionTypeLayout_getSpecializedTypePendingDataVarLayout(
                (SlangReflectionTypeLayout*) this);
        }

        SlangInt getBindingRangeCount()
        {
            return spReflectionTypeLayout_getBindingRangeCount(
                (SlangReflectionTypeLayout*) this);
        }

        BindingType getBindingRangeType(SlangInt index)
        {
            return (BindingType) spReflectionTypeLayout_getBindingRangeType(
                (SlangReflectionTypeLayout*) this,
                index);
        }

        SlangInt getBindingRangeBindingCount(SlangInt index)
        {
            return spReflectionTypeLayout_getBindingRangeBindingCount(
                (SlangReflectionTypeLayout*) this,
                index);
        }

        /*
        SlangInt getBindingRangeIndexOffset(SlangInt index)
        {
            return spReflectionTypeLayout_getBindingRangeIndexOffset(
                (SlangReflectionTypeLayout*) this,
                index);
        }

        SlangInt getBindingRangeSpaceOffset(SlangInt index)
        {
            return spReflectionTypeLayout_getBindingRangeSpaceOffset(
                (SlangReflectionTypeLayout*) this,
                index);
        }
        */

        SlangInt getFieldBindingRangeOffset(SlangInt fieldIndex)
        {
            return spReflectionTypeLayout_getFieldBindingRangeOffset(
                (SlangReflectionTypeLayout*) this,
                fieldIndex);
        }

        TypeLayoutReflection* getBindingRangeLeafTypeLayout(SlangInt index)
        {
            return (TypeLayoutReflection*) spReflectionTypeLayout_getBindingRangeLeafTypeLayout(
                (SlangReflectionTypeLayout*) this,
                index);
        }

        VariableReflection* getBindingRangeLeafVariable(SlangInt index)
        {
            return (VariableReflection*)spReflectionTypeLayout_getBindingRangeLeafVariable(
                (SlangReflectionTypeLayout*)this, index);
        }

        SlangInt getBindingRangeDescriptorSetIndex(SlangInt index)
        {
            return spReflectionTypeLayout_getBindingRangeDescriptorSetIndex(
                (SlangReflectionTypeLayout*) this,
                index);
        }

        SlangInt getBindingRangeFirstDescriptorRangeIndex(SlangInt index)
        {
            return spReflectionTypeLayout_getBindingRangeFirstDescriptorRangeIndex(
                (SlangReflectionTypeLayout*) this,
                index);
        }

        SlangInt getBindingRangeDescriptorRangeCount(SlangInt index)
        {
            return spReflectionTypeLayout_getBindingRangeDescriptorRangeCount(
                (SlangReflectionTypeLayout*) this,
                index);
        }

        SlangInt getDescriptorSetCount()
        {
            return spReflectionTypeLayout_getDescriptorSetCount(
                (SlangReflectionTypeLayout*) this);
        }

        SlangInt getDescriptorSetSpaceOffset(SlangInt setIndex)
        {
            return spReflectionTypeLayout_getDescriptorSetSpaceOffset(
                (SlangReflectionTypeLayout*) this,
                setIndex);
        }

        SlangInt getDescriptorSetDescriptorRangeCount(SlangInt setIndex)
        {
            return spReflectionTypeLayout_getDescriptorSetDescriptorRangeCount(
                (SlangReflectionTypeLayout*) this,
                setIndex);
        }

        SlangInt getDescriptorSetDescriptorRangeIndexOffset(SlangInt setIndex, SlangInt rangeIndex)
        {
            return spReflectionTypeLayout_getDescriptorSetDescriptorRangeIndexOffset(
                (SlangReflectionTypeLayout*) this,
                setIndex,
                rangeIndex);
        }

        SlangInt getDescriptorSetDescriptorRangeDescriptorCount(SlangInt setIndex, SlangInt rangeIndex)
        {
            return spReflectionTypeLayout_getDescriptorSetDescriptorRangeDescriptorCount(
                (SlangReflectionTypeLayout*) this,
                setIndex,
                rangeIndex);
        }

        BindingType getDescriptorSetDescriptorRangeType(SlangInt setIndex, SlangInt rangeIndex)
        {
            return (BindingType) spReflectionTypeLayout_getDescriptorSetDescriptorRangeType(
                (SlangReflectionTypeLayout*) this,
                setIndex,
                rangeIndex);
        }

        ParameterCategory getDescriptorSetDescriptorRangeCategory(SlangInt setIndex, SlangInt rangeIndex)
        {
            return (ParameterCategory) spReflectionTypeLayout_getDescriptorSetDescriptorRangeCategory(
                (SlangReflectionTypeLayout*) this,
                setIndex,
                rangeIndex);
        }

        SlangInt getSubObjectRangeCount()
        {
            return spReflectionTypeLayout_getSubObjectRangeCount(
                (SlangReflectionTypeLayout*) this);
        }

        SlangInt getSubObjectRangeBindingRangeIndex(SlangInt subObjectRangeIndex)
        {
            return spReflectionTypeLayout_getSubObjectRangeBindingRangeIndex(
                (SlangReflectionTypeLayout*) this,
                subObjectRangeIndex);
        }

        SlangInt getSubObjectRangeSpaceOffset(SlangInt subObjectRangeIndex)
        {
            return spReflectionTypeLayout_getSubObjectRangeSpaceOffset(
                (SlangReflectionTypeLayout*) this,
                subObjectRangeIndex);
        }

        VariableLayoutReflection* getSubObjectRangeOffset(SlangInt subObjectRangeIndex)
        {
            return (VariableLayoutReflection*) spReflectionTypeLayout_getSubObjectRangeOffset(
                (SlangReflectionTypeLayout*) this,
                subObjectRangeIndex);
        }
    };

    struct Modifier
    {
        enum ID : SlangModifierID
        {
            Shared = SLANG_MODIFIER_SHARED,
        };
    };

    struct VariableReflection
    {
        char const* getName()
        {
            return spReflectionVariable_GetName((SlangReflectionVariable*) this);
        }

        TypeReflection* getType()
        {
            return (TypeReflection*) spReflectionVariable_GetType((SlangReflectionVariable*) this);
        }

        Modifier* findModifier(Modifier::ID id)
        {
            return (Modifier*) spReflectionVariable_FindModifier((SlangReflectionVariable*) this, (SlangModifierID) id);
        }

        unsigned int getUserAttributeCount()
        {
            return spReflectionVariable_GetUserAttributeCount((SlangReflectionVariable*)this);
        }
        UserAttribute* getUserAttributeByIndex(unsigned int index)
        {
            return (UserAttribute*)spReflectionVariable_GetUserAttribute((SlangReflectionVariable*)this, index);
        }
        UserAttribute* findUserAttributeByName(SlangSession* session, char const* name)
        {
            return (UserAttribute*)spReflectionVariable_FindUserAttributeByName((SlangReflectionVariable*)this, session, name);
        }
    };

    struct VariableLayoutReflection
    {
        VariableReflection* getVariable()
        {
            return (VariableReflection*) spReflectionVariableLayout_GetVariable((SlangReflectionVariableLayout*) this);
        }

        char const* getName()
        {
            return getVariable()->getName();
        }

        Modifier* findModifier(Modifier::ID id)
        {
            return getVariable()->findModifier(id);
        }

        TypeLayoutReflection* getTypeLayout()
        {
            return (TypeLayoutReflection*) spReflectionVariableLayout_GetTypeLayout((SlangReflectionVariableLayout*) this);
        }

        ParameterCategory getCategory()
        {
            return getTypeLayout()->getParameterCategory();
        }

        unsigned int getCategoryCount()
        {
            return getTypeLayout()->getCategoryCount();
        }

        ParameterCategory getCategoryByIndex(unsigned int index)
        {
            return getTypeLayout()->getCategoryByIndex(index);
        }


        size_t getOffset(SlangParameterCategory category = SLANG_PARAMETER_CATEGORY_UNIFORM)
        {
            return spReflectionVariableLayout_GetOffset((SlangReflectionVariableLayout*) this, category);
        }

        TypeReflection* getType()
        {
            return getVariable()->getType();
        }

        unsigned getBindingIndex()
        {
            return spReflectionParameter_GetBindingIndex((SlangReflectionVariableLayout*) this);
        }

        unsigned getBindingSpace()
        {
            return spReflectionParameter_GetBindingSpace((SlangReflectionVariableLayout*) this);
        }

        size_t getBindingSpace(SlangParameterCategory category)
        {
            return spReflectionVariableLayout_GetSpace((SlangReflectionVariableLayout*) this, category);
        }

        char const* getSemanticName()
        {
            return spReflectionVariableLayout_GetSemanticName((SlangReflectionVariableLayout*) this);
        }

        size_t getSemanticIndex()
        {
            return spReflectionVariableLayout_GetSemanticIndex((SlangReflectionVariableLayout*) this);
        }

        SlangStage getStage()
        {
            return spReflectionVariableLayout_getStage((SlangReflectionVariableLayout*) this);
        }

        VariableLayoutReflection* getPendingDataLayout()
        {
            return (VariableLayoutReflection*) spReflectionVariableLayout_getPendingDataLayout((SlangReflectionVariableLayout*) this);
        }
    };

    struct EntryPointReflection
    {
        char const* getName()
        {
            return spReflectionEntryPoint_getName((SlangReflectionEntryPoint*) this);
        }

        char const* getNameOverride()
        {
            return spReflectionEntryPoint_getNameOverride((SlangReflectionEntryPoint*)this);
        }

        unsigned getParameterCount()
        {
            return spReflectionEntryPoint_getParameterCount((SlangReflectionEntryPoint*) this);
        }

        VariableLayoutReflection* getParameterByIndex(unsigned index)
        {
            return (VariableLayoutReflection*) spReflectionEntryPoint_getParameterByIndex((SlangReflectionEntryPoint*) this, index);
        }

        SlangStage getStage()
        {
            return spReflectionEntryPoint_getStage((SlangReflectionEntryPoint*) this);
        }

        void getComputeThreadGroupSize(
            SlangUInt   axisCount,
            SlangUInt*  outSizeAlongAxis)
        {
            return spReflectionEntryPoint_getComputeThreadGroupSize((SlangReflectionEntryPoint*) this, axisCount, outSizeAlongAxis);
        }

        bool usesAnySampleRateInput()
        {
            return 0 != spReflectionEntryPoint_usesAnySampleRateInput((SlangReflectionEntryPoint*) this);
        }

        VariableLayoutReflection* getVarLayout()
        {
            return (VariableLayoutReflection*) spReflectionEntryPoint_getVarLayout((SlangReflectionEntryPoint*) this);
        }

        TypeLayoutReflection* getTypeLayout()
        {
            return getVarLayout()->getTypeLayout();
        }

        VariableLayoutReflection* getResultVarLayout()
        {
            return (VariableLayoutReflection*) spReflectionEntryPoint_getResultVarLayout((SlangReflectionEntryPoint*) this);
        }

        bool hasDefaultConstantBuffer()
        {
            return spReflectionEntryPoint_hasDefaultConstantBuffer((SlangReflectionEntryPoint*) this) != 0;
        }
    };
    typedef EntryPointReflection EntryPointLayout;

    struct TypeParameterReflection
    {
        char const* getName()
        {
            return spReflectionTypeParameter_GetName((SlangReflectionTypeParameter*) this);
        }
        unsigned getIndex()
        {
            return spReflectionTypeParameter_GetIndex((SlangReflectionTypeParameter*) this);
        }
        unsigned getConstraintCount()
        {
            return spReflectionTypeParameter_GetConstraintCount((SlangReflectionTypeParameter*) this);
        }
        TypeReflection* getConstraintByIndex(int index)
        {
            return (TypeReflection*)spReflectionTypeParameter_GetConstraintByIndex((SlangReflectionTypeParameter*) this, index);
        }
    };

    enum class LayoutRules : SlangLayoutRules
    {
        Default = SLANG_LAYOUT_RULES_DEFAULT,
    };

    typedef struct ShaderReflection ProgramLayout;

    struct ShaderReflection
    {
        unsigned getParameterCount()
        {
            return spReflection_GetParameterCount((SlangReflection*) this);
        }

        unsigned getTypeParameterCount()
        {
            return spReflection_GetTypeParameterCount((SlangReflection*) this);
        }

        TypeParameterReflection* getTypeParameterByIndex(unsigned index)
        {
            return (TypeParameterReflection*)spReflection_GetTypeParameterByIndex((SlangReflection*) this, index);
        }

        TypeParameterReflection* findTypeParameter(char const* name)
        {
            return (TypeParameterReflection*)spReflection_FindTypeParameter((SlangReflection*)this, name);
        }

        VariableLayoutReflection* getParameterByIndex(unsigned index)
        {
            return (VariableLayoutReflection*) spReflection_GetParameterByIndex((SlangReflection*) this, index);
        }

        static ProgramLayout* get(SlangCompileRequest* request)
        {
            return (ProgramLayout*) spGetReflection(request);
        }

        SlangUInt getEntryPointCount()
        {
            return spReflection_getEntryPointCount((SlangReflection*) this);
        }

        EntryPointReflection* getEntryPointByIndex(SlangUInt index)
        {
            return (EntryPointReflection*) spReflection_getEntryPointByIndex((SlangReflection*) this, index);
        }

        SlangUInt getGlobalConstantBufferBinding()
        {
            return spReflection_getGlobalConstantBufferBinding((SlangReflection*)this);
        }

        size_t getGlobalConstantBufferSize()
        {
            return spReflection_getGlobalConstantBufferSize((SlangReflection*)this);
        }

        TypeReflection* findTypeByName(const char* name)
        {
            return (TypeReflection*)spReflection_FindTypeByName(
                (SlangReflection*) this,
                name);
        }

        TypeLayoutReflection* getTypeLayout(
            TypeReflection* type,
            LayoutRules     rules = LayoutRules::Default)
        {
            return (TypeLayoutReflection*)spReflection_GetTypeLayout(
                (SlangReflection*) this,
                (SlangReflectionType*)type,
                SlangLayoutRules(rules));
        }

        EntryPointReflection* findEntryPointByName(const char* name)
        {
            return (EntryPointReflection*)spReflection_findEntryPointByName(
                (SlangReflection*) this,
                name);
        }

        TypeReflection* specializeType(
            TypeReflection*         type,
            SlangInt                specializationArgCount,
            TypeReflection* const*  specializationArgs,
            ISlangBlob**            outDiagnostics)
        {
            return (TypeReflection*) spReflection_specializeType(
                (SlangReflection*) this,
                (SlangReflectionType*) type,
                specializationArgCount,
                (SlangReflectionType* const*) specializationArgs,
                outDiagnostics);
        }

        SlangUInt getHashedStringCount() const { return spReflection_getHashedStringCount((SlangReflection*)this); }

        const char* getHashedString(SlangUInt index, size_t* outCount) const
        {
            return spReflection_getHashedString((SlangReflection*)this, index, outCount);
        }

        TypeLayoutReflection* getGlobalParamsTypeLayout()
        {
            return (TypeLayoutReflection*) spReflection_getGlobalParamsTypeLayout((SlangReflection*) this);
        }

        VariableLayoutReflection* getGlobalParamsVarLayout()
        {
            return (VariableLayoutReflection*) spReflection_getGlobalParamsVarLayout((SlangReflection*) this);
        }
    };

    typedef uint32_t CompileStdLibFlags;
    struct CompileStdLibFlag
    {
        enum Enum : CompileStdLibFlags
        {
            WriteDocumentation = 0x1,
        };
    };

    typedef ISlangBlob IBlob;

    struct IComponentType;
    struct ITypeConformance;
    struct IGlobalSession;
    struct IModule;
    struct ISession;

    struct SessionDesc;
    struct SpecializationArg;
    struct TargetDesc;

        /** A global session for interaction with the Slang library.

        An application may create and re-use a single global session across
        multiple sessions, in order to amortize startups costs (in current
        Slang this is mostly the cost of loading the Slang standard library).

        The global session is currently *not* thread-safe and objects created from
        a single global session should only be used from a single thread at
        a time.
        */
    struct IGlobalSession : public ISlangUnknown
    {
        SLANG_COM_INTERFACE(0xc140b5fd, 0xc78, 0x452e, { 0xba, 0x7c, 0x1a, 0x1e, 0x70, 0xc7, 0xf7, 0x1c })

            /** Create a new session for loading and compiling code.
            */
        virtual SLANG_NO_THROW SlangResult SLANG_MCALL createSession(
            SessionDesc const&  desc,
            ISession**          outSession) = 0;

            /** Look up the internal ID of a profile by its `name`.

            Profile IDs are *not* guaranteed to be stable across versions
            of the Slang library, so clients are expected to look up
            profiles by name at runtime.
            */
        virtual SLANG_NO_THROW SlangProfileID SLANG_MCALL findProfile(
            char const*     name) = 0;

            /** Set the path that downstream compilers (aka back end compilers) will
            be looked from.
            @param passThrough Identifies the downstream compiler
            @param path The path to find the downstream compiler (shared library/dll/executable)

            For back ends that are dlls/shared libraries, it will mean the path will
            be prefixed with the path when calls are made out to ISlangSharedLibraryLoader.
            For executables - it will look for executables along the path */
        virtual SLANG_NO_THROW void SLANG_MCALL setDownstreamCompilerPath(
            SlangPassThrough passThrough,
            char const* path) = 0;

            /** DEPRECIATED: Use setLanguagePrelude

            Set the 'prelude' for generated code for a 'downstream compiler'.
            @param passThrough The downstream compiler for generated code that will have the prelude applied to it. 
            @param preludeText The text added pre-pended verbatim before the generated source

            That for pass-through usage, prelude is not pre-pended, preludes are for code generation only. 
            */
        virtual SLANG_NO_THROW void SLANG_MCALL setDownstreamCompilerPrelude(
            SlangPassThrough passThrough,
            const char* preludeText) = 0;

            /** DEPRECIATED: Use getLanguagePrelude

            Get the 'prelude' for generated code for a 'downstream compiler'.
            @param passThrough The downstream compiler for generated code that will have the prelude applied to it. 
            @param outPrelude  On exit holds a blob that holds the string of the prelude.
            */
        virtual SLANG_NO_THROW void SLANG_MCALL getDownstreamCompilerPrelude(
            SlangPassThrough passThrough,
            ISlangBlob** outPrelude) = 0;

            /** Get the build version 'tag' string. The string is the same as produced via `git describe --tags`
            for the project. If Slang is built separately from the automated build scripts
            the contents will by default be 'unknown'. Any string can be set by changing the
            contents of 'slang-tag-version.h' file and recompiling the project.

            This method will return exactly the same result as the free function spGetBuildTagString.

            @return The build tag string
            */
        virtual SLANG_NO_THROW const char* SLANG_MCALL getBuildTagString() = 0;

            /* For a given source language set the default compiler.
            If a default cannot be chosen (for example the target cannot be achieved by the default),
            the default will not be used. 

            @param sourceLanguage the source language 
            @param defaultCompiler the default compiler for that language
            @return 
            */
        virtual SLANG_NO_THROW SlangResult SLANG_MCALL setDefaultDownstreamCompiler(
            SlangSourceLanguage sourceLanguage,
            SlangPassThrough defaultCompiler) = 0;

            /* For a source type get the default compiler 

            @param sourceLanguage the source language 
            @return The downstream compiler for that source language */
        virtual SlangPassThrough SLANG_MCALL getDefaultDownstreamCompiler(
            SlangSourceLanguage sourceLanguage) = 0;

            /* Set the 'prelude' placed before generated code for a specific language type.
            
            @param sourceLanguage The language the prelude should be inserted on.
            @param preludeText The text added pre-pended verbatim before the generated source

            Note! That for pass-through usage, prelude is not pre-pended, preludes are for code generation only. 
            */
        virtual SLANG_NO_THROW void SLANG_MCALL setLanguagePrelude(
            SlangSourceLanguage sourceLanguage,
            const char* preludeText) = 0;

            /** Get the 'prelude' associated with a specific source language. 
            @param sourceLanguage The language the prelude should be inserted on.
            @param outPrelude  On exit holds a blob that holds the string of the prelude.
            */
        virtual SLANG_NO_THROW void SLANG_MCALL getLanguagePrelude(
            SlangSourceLanguage sourceLanguage,
            ISlangBlob** outPrelude) = 0;

            /** Create a compile request.
            */
        virtual SLANG_NO_THROW SlangResult SLANG_MCALL createCompileRequest(
            slang::ICompileRequest** outCompileRequest) = 0;

            /** Add new builtin declarations to be used in subsequent compiles.
            */
        virtual SLANG_NO_THROW void SLANG_MCALL addBuiltins(
            char const*     sourcePath,
            char const*     sourceString) = 0;

            /** Set the session shared library loader. If this changes the loader, it may cause shared libraries to be unloaded
            @param loader The loader to set. Setting nullptr sets the default loader. 
            */
        virtual SLANG_NO_THROW void SLANG_MCALL setSharedLibraryLoader(
            ISlangSharedLibraryLoader* loader) = 0;

            /** Gets the currently set shared library loader
            @return Gets the currently set loader. If returns nullptr, it's the default loader
            */
        virtual SLANG_NO_THROW ISlangSharedLibraryLoader* SLANG_MCALL getSharedLibraryLoader() = 0;

            /** Returns SLANG_OK if a the compilation target is supported for this session
            
            @param target The compilation target to test
            @return SLANG_OK if the target is available
            SLANG_E_NOT_IMPLEMENTED if not implemented in this build
            SLANG_E_NOT_FOUND if other resources (such as shared libraries) required to make target work could not be found
            SLANG_FAIL other kinds of failures */
        virtual SLANG_NO_THROW SlangResult SLANG_MCALL checkCompileTargetSupport(
            SlangCompileTarget  target) = 0;

            /** Returns SLANG_OK if a the pass through support is supported for this session
            @param session Session
            @param target The compilation target to test
            @return SLANG_OK if the target is available
            SLANG_E_NOT_IMPLEMENTED if not implemented in this build
            SLANG_E_NOT_FOUND if other resources (such as shared libraries) required to make target work could not be found
            SLANG_FAIL other kinds of failures */
        virtual SLANG_NO_THROW SlangResult SLANG_MCALL checkPassThroughSupport(
            SlangPassThrough    passThrough) = 0;

            /** Compile from (embedded source) the StdLib on the session.
            Will return a failure if there is already a StdLib available
            NOTE! API is experimental and not ready for production code
            @param flags to control compilation
            */
        virtual SLANG_NO_THROW SlangResult SLANG_MCALL compileStdLib(CompileStdLibFlags flags) = 0;

            /** Load the StdLib. Currently loads modules from the file system. 
            @param stdLib Start address of the serialized stdlib
            @param stdLibSizeInBytes The size in bytes of the serialized stdlib

            NOTE! API is experimental and not ready for production code
            */
        virtual SLANG_NO_THROW SlangResult SLANG_MCALL loadStdLib(const void* stdLib, size_t stdLibSizeInBytes) = 0;

            /** Save the StdLib modules to the file system
            @param archiveType The type of archive used to hold the stdlib
            @param outBlob The serialized blob containing the standard library

            NOTE! API is experimental and not ready for production code  */
        virtual SLANG_NO_THROW SlangResult SLANG_MCALL saveStdLib(SlangArchiveType archiveType, ISlangBlob** outBlob) = 0;

            /** Look up the internal ID of a capability by its `name`.

            Capability IDs are *not* guaranteed to be stable across versions
            of the Slang library, so clients are expected to look up
            capabilities by name at runtime.
            */
        virtual SLANG_NO_THROW SlangCapabilityID SLANG_MCALL findCapability(
            char const*     name) = 0;

            /** Set the downstream/pass through compiler to be used for a transition from the source type to the target type
            @param source The source 'code gen target'
            @param target The target 'code gen target'
            @param compiler The compiler/pass through to use for the transition from source to target
            */
        virtual SLANG_NO_THROW void SLANG_MCALL setDownstreamCompilerForTransition(SlangCompileTarget source, SlangCompileTarget target, SlangPassThrough compiler) = 0;

            /** Get the downstream/pass through compiler for a transition specified by source and target
            @param source The source 'code gen target'
            @param target The target 'code gen target'
            @return The compiler that is used for the transition. Returns SLANG_PASS_THROUGH_NONE it is not defined
            */
        virtual SLANG_NO_THROW SlangPassThrough SLANG_MCALL getDownstreamCompilerForTransition(SlangCompileTarget source, SlangCompileTarget target) = 0;

            /** Get the time in seconds spent in the downstream compiler.
            @return The time spent in the downstream compiler in the current global session.
            */
        virtual SLANG_NO_THROW double SLANG_MCALL getDownstreamCompilerElapsedTime() = 0;
    };

    #define SLANG_UUID_IGlobalSession IGlobalSession::getTypeGuid()

    /*!
    @brief A request for one or more compilation actions to be performed.
    */
    struct ICompileRequest : public ISlangUnknown
    {
        SLANG_COM_INTERFACE( 0x96d33993, 0x317c, 0x4db5, { 0xaf, 0xd8, 0x66, 0x6e, 0xe7, 0x72, 0x48, 0xe2 } )
   
            /** Set the filesystem hook to use for a compile request

            The provided `fileSystem` will be used to load any files that
            need to be loaded during processing of the compile `request`.
            This includes:

              - Source files loaded via `spAddTranslationUnitSourceFile`
              - Files referenced via `#include`
              - Files loaded to resolve `#import` operations
                */
        virtual SLANG_NO_THROW void SLANG_MCALL setFileSystem(
            ISlangFileSystem*       fileSystem) = 0;

            /*!
            @brief Set flags to be used for compilation.
            */
        virtual SLANG_NO_THROW void SLANG_MCALL setCompileFlags(
            SlangCompileFlags       flags) = 0;

            /*!
            @brief Set whether to dump intermediate results (for debugging) or not.
            */
        virtual SLANG_NO_THROW void SLANG_MCALL setDumpIntermediates(
            int                     enable) = 0;

        virtual SLANG_NO_THROW void SLANG_MCALL setDumpIntermediatePrefix(
            const char* prefix) = 0;

            /*!
            @brief Set whether (and how) `#line` directives should be output.
            */
        virtual SLANG_NO_THROW void SLANG_MCALL setLineDirectiveMode(
            SlangLineDirectiveMode  mode) = 0;

            /*!
            @brief Sets the target for code generation.
            @param target The code generation target. Possible values are:
            - SLANG_GLSL. Generates GLSL code.
            - SLANG_HLSL. Generates HLSL code.
            - SLANG_SPIRV. Generates SPIR-V code.
            */
        virtual SLANG_NO_THROW void SLANG_MCALL setCodeGenTarget(
            SlangCompileTarget target) = 0;

            /*!
            @brief Add a code-generation target to be used.
            */
        virtual SLANG_NO_THROW int SLANG_MCALL addCodeGenTarget(
            SlangCompileTarget      target) = 0;

        virtual SLANG_NO_THROW void SLANG_MCALL setTargetProfile(
            int                     targetIndex,
            SlangProfileID          profile) = 0;

        virtual SLANG_NO_THROW void SLANG_MCALL setTargetFlags(
            int                     targetIndex,
            SlangTargetFlags        flags) = 0;


            /*!
            @brief Set the floating point mode (e.g., precise or fast) to use a target.
            */
        virtual SLANG_NO_THROW void SLANG_MCALL setTargetFloatingPointMode(
            int                     targetIndex,
            SlangFloatingPointMode  mode) = 0;

            /* DEPRECATED: use `spSetMatrixLayoutMode` instead. */
        virtual SLANG_NO_THROW void SLANG_MCALL setTargetMatrixLayoutMode(
            int                     targetIndex,
            SlangMatrixLayoutMode   mode) = 0;

        virtual SLANG_NO_THROW void SLANG_MCALL setMatrixLayoutMode(
            SlangMatrixLayoutMode   mode) = 0;

            /*!
            @brief Set the level of debug information to produce.
            */
        virtual SLANG_NO_THROW void SLANG_MCALL setDebugInfoLevel(
            SlangDebugInfoLevel     level) = 0;

            /*!
            @brief Set the level of optimization to perform.
            */
        virtual SLANG_NO_THROW void SLANG_MCALL setOptimizationLevel(
            SlangOptimizationLevel  level) = 0;


    
            /*!
            @brief Set the container format to be used for binary output.
            */
        virtual SLANG_NO_THROW void SLANG_MCALL setOutputContainerFormat(
            SlangContainerFormat    format) = 0;

        virtual SLANG_NO_THROW void SLANG_MCALL setPassThrough(
            SlangPassThrough        passThrough) = 0;

    
        virtual SLANG_NO_THROW void SLANG_MCALL setDiagnosticCallback(
            SlangDiagnosticCallback callback,
            void const*             userData) = 0;

        virtual SLANG_NO_THROW void SLANG_MCALL setWriter(
            SlangWriterChannel      channel, 
            ISlangWriter*           writer) = 0;

        virtual SLANG_NO_THROW ISlangWriter* SLANG_MCALL getWriter(
            SlangWriterChannel      channel) = 0;

            /*!
            @brief Add a path to use when searching for referenced files.
            This will be used for both `#include` directives and also for explicit `__import` declarations.
            @param ctx The compilation context.
            @param searchDir The additional search directory.
            */
        virtual SLANG_NO_THROW void SLANG_MCALL addSearchPath(
            const char*             searchDir) = 0;

            /*!
            @brief Add a macro definition to be used during preprocessing.
            @param key The name of the macro to define.
            @param value The value of the macro to define.
            */
        virtual SLANG_NO_THROW void SLANG_MCALL addPreprocessorDefine(
            const char*             key,
            const char*             value) = 0;

            /*!
            @brief Set options using arguments as if specified via command line.
            @return Returns SlangResult. On success SLANG_SUCCEEDED(result) is true.
            */
        virtual SLANG_NO_THROW SlangResult SLANG_MCALL processCommandLineArguments(
            char const* const*      args,
            int                     argCount) = 0;

            /** Add a distinct translation unit to the compilation request

            `name` is optional. 
            Returns the zero-based index of the translation unit created.
            */
        virtual SLANG_NO_THROW int SLANG_MCALL addTranslationUnit(
            SlangSourceLanguage     language,
            char const*             name) = 0;

    
            /** Set a default module name. Translation units will default to this module name if one is not
            passed. If not set each translation unit will get a unique name. 
            */
        virtual SLANG_NO_THROW void SLANG_MCALL setDefaultModuleName(
            const char* defaultModuleName) = 0;

            /** Add a preprocessor definition that is scoped to a single translation unit.

            @param translationUnitIndex The index of the translation unit to get the definition.
            @param key The name of the macro to define.
            @param value The value of the macro to define.
            */
        virtual SLANG_NO_THROW void SLANG_MCALL addTranslationUnitPreprocessorDefine(
            int                     translationUnitIndex,
            const char*             key,
            const char*             value) = 0;


            /** Add a source file to the given translation unit.

            If a user-defined file system has been specified via
            `spSetFileSystem`, then it will be used to load the
            file at `path`. Otherwise, Slang will use the OS
            file system.

            This function does *not* search for a file using
            the registered search paths (`spAddSearchPath`),
            and instead using the given `path` as-is.
            */
        virtual SLANG_NO_THROW void SLANG_MCALL addTranslationUnitSourceFile(
            int                     translationUnitIndex,
            char const*             path) = 0;

            /** Add a source string to the given translation unit.

            @param translationUnitIndex The index of the translation unit to add source to.
            @param path The file-system path that should be assumed for the source code.
            @param source A null-terminated UTF-8 encoded string of source code.

            The implementation will make a copy of the source code data.
            An application may free the buffer immediately after this call returns.

            The `path` will be used in any diagnostic output, as well
            as to determine the base path when resolving relative
            `#include`s.
            */
        virtual SLANG_NO_THROW void SLANG_MCALL addTranslationUnitSourceString(
            int                     translationUnitIndex,
            char const*             path,
            char const*             source) = 0;


            /** Add a slang library - such that its contents can be referenced during linking.
            This is equivalent to the -r command line option.

            @param libData The library data
            @param libDataSize The size of the library data
            */
        virtual SLANG_NO_THROW SlangResult SLANG_MCALL addLibraryReference(
            const void* libData,
            size_t libDataSize) = 0;

            /** Add a source string to the given translation unit.

            @param translationUnitIndex The index of the translation unit to add source to.
            @param path The file-system path that should be assumed for the source code.
            @param sourceBegin A pointer to a buffer of UTF-8 encoded source code.
            @param sourceEnd A pointer to to the end of the buffer specified in `sourceBegin`

            The implementation will make a copy of the source code data.
            An application may free the buffer immediately after this call returns.

            The `path` will be used in any diagnostic output, as well
            as to determine the base path when resolving relative
            `#include`s.
            */
        virtual SLANG_NO_THROW void SLANG_MCALL addTranslationUnitSourceStringSpan(
            int                     translationUnitIndex,
            char const*             path,
            char const*             sourceBegin,
            char const*             sourceEnd) = 0;

            /** Add a blob of source code to the given translation unit.

            @param translationUnitIndex The index of the translation unit to add source to.
            @param path The file-system path that should be assumed for the source code.
            @param sourceBlob A blob containing UTF-8 encoded source code.
            @param sourceEnd A pointer to to the end of the buffer specified in `sourceBegin`

            The compile request will retain a reference to the blob.

            The `path` will be used in any diagnostic output, as well
            as to determine the base path when resolving relative
            `#include`s.
            */
        virtual SLANG_NO_THROW void SLANG_MCALL addTranslationUnitSourceBlob(
            int                     translationUnitIndex,
            char const*             path,
            ISlangBlob*             sourceBlob) = 0;

            /** Add an entry point in a particular translation unit
            */
        virtual SLANG_NO_THROW int SLANG_MCALL addEntryPoint(
            int                     translationUnitIndex,
            char const*             name,
            SlangStage              stage) = 0;

            /** Add an entry point in a particular translation unit,
                with additional arguments that specify the concrete
                type names for entry-point generic type parameters.
            */
        virtual SLANG_NO_THROW int SLANG_MCALL addEntryPointEx(
            int                     translationUnitIndex,
            char const*             name,
            SlangStage              stage,
            int                     genericArgCount,
            char const**            genericArgs) = 0;

            /** Specify the arguments to use for global generic parameters.
            */
        virtual SLANG_NO_THROW SlangResult SLANG_MCALL setGlobalGenericArgs(
            int                     genericArgCount,
            char const**            genericArgs) = 0;

            /** Specify the concrete type to be used for a global "existential slot."

            Every shader parameter (or leaf field of a `struct`-type shader parameter)
            that has an interface or array-of-interface type introduces an existential
            slot. The number of slots consumed by a shader parameter, and the starting
            slot of each parameter can be queried via the reflection API using
            `SLANG_PARAMETER_CATEGORY_EXISTENTIAL_TYPE_PARAM`.

            In order to generate specialized code, a concrete type needs to be specified
            for each existential slot. This function specifies the name of the type
            (or in general a type *expression*) to use for a specific slot at the
            global scope.
            */
        virtual SLANG_NO_THROW SlangResult SLANG_MCALL setTypeNameForGlobalExistentialTypeParam(
            int                     slotIndex,
            char const*             typeName) = 0;

            /** Specify the concrete type to be used for an entry-point "existential slot."

            Every shader parameter (or leaf field of a `struct`-type shader parameter)
            that has an interface or array-of-interface type introduces an existential
            slot. The number of slots consumed by a shader parameter, and the starting
            slot of each parameter can be queried via the reflection API using
            `SLANG_PARAMETER_CATEGORY_EXISTENTIAL_TYPE_PARAM`.

            In order to generate specialized code, a concrete type needs to be specified
            for each existential slot. This function specifies the name of the type
            (or in general a type *expression*) to use for a specific slot at the
            entry-point scope.
            */
        virtual SLANG_NO_THROW SlangResult SLANG_MCALL setTypeNameForEntryPointExistentialTypeParam(
            int                     entryPointIndex,
            int                     slotIndex,
            char const*             typeName) = 0;

            /** Execute the compilation request.

            @returns  SlangResult, SLANG_OK on success. Use SLANG_SUCCEEDED() and SLANG_FAILED() to test SlangResult.
            */
        virtual SLANG_NO_THROW SlangResult SLANG_MCALL compile() = 0;


            /** Get any diagnostic messages reported by the compiler.

            @returns A null-terminated UTF-8 encoded string of diagnostic messages.

            The returned pointer is only guaranteed to be valid
            until `request` is destroyed. Applications that wish to
            hold on to the diagnostic output for longer should use
            `getDiagnosticOutputBlob`.
            */
        virtual SLANG_NO_THROW char const* SLANG_MCALL getDiagnosticOutput() = 0;

            /** Get diagnostic messages reported by the compiler.

            @param outBlob A pointer to receive a blob holding a nul-terminated UTF-8 encoded string of diagnostic messages.
            @returns A `SlangResult` indicating success or failure.
            */
        virtual SLANG_NO_THROW SlangResult SLANG_MCALL getDiagnosticOutputBlob(
            ISlangBlob**            outBlob) = 0;


            /** Get the number of files that this compilation depended on.

            This includes both the explicit source files, as well as any
            additional files that were transitively referenced (e.g., via
            a `#include` directive).
            */
        virtual SLANG_NO_THROW int SLANG_MCALL getDependencyFileCount() = 0;

            /** Get the path to a file this compilation depended on.
            */
        virtual SLANG_NO_THROW char const* SLANG_MCALL getDependencyFilePath(
            int                     index) = 0;

            /** Get the number of translation units associated with the compilation request
            */
        virtual SLANG_NO_THROW int SLANG_MCALL getTranslationUnitCount() = 0;

            /** Get the output source code associated with a specific entry point.

            The lifetime of the output pointer is the same as `request`.
            */
        virtual SLANG_NO_THROW char const* SLANG_MCALL getEntryPointSource(
            int                     entryPointIndex) = 0;

            /** Get the output bytecode associated with a specific entry point.

            The lifetime of the output pointer is the same as `request`.
            */
        virtual SLANG_NO_THROW void const* SLANG_MCALL getEntryPointCode(
            int                     entryPointIndex,
            size_t*                 outSize) = 0;

            /** Get the output code associated with a specific entry point.

            @param entryPointIndex The index of the entry point to get code for.
            @param targetIndex The index of the target to get code for (default: zero).
            @param outBlob A pointer that will receive the blob of code
            @returns A `SlangResult` to indicate success or failure.
            */
        virtual SLANG_NO_THROW SlangResult SLANG_MCALL getEntryPointCodeBlob(
            int                     entryPointIndex,
            int                     targetIndex,
            ISlangBlob**            outBlob) = 0;

            /** Get entry point 'callable' functions accessible through the ISlangSharedLibrary interface.

            That the functions remain in scope as long as the ISlangSharedLibrary interface is in scope.

            NOTE! Requires a compilation target of SLANG_HOST_CALLABLE.
    
            @param entryPointIndex  The index of the entry point to get code for.
            @param targetIndex      The index of the target to get code for (default: zero).
            @param outSharedLibrary A pointer to a ISharedLibrary interface which functions can be queried on.
            @returns                A `SlangResult` to indicate success or failure.
            */
        virtual SLANG_NO_THROW SlangResult SLANG_MCALL getEntryPointHostCallable(
            int                     entryPointIndex,
            int                     targetIndex,
            ISlangSharedLibrary**   outSharedLibrary) = 0;

            /** Get the output code associated with a specific target.

            @param targetIndex The index of the target to get code for (default: zero).
            @param outBlob A pointer that will receive the blob of code
            @returns A `SlangResult` to indicate success or failure.
            */
        virtual SLANG_NO_THROW SlangResult SLANG_MCALL getTargetCodeBlob(
            int                     targetIndex,
            ISlangBlob**            outBlob) = 0;

            /** Get 'callable' functions for a target accessible through the ISlangSharedLibrary interface.

            That the functions remain in scope as long as the ISlangSharedLibrary interface is in scope.

            NOTE! Requires a compilation target of SLANG_HOST_CALLABLE.
    
            @param targetIndex      The index of the target to get code for (default: zero).
            @param outSharedLibrary A pointer to a ISharedLibrary interface which functions can be queried on.
            @returns                A `SlangResult` to indicate success or failure.
            */
        virtual SLANG_NO_THROW SlangResult SLANG_MCALL getTargetHostCallable(
            int                     targetIndex,
            ISlangSharedLibrary**   outSharedLibrary) = 0;

            /** Get the output bytecode associated with an entire compile request.

            The lifetime of the output pointer is the same as `request` and the last spCompile.

            @param outSize          The size of the containers contents in bytes. Will be zero if there is no code available.
            @returns                Pointer to start of the contained data, or nullptr if there is no code available.
            */
        virtual SLANG_NO_THROW void const* SLANG_MCALL getCompileRequestCode(
            size_t*                 outSize) = 0;

            /** Return the container code as a blob. The container blob is created as part of a compilation (with spCompile),
            and a container is produced with a suitable ContainerFormat. 

            @param outSize          The blob containing the container data. 
            @returns                A `SlangResult` to indicate success or failure.
            */
        virtual SLANG_NO_THROW SlangResult SLANG_MCALL getContainerCode(
            ISlangBlob**            outBlob) = 0;

            /** Load repro from memory specified.

            Should only be performed on a newly created request.

            NOTE! When using the fileSystem, files will be loaded via their `unique names` as if they are part of the flat file system. This
            mechanism is described more fully in docs/repro.md.

            @param fileSystem       An (optional) filesystem. Pass nullptr to just use contents of repro held in data.
            @param data             The data to load from.
            @param size             The size of the data to load from. 
            @returns                A `SlangResult` to indicate success or failure.
            */
        virtual SLANG_NO_THROW SlangResult SLANG_MCALL loadRepro(
            ISlangFileSystem* fileSystem,
            const void* data,
            size_t size) = 0;

            /** Save repro state. Should *typically* be performed after spCompile, so that everything
            that is needed for a compilation is available. 

            @param outBlob          Blob that will hold the serialized state
            @returns                A `SlangResult` to indicate success or failure.
            */
        virtual SLANG_NO_THROW SlangResult SLANG_MCALL saveRepro(
            ISlangBlob** outBlob) = 0;

            /** Enable repro capture.

            Should be set after any ISlangFileSystem has been set, but before any compilation. It ensures that everything
            that the ISlangFileSystem accesses will be correctly recorded.
            Note that if a ISlangFileSystem/ISlangFileSystemExt isn't explicitly set (ie the default is used), then the
            request will automatically be set up to record everything appropriate. 

            @returns                A `SlangResult` to indicate success or failure.
            */
        virtual SLANG_NO_THROW SlangResult SLANG_MCALL enableReproCapture() = 0;

            /** Get the (linked) program for a compile request.

            The linked program will include all of the global-scope modules for the
            translation units in the program, plus any modules that they `import`
            (transitively), specialized to any global specialization arguments that
            were provided via the API.
            */
        virtual SLANG_NO_THROW SlangResult SLANG_MCALL getProgram(
            slang::IComponentType** outProgram) = 0;

            /** Get the (partially linked) component type for an entry point.

            The returned component type will include the entry point at the
            given index, and will be specialized using any specialization arguments
            that were provided for it via the API.

            The returned component will *not* include the modules representing
            the global scope and its dependencies/specialization, so a client
            program will typically want to compose this component type with
            the one returned by `spCompileRequest_getProgram` to get a complete
            and usable component type from which kernel code can be requested.
            */
        virtual SLANG_NO_THROW SlangResult SLANG_MCALL getEntryPoint(
            SlangInt                entryPointIndex,
            slang::IComponentType** outEntryPoint) = 0;

            /** Get the (un-linked) module for a translation unit.

            The returned module will not be linked against any dependencies,
            nor against any entry points (even entry points declared inside
            the module). Similarly, the module will not be specialized
            to the arguments that might have been provided via the API.

            This function provides an atomic unit of loaded code that
            is suitable for looking up types and entry points in the
            given module, and for linking together to produce a composite
            program that matches the needs of an application.
            */
        virtual SLANG_NO_THROW SlangResult SLANG_MCALL getModule(
            SlangInt                translationUnitIndex,
            slang::IModule**        outModule) = 0;

            /** Get the `ISession` handle behind the `SlangCompileRequest`.
            TODO(JS): Arguably this should just return the session pointer.
            */
        virtual SLANG_NO_THROW SlangResult SLANG_MCALL getSession(
            slang::ISession** outSession) = 0;

            /** get reflection data from a compilation request */
        virtual SLANG_NO_THROW SlangReflection* SLANG_MCALL getReflection() = 0;

            /** Make output specially handled for command line output */
        virtual SLANG_NO_THROW void SLANG_MCALL setCommandLineCompilerMode() = 0;

            /** Add a defined capability that should be assumed available on the target */
        virtual SLANG_NO_THROW SlangResult SLANG_MCALL addTargetCapability(
            SlangInt            targetIndex,
            SlangCapabilityID   capability) = 0;

            /** Get the (linked) program for a compile request, including all entry points.

            The resulting program will include all of the global-scope modules for the
            translation units in the program, plus any modules that they `import`
            (transitively), specialized to any global specialization arguments that
            were provided via the API, as well as all entry points specified for compilation,
            specialized to their entry-point specialization arguments.
            */
        virtual SLANG_NO_THROW SlangResult SLANG_MCALL getProgramWithEntryPoints(
            slang::IComponentType** outProgram) = 0;

            /** Set the line directive mode for a target.
            */
        virtual SLANG_NO_THROW void SLANG_MCALL setTargetLineDirectiveMode(
            SlangInt targetIndex,
            SlangLineDirectiveMode mode) = 0;

            /** Set whether to use scalar buffer layouts for GLSL/Vulkan targets.
                If true, the generated GLSL/Vulkan code will use `scalar` layout for storage buffers.
                If false, the resulting code will std430 for storage buffers.
            */
        virtual SLANG_NO_THROW void SLANG_MCALL setTargetForceGLSLScalarBufferLayout(int targetIndex, bool forceScalarLayout) = 0;
    };

    #define SLANG_UUID_ICompileRequest ICompileRequest::getTypeGuid()

        /** Description of a code generation target.
        */
    struct TargetDesc
    {
            /** The size of this structure, in bytes.
            */
        size_t structureSize = sizeof(TargetDesc);

            /** The target format to generate code for (e.g., SPIR-V, DXIL, etc.)
            */
        SlangCompileTarget      format = SLANG_TARGET_UNKNOWN;

            /** The compilation profile supported by the target (e.g., "Shader Model 5.1")
            */
        SlangProfileID          profile = SLANG_PROFILE_UNKNOWN;

            /** Flags for the code generation target. Currently unused. */
        SlangTargetFlags        flags = 0;

            /** Default mode to use for floating-point operations on the target.
            */
        SlangFloatingPointMode  floatingPointMode = SLANG_FLOATING_POINT_MODE_DEFAULT;

            /** Optimization level to use for the target.
            */
        SlangOptimizationLevel optimizationLevel = SLANG_OPTIMIZATION_LEVEL_DEFAULT;

            /** The line directive mode for output source code.
            */
        SlangLineDirectiveMode lineDirectiveMode = SLANG_LINE_DIRECTIVE_MODE_DEFAULT;

            /** Whether to force `scalar` layout for glsl shader storage buffers.
            */
        bool forceGLSLScalarBufferLayout = false;
    };

    typedef uint32_t SessionFlags;
    enum
    {
        kSessionFlags_None = 0,

        /** Use application-specific policy for semantics of the `shared` keyword.
        
        This is a legacy/compatibility flag to help an existing Slang client
        migrate to new language features, and should *not* be used by other
        clients. This feature may be removed in a future release without a
        deprecation warning, and this bit may be re-used for another feature.
        You have been warned.
        */
        kSessionFlag_FalcorCustomSharedKeywordSemantics = 1 << 0,
    };

    struct PreprocessorMacroDesc
    {
        const char* name;
        const char* value;
    };

    struct SessionDesc
    {
            /** The size of this structure, in bytes.
             */
        size_t structureSize = sizeof(SessionDesc);

            /** Code generation targets to include in the session.
            */
        TargetDesc const*   targets = nullptr;
        SlangInt            targetCount = 0;

            /** Flags to configure the session.
            */
        SessionFlags flags = kSessionFlags_None;

            /** Default layout to assume for variables with matrix types.
            */
        SlangMatrixLayoutMode defaultMatrixLayoutMode = SLANG_MATRIX_LAYOUT_ROW_MAJOR;

            /** Paths to use when searching for `#include`d or `import`ed files.
            */
        char const* const*  searchPaths = nullptr;
        SlangInt            searchPathCount = 0;

        PreprocessorMacroDesc const*    preprocessorMacros = nullptr;
        SlangInt                        preprocessorMacroCount = 0;
    };

    enum class ContainerType
    {
        None, UnsizedArray, StructuredBuffer, ConstantBuffer, ParameterBlock
    };

        /** A session provides a scope for code that is loaded.

        A session can be used to load modules of Slang source code,
        and to request target-specific compiled binaries and layout
        information.

        In order to be able to load code, the session owns a set
        of active "search paths" for resolving `#include` directives
        and `import` declrations, as well as a set of global
        preprocessor definitions that will be used for all code
        that gets `import`ed in the session.

        If multiple user shaders are loaded in the same session,
        and import the same module (e.g., two source files do `import X`)
        then there will only be one copy of `X` loaded within the session.

        In order to be able to generate target code, the session
        owns a list of available compilation targets, which specify
        code generation options.

        Code loaded and compiled within a session is owned by the session
        and will remain resident in memory until the session is released.
        Applications wishing to control the memory usage for compiled
        and loaded code should use multiple sessions.
        */
    struct ISession : public ISlangUnknown
    {
        SLANG_COM_INTERFACE( 0x67618701, 0xd116, 0x468f, { 0xab, 0x3b, 0x47, 0x4b, 0xed, 0xce, 0xe, 0x3d } )

            /** Get the global session thas was used to create this session.
            */
        virtual SLANG_NO_THROW IGlobalSession* SLANG_MCALL getGlobalSession() = 0;

            /** Load a module as it would be by code using `import`.
            */
        virtual SLANG_NO_THROW IModule* SLANG_MCALL loadModule(
            const char* moduleName,
            IBlob**     outDiagnostics = nullptr) = 0;

            /** Load a module from Slang source code.
            */
        virtual SLANG_NO_THROW IModule* SLANG_MCALL loadModuleFromSource(
            const char* moduleName,
            slang::IBlob* source,
            slang::IBlob** outDiagnostics = nullptr) = 0;

            /** Combine multiple component types to create a composite component type.

            The `componentTypes` array must contain `componentTypeCount` pointers
            to component types that were loaded or created using the same session.

            The shader parameters and specialization parameters of the composite will
            be the union of those in `componentTypes`. The relative order of child
            component types is significant, and will affect the order in which
            parameters are reflected and laid out.

            The entry-point functions of the composite will be the union of those in
            `componentTypes`, and will follow the ordering of `componentTypes`.

            The requirements of the composite component type will be a subset of
            those in `componentTypes`. If an entry in `componentTypes` has a requirement
            that can be satisfied by another entry, then the composition will
            satisfy the requirement and it will not appear as a requirement of
            the composite. If multiple entries in `componentTypes` have a requirement
            for the same type, then only the first such requirement will be retained
            on the composite. The relative ordering of requirements on the composite
            will otherwise match that of `componentTypes`.

            If any diagnostics are generated during creation of the composite, they
            will be written to `outDiagnostics`. If an error is encountered, the
            function will return null.

            It is an error to create a composite component type that recursively
            aggregates the a single module more than once.
            */
        virtual SLANG_NO_THROW SlangResult SLANG_MCALL createCompositeComponentType(
            IComponentType* const*  componentTypes,
            SlangInt                componentTypeCount,
            IComponentType**        outCompositeComponentType,
            ISlangBlob**            outDiagnostics = nullptr) = 0;

            /** Specialize a type based on type arguments.
            */
        virtual SLANG_NO_THROW TypeReflection* SLANG_MCALL specializeType(
            TypeReflection*             type,
            SpecializationArg const*    specializationArgs,
            SlangInt                    specializationArgCount,
            ISlangBlob**                outDiagnostics = nullptr) = 0;


            /** Get the layout `type` on the chosen `target`.
            */
        virtual SLANG_NO_THROW TypeLayoutReflection* SLANG_MCALL getTypeLayout(
            TypeReflection* type,
            SlangInt        targetIndex = 0,
            LayoutRules     rules = LayoutRules::Default,
            ISlangBlob**    outDiagnostics = nullptr) = 0;

            /** Get a container type from `elementType`. For example, given type `T`, returns
                a type that represents `StructuredBuffer<T>`.

                @param `elementType`: the element type to wrap around.
                @param `containerType`: the type of the container to wrap `elementType` in.
                @param `outDiagnostics`: a blob to receive diagnostic messages.
            */
        virtual SLANG_NO_THROW TypeReflection* SLANG_MCALL getContainerType(
            TypeReflection* elementType,
            ContainerType containerType,
            ISlangBlob** outDiagnostics = nullptr) = 0;

            /** Return a `TypeReflection` that represents the `__Dynamic` type.
                This type can be used as a specialization argument to indicate using
                dynamic dispatch.
            */
        virtual SLANG_NO_THROW TypeReflection* SLANG_MCALL getDynamicType() = 0;

            /** Get the mangled name for a type RTTI object.
            */
        virtual SLANG_NO_THROW SlangResult SLANG_MCALL getTypeRTTIMangledName(
            TypeReflection* type,
            ISlangBlob** outNameBlob) = 0;

            /** Get the mangled name for a type witness.
            */
        virtual SLANG_NO_THROW SlangResult SLANG_MCALL getTypeConformanceWitnessMangledName(
            TypeReflection* type,
            TypeReflection* interfaceType,
            ISlangBlob** outNameBlob) = 0;

            /** Get the sequential ID used to identify a type witness in a dynamic object.
            */
        virtual SLANG_NO_THROW SlangResult SLANG_MCALL getTypeConformanceWitnessSequentialID(
            slang::TypeReflection* type,
            slang::TypeReflection* interfaceType,
            uint32_t*              outId) = 0;

            /** Create a request to load/compile front-end code.
            */
        virtual SLANG_NO_THROW SlangResult SLANG_MCALL createCompileRequest(
            SlangCompileRequest**   outCompileRequest) = 0;

        
            /** Creates a `IComponentType` that represents a type's conformance to an interface.
                The retrieved `ITypeConformance` objects can be included in a composite `IComponentType`
                to explicitly specify which implementation types should be included in the final compiled
                code. For example, if an module defines `IMaterial` interface and `AMaterial`,
                `BMaterial`, `CMaterial` types that implements the interface, the user can exclude
                `CMaterial` implementation from the resulting shader code by explcitly adding
                `AMaterial:IMaterial` and `BMaterial:IMaterial` conformances to a composite
                `IComponentType` and get entry point code from it. The resulting code will not have
                anything related to `CMaterial` in the dynamic dispatch logic. If the user does not
                explicitly include any `TypeConformances` to an interface type, all implementations to
                that interface will be included by default. By linking a `ITypeConformance`, the user is
                also given the opportunity to specify the dispatch ID of the implementation type. If
                `conformanceIdOverride` is -1, there will be no override behavior and Slang will
                automatically assign IDs to implementation types. The automatically assigned IDs can be
                queried via `ISession::getTypeConformanceWitnessSequentialID`.

                Returns SLANG_OK if succeeds, or SLANG_FAIL if `type` does not conform to `interfaceType`.
            */
        virtual SLANG_NO_THROW SlangResult SLANG_MCALL createTypeConformanceComponentType(
            slang::TypeReflection* type,
            slang::TypeReflection* interfaceType,
            ITypeConformance** outConformance,
            SlangInt conformanceIdOverride,
            ISlangBlob** outDiagnostics) = 0;
    };

    #define SLANG_UUID_ISession ISession::getTypeGuid()

        /** A component type is a unit of shader code layout, reflection, and linking.

        A component type is a unit of shader code that can be included into
        a linked and compiled shader program. Each component type may have:

        * Zero or more uniform shader parameters, representing textures,
          buffers, etc. that the code in the component depends on.

        * Zero or more *specialization* parameters, which are type or
          value parameters that can be used to synthesize specialized
          versions of the component type.

        * Zero or more entry points, which are the individually invocable
          kernels that can have final code generated.

        * Zero or more *requirements*, which are other component
          types on which the component type depends.

        One example of a component type is a module of Slang code:

        * The global-scope shader parameters declared in the module are
          the parameters when considered as a component type.

        * Any global-scope generic or interface type parameters introduce
          specialization parameters for the module.

        * A module does not by default include any entry points when
          considered as a component type (although the code of the
          module might *declare* some entry points).

        * Any other modules that are `import`ed in the source code
          become requirements of the module, when considered as a
          component type.

        An entry point is another example of a component type:

        * The `uniform` parameters of the entry point function are
          its shader parameters when considered as a component type.

        * Any generic or interface-type parameters of the entry point
          introduce specialization parameters.

        * An entry point component type exposes a single entry point (itself).

        * An entry point has one requirement for the module in which
          it was defined.

        Component types can be manipulated in a few ways:

        * Multiple component types can be combined into a composite, which
          combines all of their code, parameters, etc.

        * A component type can be specialized, by "plugging in" types and
          values for its specialization parameters.

        * A component type can be laid out for a particular target, giving
          offsets/bindings to the shader parameters it contains.

        * Generated kernel code can be requested for entry points.

        */
    struct IComponentType : public ISlangUnknown
    {
        SLANG_COM_INTERFACE(0x5bc42be8, 0x5c50, 0x4929, { 0x9e, 0x5e, 0xd1, 0x5e, 0x7c, 0x24, 0x1, 0x5f })

            /** Get the runtime session that this component type belongs to.
            */
        virtual SLANG_NO_THROW ISession* SLANG_MCALL getSession() = 0;

            /** Get the layout for this program for the chosen `targetIndex`.

            The resulting layout will establish offsets/bindings for all
            of the global and entry-point shader parameters in the
            component type.

            If this component type has specialization parameters (that is,
            it is not fully specialized), then the resulting layout may
            be incomplete, and plugging in arguments for generic specialization
            parameters may result in a component type that doesn't have
            a compatible layout. If the component type only uses
            interface-type specialization parameters, then the layout
            for a specialization should be compatible with an unspecialized
            layout (all parameters in the unspecialized layout will have
            the same offset/binding in the specialized layout).

            If this component type is combined into a composite, then
            the absolute offsets/bindings of parameters may not stay the same.
            If the shader parameters in a component type don't make
            use of explicit binding annotations (e.g., `register(...)`),
            then the *relative* offset of shader parameters will stay
            the same when it is used in a composition.
            */
        virtual SLANG_NO_THROW ProgramLayout* SLANG_MCALL getLayout(
            SlangInt    targetIndex = 0,
            IBlob**     outDiagnostics = nullptr) = 0;

            /** Get the number of (unspecialized) specialization parameters for the component type.
            */
        virtual SLANG_NO_THROW SlangInt SLANG_MCALL getSpecializationParamCount() = 0;

            /** Get the compiled code for the entry point at `entryPointIndex` for the chosen `targetIndex`

            Entry point code can only be computed for a component type that
            has no specialization parameters (it must be fully specialized)
            and that has no requirements (it must be fully linked).

            If code has not already been generated for the given entry point and target,
            then a compilation error may be detected, in which case `outDiagnostics`
            (if non-null) will be filled in with a blob of messages diagnosing the error.
            */
        virtual SLANG_NO_THROW SlangResult SLANG_MCALL getEntryPointCode(
            SlangInt    entryPointIndex,
            SlangInt    targetIndex,
            IBlob**     outCode,
            IBlob**     outDiagnostics = nullptr) = 0;

            /** Specialize the component by binding its specialization parameters to concrete arguments.

            The `specializationArgs` array must have `specializationArgCount` entries, and
            this must match the number of specialization parameters on this component type.

            If any diagnostics (error or warnings) are produced, they will be written to `outDiagnostics`.
            */
        virtual SLANG_NO_THROW SlangResult SLANG_MCALL specialize(
            SpecializationArg const*    specializationArgs,
            SlangInt                    specializationArgCount,
            IComponentType**            outSpecializedComponentType,
            ISlangBlob**                outDiagnostics = nullptr) = 0;

            /** Link this component type against all of its unsatisifed dependencies.
            
            A component type may have unsatisfied dependencies. For example, a module
            depends on any other modules it `import`s, and an entry point depends
            on the module that defined it.

            A user can manually satisfy dependencies by creating a composite
            component type, and when doing so they retain full control over
            the relative ordering of shader parameters in the resulting layout.

            It is an error to try to generate/access compiled kernel code for
            a component type with unresolved dependencies, so if dependencies
            remain after whatever manual composition steps an application
            cares to peform, the `link()` function can be used to automatically
            compose in any remaining dependencies. The order of parameters
            (and hence the global layout) that results will be deterministic,
            but is not currently documented.
            */
        virtual SLANG_NO_THROW SlangResult SLANG_MCALL link(
            IComponentType**            outLinkedComponentType,
            ISlangBlob**                outDiagnostics = nullptr) = 0;

            /** Get entry point 'callable' functions accessible through the ISlangSharedLibrary interface.

            The functions remain in scope as long as the ISlangSharedLibrary interface is in scope.

            NOTE! Requires a compilation target of SLANG_HOST_CALLABLE.
    
            @param entryPointIndex  The index of the entry point to get code for.
            @param targetIndex      The index of the target to get code for (default: zero).
            @param outSharedLibrary A pointer to a ISharedLibrary interface which functions can be queried on.
            @returns                A `SlangResult` to indicate success or failure.
            */
        virtual SLANG_NO_THROW SlangResult SLANG_MCALL getEntryPointHostCallable(
            int                     entryPointIndex,
            int                     targetIndex,
            ISlangSharedLibrary**   outSharedLibrary,
            slang::IBlob**          outDiagnostics = 0) = 0;

            /** Get a new ComponentType object that represents a renamed entry point.

            The current object must be a single EntryPoint, or a CompositeComponentType or
            SpecializedComponentType that contains one EntryPoint component.
            */
        virtual SLANG_NO_THROW SlangResult SLANG_MCALL renameEntryPoint(
            const char* newName, IComponentType** outEntryPoint) = 0;
    };
    #define SLANG_UUID_IComponentType IComponentType::getTypeGuid()

    struct IEntryPoint : public IComponentType
    {
        SLANG_COM_INTERFACE(0x8f241361, 0xf5bd, 0x4ca0, { 0xa3, 0xac, 0x2, 0xf7, 0xfa, 0x24, 0x2, 0xb8 })
    };

    #define SLANG_UUID_IEntryPoint IEntryPoint::getTypeGuid()

    struct ITypeConformance : public IComponentType
    {
        SLANG_COM_INTERFACE(0x73eb3147, 0xe544, 0x41b5, { 0xb8, 0xf0, 0xa2, 0x44, 0xdf, 0x21, 0x94, 0xb })
    };
    #define SLANG_UUID_ITypeConformance ITypeConformance::getTypeGuid()

        /** A module is the granularity of shader code compilation and loading.

        In most cases a module corresponds to a single compile "translation unit."
        This will often be a single `.slang` or `.hlsl` file and everything it
        `#include`s.

        Notably, a module `M` does *not* include the things it `import`s, as these
        as distinct modules that `M` depends on. There is a directed graph of
        module dependencies, and all modules in the graph must belong to the
        same session (`ISession`).

        A module establishes a namespace for looking up types, functions, etc.
        */
    struct IModule : public IComponentType
    {
        SLANG_COM_INTERFACE(0xc720e64, 0x8722, 0x4d31, { 0x89, 0x90, 0x63, 0x8a, 0x98, 0xb1, 0xc2, 0x79 })

        virtual SLANG_NO_THROW SlangResult SLANG_MCALL findEntryPointByName(
            char const*     name,
            IEntryPoint**   outEntryPoint) = 0;
    };
    
    #define SLANG_UUID_IModule IModule::getTypeGuid()

        /** Argument used for specialization to types/values.
        */
    struct SpecializationArg
    {
        enum class Kind : int32_t
        {
            Unknown,    /**< An invalid specialization argument. */
            Type,       /**< Specialize to a type. */
        };

        /** The kind of specialization argument. */
        Kind kind;
        union
        {
            /** A type specialization argument, used for `Kind::Type`. */
            TypeReflection* type;
        };

        static SpecializationArg fromType(TypeReflection* inType)
        {
            SpecializationArg rs;
            rs.kind = Kind::Type;
            rs.type = inType;
            return rs;
        }
    };
}

// Passed into functions to create globalSession to identify the API version client code is
// using. 
#define SLANG_API_VERSION 0

/* Create a global session, with built in StdLib.

@param apiVersion Pass in SLANG_API_VERSION
@param outGlobalSession (out)The created global session. 
*/
SLANG_EXTERN_C SLANG_API SlangResult slang_createGlobalSession(
    SlangInt                apiVersion,
    slang::IGlobalSession** outGlobalSession);

/* Create a global session, but do not set up the stdlib. The stdlib can
then be loaded via loadStdLib or compileStdLib

@param apiVersion Pass in SLANG_API_VERSION
@param outGlobalSession (out)The created global session that doesn't have a StdLib setup.

NOTE! API is experimental and not ready for production code 
*/
SLANG_EXTERN_C SLANG_API SlangResult slang_createGlobalSessionWithoutStdLib(
    SlangInt                apiVersion,
    slang::IGlobalSession** outGlobalSession);

/* Returns a blob that contains the serialized stdlib.
Returns nullptr if there isn't an embedded stdlib.
*/
SLANG_API ISlangBlob* slang_getEmbeddedStdLib();

namespace slang
{
    inline SlangResult createGlobalSession(
        slang::IGlobalSession** outGlobalSession)
    {
        return slang_createGlobalSession(SLANG_API_VERSION, outGlobalSession);
    }
}

/** @see slang::ICompileRequest::getProgram
*/
SLANG_EXTERN_C SLANG_API SlangResult spCompileRequest_getProgram(
    SlangCompileRequest*    request,
    slang::IComponentType** outProgram);

/** @see slang::ICompileRequest::getProgramWithEntryPoints
*/
SLANG_EXTERN_C SLANG_API SlangResult spCompileRequest_getProgramWithEntryPoints(
    SlangCompileRequest*    request,
    slang::IComponentType** outProgram);

/** @see slang::ICompileRequest::getEntryPoint
*/
SLANG_EXTERN_C SLANG_API SlangResult spCompileRequest_getEntryPoint(
    SlangCompileRequest*    request,
    SlangInt                entryPointIndex,
    slang::IComponentType** outEntryPoint);

/** @see slang::ICompileRequest::getModule
*/
SLANG_EXTERN_C SLANG_API SlangResult spCompileRequest_getModule(
    SlangCompileRequest*    request,
    SlangInt                translationUnitIndex,
    slang::IModule**        outModule);

/** @see slang::ICompileRequest::getSession
*/
SLANG_EXTERN_C SLANG_API SlangResult spCompileRequest_getSession(
    SlangCompileRequest* request,
    slang::ISession** outSession);
#endif

/* DEPRECATED DEFINITIONS

Everything below this point represents deprecated APIs/definition that are only
being kept around for source/binary compatibility with old client code. New
code should not use any of these declarations, and the Slang API will drop these
declarations over time.
*/

#ifdef __cplusplus
extern "C" {
#endif

#define SLANG_ERROR_INSUFFICIENT_BUFFER SLANG_E_BUFFER_TOO_SMALL
#define SLANG_ERROR_INVALID_PARAMETER SLANG_E_INVALID_ARG

SLANG_API char const* spGetTranslationUnitSource(
    SlangCompileRequest*    request,
    int                     translationUnitIndex);

#ifdef __cplusplus
}
#endif

#endif


#include <assert.h>

#include <stdint.h>

#ifndef SLANG_CORE_SIGNAL_H
#define SLANG_CORE_SIGNAL_H


namespace Slang
{

enum class SignalType
{
    Unexpected,
    Unimplemented,
    AssertFailure,
    Unreachable,
    InvalidOperation,
    AbortCompilation,
};


// Note that message can be passed as nullptr for no message.
SLANG_RETURN_NEVER void handleSignal(SignalType type, char const* message);

#define SLANG_UNEXPECTED(reason) \
    ::Slang::handleSignal(::Slang::SignalType::Unexpected, reason)

#define SLANG_UNIMPLEMENTED_X(what) \
    ::Slang::handleSignal(::Slang::SignalType::Unimplemented, what)

#define SLANG_UNREACHABLE(msg) \
    ::Slang::handleSignal(::Slang::SignalType::Unreachable, msg)

#define SLANG_ASSERT_FAILURE(msg) \
    ::Slang::handleSignal(::Slang::SignalType::AssertFailure, msg)

#define SLANG_INVALID_OPERATION(msg) \
    ::Slang::handleSignal(::Slang::SignalType::InvalidOperation, msg)

#define SLANG_ABORT_COMPILATION(msg) \
    ::Slang::handleSignal(::Slang::SignalType::AbortCompilation, msg)


}

#endif


#define VARIADIC_TEMPLATE

namespace Slang
{
	typedef int32_t Int32;
	typedef uint32_t UInt32;

	typedef int64_t Int64;
	typedef uint64_t UInt64;

    // Define 
    typedef SlangUInt UInt;
    typedef SlangInt Int;

    static const UInt kMaxUInt = ~UInt(0);
    static const Int kMaxInt = Int(kMaxUInt >> 1);

//	typedef unsigned short Word;

	typedef intptr_t PtrInt;

    // TODO(JS): It looks like Index is actually 64 bit on 64 bit targets(!)
    // Previous discussions landed on Index being int32_t.

    // Type used for indexing, in arrays/views etc. Signed.
    typedef Int Index;
    typedef UInt UIndex;
    typedef Int Count;
    typedef UInt UCount;

    static const Index kMaxIndex = kMaxInt;

    typedef uint8_t Byte;

    // TODO(JS):
    // Perhaps these should be named Utf8, Utf16 and UnicodePoint/Rune/etc? For now, just keep it simple
    //
    typedef char Char8;
    // 16 bit character. Note much like in utf8, a character may or may not represent a code point (it can be part of a code point).  
    typedef uint16_t Char16;

    // Can always hold a unicode code point.
    typedef uint32_t Char32;

	template <typename T>
	inline T&& _Move(T & obj)
	{
		return static_cast<T&&>(obj);
	}

	template <typename T>
	inline void Swap(T & v0, T & v1)
	{
		T tmp = _Move(v0);
		v0 = _Move(v1);
		v1 = _Move(tmp);
	}

// TODO: Shouldn't these be SLANG_ prefixed?
#ifdef _MSC_VER
#define UNREACHABLE_RETURN(x)
#define UNREACHABLE(x)
#else
#define UNREACHABLE_RETURN(x) return x;
#define UNREACHABLE(x) x;
#endif

}

#ifdef _DEBUG
#define SLANG_EXPECT(VALUE, MSG) if(VALUE) {} else SLANG_ASSERT_FAILURE(MSG)
#define SLANG_ASSERT(VALUE) SLANG_EXPECT(VALUE, #VALUE)
#else
#define SLANG_EXPECT(VALUE, MSG) do {} while(0)
#define SLANG_ASSERT(VALUE) do {} while(0)
#endif

#define SLANG_RELEASE_ASSERT(VALUE) if(VALUE) {} else SLANG_ASSERT_FAILURE(#VALUE)
#define SLANG_RELEASE_EXPECT(VALUE, WHAT) if(VALUE) {} else SLANG_UNEXPECTED(WHAT)

template<typename T> void slang_use_obj(T&) {}

#define SLANG_UNREFERENCED_PARAMETER(P) slang_use_obj(P)
#define SLANG_UNREFERENCED_VARIABLE(P) slang_use_obj(P)
#endif

#ifndef SLANG_CORE_HASH_H
#define SLANG_CORE_HASH_H

#ifndef SLANG_CORE_MATH_H
#define SLANG_CORE_MATH_H

#include <cmath>

namespace Slang
{
    // Some handy constants

    // The largest positive (or negative) number 
#   define SLANG_HALF_MAX 65504.0f
    // Smallest (denormalized) value. 1 / 2^24
#   define SLANG_HALF_SUB_NORMAL_MIN (1.0f / 16777216.0f)

	class Math
	{
	public:
        // Use to fix type punning issues with strict aliasing
        union FloatIntUnion
        {
            float fvalue;
            int ivalue;

            SLANG_FORCE_INLINE static FloatIntUnion makeFromInt(int i) { FloatIntUnion cast; cast.ivalue = i; return cast; }
            SLANG_FORCE_INLINE static FloatIntUnion makeFromFloat(float f) { FloatIntUnion cast; cast.fvalue = f; return cast; }
        };
        union DoubleInt64Union
        {
            double dvalue;
            int64_t ivalue;
            SLANG_FORCE_INLINE static DoubleInt64Union makeFromInt64(int64_t i) { DoubleInt64Union cast; cast.ivalue = i; return cast; }
            SLANG_FORCE_INLINE static DoubleInt64Union makeFromDouble(double d) { DoubleInt64Union cast; cast.dvalue = d; return cast; }
        };
        
		static const float Pi;

        template <typename T>
        static T Abs(T a)
        {
            return (a < 0) ? -a : a;
        }

		template<typename T>
		static T Min(const T& v1, const T&v2)
		{
			return v1<v2?v1:v2;
		}
		template<typename T>
		static T Max(const T& v1, const T&v2)
		{
			return v1>v2?v1:v2;
		}
		template<typename T>
		static T Min(const T& v1, const T&v2, const T&v3)
		{
			return Min(v1, Min(v2, v3));
		}
		template<typename T>
		static T Max(const T& v1, const T&v2, const T&v3)
		{
			return Max(v1, Max(v2, v3));
		}
		template<typename T>
		static T Clamp(const T& val, const T& vmin, const T&vmax)
		{
			if (val < vmin) return vmin;
			else if (val > vmax) return vmax;
			else return val;
		}

		static inline int FastFloor(float x)
		{
			int i = (int)x;
			return i - (i > x);
		}

		static inline int FastFloor(double x)
		{
			int i = (int)x;
			return i - (i > x);
		}

		static inline int IsNaN(float x)
		{
			return std::isnan(x);
		}

		static inline int IsInf(float x)
		{
			return std::isinf(x);
		}

		static inline unsigned int Ones32(unsigned int x)
		{
			/* 32-bit recursive reduction using SWAR...
				but first step is mapping 2-bit values
				into sum of 2 1-bit values in sneaky way
			*/
			x -= ((x >> 1) & 0x55555555);
			x = (((x >> 2) & 0x33333333) + (x & 0x33333333));
			x = (((x >> 4) + x) & 0x0f0f0f0f);
			x += (x >> 8);
			x += (x >> 16);
			return(x & 0x0000003f);
		}

		static inline unsigned int Log2Floor(unsigned int x)
		{
			x |= (x >> 1);
			x |= (x >> 2);
			x |= (x >> 4);
			x |= (x >> 8);
			x |= (x >> 16);
			return(Ones32(x >> 1));
		}

		static inline unsigned int Log2Ceil(unsigned int x)
		{
			int y = (x & (x - 1));
			y |= -y;
			y >>= (32 - 1);
			x |= (x >> 1);
			x |= (x >> 2);
			x |= (x >> 4);
			x |= (x >> 8);
			x |= (x >> 16);
			return(Ones32(x >> 1) - y);
		}
		/*
		static inline int Log2(float x)
		{
			unsigned int ix = (unsigned int&)x;
			unsigned int exp = (ix >> 23) & 0xFF;
			int log2 = (unsigned int)(exp) - 127;

			return log2;
		}
		*/

        static bool AreNearlyEqual(double a, double b, double epsilon)
        {
            // If they are equal then we are done
            if (a == b)
            {
                return true;
            }

            const double absA = Abs(a);
            const double absB = Abs(b);
            const double diff = Abs(a - b);

            // https://en.wikipedia.org/wiki/Double_precision_floating-point_format
            const double minNormal = 2.2250738585072014e-308;
            // Either a or b are very close to being zero, so doing relative comparison isn't really appropriate
            if (a == 0.0 || b == 0.0 || (absA + absB < minNormal))
            {
                return diff < (epsilon * minNormal);
            }
            else
            {
                // Calculate a relative relative error
                return diff < epsilon * (absA + absB);
            }
        }

        template <typename T>
        static T getLowestBit(T val)
        {
            return val & (-val);
        }
	};
    inline int FloatAsInt(float val)
	{
        return Math::FloatIntUnion::makeFromFloat(val).ivalue; 
	}
    inline float IntAsFloat(int val)
	{
        return Math::FloatIntUnion::makeFromInt(val).fvalue; 
	}

    SLANG_FORCE_INLINE int64_t DoubleAsInt64(double val)
    {
        return Math::DoubleInt64Union::makeFromDouble(val).ivalue;
    }
    SLANG_FORCE_INLINE double Int64AsDouble(int64_t value)
    {
        return Math::DoubleInt64Union::makeFromInt64(value).dvalue;
    }

	inline unsigned short FloatToHalf(float val)
	{
        const auto x = FloatAsInt(val);
        
		unsigned short bits = (x >> 16) & 0x8000;
		unsigned short m = (x >> 12) & 0x07ff;
		unsigned int e = (x >> 23) & 0xff;
		if (e < 103)
			return bits;
		if (e > 142)
		{
			bits |= 0x7c00u;
			bits |= e == 255 && (x & 0x007fffffu);
			return bits;
		}
		if (e < 113)
		{
			m |= 0x0800u;
			bits |= (m >> (114 - e)) + ((m >> (113 - e)) & 1);
			return bits;
		}
		bits |= ((e - 112) << 10) | (m >> 1);
		bits += m & 1;
		return bits;
	}

	inline float HalfToFloat(unsigned short input)
	{
		static const auto magic = Math::FloatIntUnion::makeFromInt((127 + (127 - 15)) << 23);
		static const auto was_infnan = Math::FloatIntUnion::makeFromInt((127 + 16) << 23);
        Math::FloatIntUnion o;
		o.ivalue = (input & 0x7fff) << 13;     // exponent/mantissa bits
		o.fvalue *= magic.fvalue;                 // exponent adjust
		if (o.fvalue >= was_infnan.fvalue)        // make sure Inf/NaN survive
			o.ivalue |= 255 << 23;
		o.ivalue |= (input & 0x8000) << 16;    // sign bit
		return o.fvalue;
	}

	class Random
	{
	private:
		unsigned int seed;
	public:
		Random(int seed)
		{
			this->seed = seed;
		}
		int Next() // random between 0 and RandMax (currently 0x7fff)
		{
			return ((seed = ((seed << 12) + 150889L) % 714025) & 0x7fff);
		}
		int Next(int min, int max) // inclusive min, exclusive max
		{
			unsigned int a = ((seed = ((seed << 12) + 150889L) % 714025) & 0xFFFF);
			unsigned int b = ((seed = ((seed << 12) + 150889L) % 714025) & 0xFFFF);
			unsigned int r = (a << 16) + b;
			return min + r % (max - min);
		}
		float NextFloat()
		{
			return ((Next() << 15) + Next()) / ((float)(1 << 30));
		}
		float NextFloat(float valMin, float valMax)
		{
			return valMin + (valMax - valMin) * NextFloat();
		}
		static int RandMax()
		{
			return 0x7fff;
		}
	};
}

#endif 

#include <string.h>
#include <type_traits>

namespace Slang
{
    // Ideally Hash codes should be unsigned types - makes accumulation simpler (as overflow/underflow behavior are defined)
    // Only downside is around multiply, where unsigned multiply can be slightly slower on some targets.

    // HashCode - size may vary by platform. Typically has 'best' combination of bits/performance. Should not be exposed externally as value from same input may change depending on compilation platform.
    typedef unsigned int HashCode;

    // A fixed 64bit wide hash on all targets.
    typedef uint64_t HashCode64;
    // A fixed 32bit wide hash on all targets.
    typedef uint32_t HashCode32;

    SLANG_FORCE_INLINE HashCode32 toHash32(HashCode value) { return (sizeof(HashCode) == sizeof(int64_t)) ? (HashCode32(uint64_t(value) >> 32) ^ HashCode(value)) : HashCode32(value); }
    SLANG_FORCE_INLINE HashCode64 toHash64(HashCode value) { return (sizeof(HashCode) == sizeof(int64_t)) ? HashCode(value) : ((HashCode64(value) << 32) | value); }

    SLANG_FORCE_INLINE HashCode getHashCode(int64_t value)
    {
        return (sizeof(HashCode) == sizeof(int64_t)) ? HashCode(value) : (HashCode(uint64_t(value) >> 32) ^ HashCode(value));
    }
    SLANG_FORCE_INLINE HashCode getHashCode(uint64_t value)
    {
        return (sizeof(HashCode) == sizeof(uint64_t)) ? HashCode(value) : (HashCode(value >> 32) ^ HashCode(value));
    }

	inline HashCode getHashCode(double key)
	{
		return getHashCode(DoubleAsInt64(key));
	}
	inline HashCode getHashCode(float key)
	{
		return FloatAsInt(key);
	} 
	inline HashCode getHashCode(const char* buffer)
	{
		if (!buffer)
			return 0;
		HashCode hash = 0;
		auto str = buffer;
		HashCode c = HashCode(*str++);
		while (c)
		{
			hash = c + (hash << 6) + (hash << 16) - hash;
			c = HashCode(*str++);
		}
		return hash;
	} 
	inline HashCode getHashCode(char* buffer)
	{
		return getHashCode(const_cast<const char *>(buffer));
	}
    inline HashCode getHashCode(const char* buffer, size_t numChars)
    {
        HashCode hash = 0;
        for (size_t i = 0; i < numChars; ++i)
        {      
            hash = HashCode(buffer[i]) + (hash << 6) + (hash << 16) - hash;
        }
        return hash;
    }

    /* The 'Stable' hash code functions produce hashes that must be

    * The same result for the same inputs on all targets
    * Rarely change - as their values can change the output of the Slang API/Serialization

    Hash value used from the 'Stable' functions can also be used as part of serialization -
    so it is in effect part of the API.

    In effect this means changing a 'Stable' algorithm will typically require doing a new release. 
    */
    inline HashCode32 getStableHashCode32(const char* buffer, size_t numChars)
    {
        HashCode32 hash = 0;
        for (size_t i = 0; i < numChars; ++i)
        {
            hash = HashCode32(buffer[i]) + (hash << 6) + (hash << 16) - hash;
        }
        return hash;
    }

    inline HashCode64 getStableHashCode64(const char* buffer, size_t numChars)
    {
        // Use HashCode64 is assumed unsigned because hash requires wrap around behavior and int is undefined on over/underflows
        HashCode64 hash = 0;
        for (size_t i = 0; i < numChars; ++i)
        {
            hash = HashCode64(HashCode64(buffer[i])) + (hash << 6) + (hash << 16) - hash;
        }
        return hash;
    }

    // Hash functions with specific sized results
    // TODO(JS): We might want to implement HashCode as just an alias a suitable Hash32/Hash32 based on target.
    // For now just use Stable for 64bit.
    SLANG_FORCE_INLINE HashCode64 getHashCode64(const char* buffer, size_t numChars) { return getStableHashCode64(buffer, numChars); }
    SLANG_FORCE_INLINE HashCode32 getHashCode32(const char* buffer, size_t numChars) { return toHash32(getHashCode(buffer, numChars)); }

	template<int IsInt>
	class Hash
	{
	public:
	};
	template<>
	class Hash<1>
	{
	public:
		template<typename TKey>
		static HashCode getHashCode(TKey& key)
		{
			return (HashCode)key;
		}
	};
	template<>
	class Hash<0>
	{
	public:
		template<typename TKey>
		static HashCode getHashCode(TKey& key)
		{
			return HashCode(key.getHashCode());
		}
	};
	template<int IsPointer>
	class PointerHash
	{};
	template<>
	class PointerHash<1>
	{
	public:
		template<typename TKey>
		static HashCode getHashCode(TKey const& key)
		{
			return (HashCode)((PtrInt)key) / 16; // sizeof(typename std::remove_pointer<TKey>::type);
		}
	};
	template<>
	class PointerHash<0>
	{
	public:
		template<typename TKey>
		static HashCode getHashCode(TKey& key)
		{
			return Hash<std::is_integral<TKey>::value || std::is_enum<TKey>::value>::getHashCode(key);
		}
	};

	template<typename TKey>
	HashCode getHashCode(const TKey& key)
	{
		return PointerHash<std::is_pointer<TKey>::value>::getHashCode(key);
	}

	template<typename TKey>
	HashCode getHashCode(TKey& key)
	{
		return PointerHash<std::is_pointer<TKey>::value>::getHashCode(key);
	}

    inline HashCode combineHash(HashCode left, HashCode right)
    {
        return (left * 16777619) ^ right;
    }

    inline HashCode combineHash(HashCode hash0, HashCode hash1, HashCode hash2)
    {
        auto h = hash0;
        h = combineHash(h, hash1);
        h = combineHash(h, hash2);
        return h;
    }

    struct Hasher
    {
    public:
        Hasher() {}

            /// Hash the given `value` and combine it into this hash state
        template<typename T>
        void hashValue(T const& value)
        {
            // TODO: Eventually, we should replace `getHashCode`
            // with a "hash into" operation that takes the value
            // and a `Hasher`.

            m_hashCode = combineHash(m_hashCode, getHashCode(value));
        }

            /// Hash the given `object` and combine it into this hash state
        template<typename T>
        void hashObject(T const& object)
        {
            // TODO: Eventually, we should replace `getHashCode`
            // with a "hash into" operation that takes the value
            // and a `Hasher`.

            m_hashCode = combineHash(m_hashCode, object->getHashCode());
        }

            /// Combine the given `hash` code into the hash state.
            ///
            /// Note: users should prefer to use `hashValue` or `hashObject`
            /// when possible, as they may be able to ensure a higher-quality
            /// hash result (e.g., by using more bits to represent the state
            /// during hashing than are used for the final hash code).
            ///
        void addHash(HashCode hash)
        {
            m_hashCode = combineHash(m_hashCode, hash);
        }

        HashCode getResult() const
        {
            return m_hashCode;
        }

    private:
        HashCode m_hashCode = 0;
    };
}

#endif

#ifndef SLANG_CORE_TYPE_TRAITS_H
#define SLANG_CORE_TYPE_TRAITS_H

namespace Slang
{
	struct TraitResultYes
	{
		char x;
	};
	struct TraitResultNo
	{
		char x[2];
	};

	template <typename B, typename D>
	struct IsBaseOfTraitHost
	{
		operator B*() const { return nullptr; }
		operator D*() { return nullptr; }
	};

	template <typename B, typename D>
	struct IsBaseOf
	{
		template <typename T>
		static TraitResultYes Check(D*, T) { return TraitResultYes(); }
		static TraitResultNo Check(B*, int) { return TraitResultNo(); }
		enum { Value = sizeof(Check(IsBaseOfTraitHost<B, D>(), int())) == sizeof(TraitResultYes) };
	};

	template<bool B, class T = void>
	struct EnableIf {};

	template<class T>
	struct EnableIf<true, T> { typedef T type; };

	template <typename B, typename D>
	struct IsConvertible
	{
		static TraitResultYes Use(B) { return TraitResultYes(); };
		static TraitResultNo Use(...) { return TraitResultNo(); };
		enum { Value = sizeof(Use(*(D*)(nullptr))) == sizeof(TraitResultYes) };
	};
}

#endif



namespace Slang
{
    // Base class for all reference-counted objects
    class RefObject
    {
    private:
        UInt referenceCount;

    public:
        RefObject()
            : referenceCount(0)
        {}

        RefObject(const RefObject &)
            : referenceCount(0)
        {}

        RefObject& operator=(const RefObject& rhs) = default;

        virtual ~RefObject()
        {}

        UInt addReference()
        {
            return ++referenceCount;
        }

        UInt decreaseReference()
        {
            return --referenceCount;
        }

        UInt releaseReference()
        {
            SLANG_ASSERT(referenceCount != 0);
            if(--referenceCount == 0)
            {
                delete this;
                return 0;
            }
            return referenceCount;
        }

        bool isUniquelyReferenced()
        {
            SLANG_ASSERT(referenceCount != 0);
            return referenceCount == 1;
        }

        UInt debugGetReferenceCount()
        {
            return referenceCount;
        }
    };

    SLANG_FORCE_INLINE void addReference(RefObject* obj)
    {
        if(obj) obj->addReference();
    }

    SLANG_FORCE_INLINE void releaseReference(RefObject* obj)
    {
        if(obj) obj->releaseReference();
    }

    // For straight dynamic cast.
    // Use instead of dynamic_cast as it allows for replacement without using Rtti in the future
    template <typename T>
    SLANG_FORCE_INLINE T* dynamicCast(RefObject* obj) { return dynamic_cast<T*>(obj); }
    template <typename T>
    SLANG_FORCE_INLINE const T* dynamicCast(const RefObject* obj) { return dynamic_cast<const T*>(obj); }

    // Like a dynamicCast, but allows a type to implement a specific implementation that is suitable for it
    template <typename T>
    SLANG_FORCE_INLINE T* as(RefObject* obj) { return dynamicCast<T>(obj); }
    template <typename T>
    SLANG_FORCE_INLINE const T* as(const RefObject* obj) { return dynamicCast<T>(obj); }

    // "Smart" pointer to a reference-counted object
    template<typename T>
    struct RefPtr
    {
        RefPtr()
            : pointer(nullptr)
        {}

        RefPtr(T* p)
            : pointer(p)
        {
            addReference(p);
        }

        RefPtr(RefPtr<T> const& p)
            : pointer(p.pointer)
        {
            addReference(p.pointer);
        }

        RefPtr(RefPtr<T>&& p)
            : pointer(p.pointer)
        {
            p.pointer = nullptr;
        }

        template <typename U>
        RefPtr(RefPtr<U> const& p,
            typename EnableIf<IsConvertible<T*, U*>::Value, void>::type * = 0)
            : pointer(static_cast<U*>(p))
        {
            addReference(static_cast<U*>(p));
        }

#if 0
        void operator=(T* p)
        {
            T* old = pointer;
            addReference(p);
            pointer = p;
            releaseReference(old);
        }
#endif

        void operator=(RefPtr<T> const& p)
        {
            T* old = pointer;
            addReference(p.pointer);
            pointer = p.pointer;
            releaseReference(old);
        }

        void operator=(RefPtr<T>&& p)
        {
            T* old = pointer;
            pointer = p.pointer;
            p.pointer = old;
        }

        template <typename U>
        typename EnableIf<IsConvertible<T*, U*>::value, void>::type
            operator=(RefPtr<U> const& p)
        {
            T* old = pointer;
            addReference(p.pointer);
            pointer = p.pointer;
            releaseReference(old);
        }

        HashCode getHashCode()
        {
            // Note: We need a `RefPtr<T>` to hash the same as a `T*`,
            // so that a `T*` can be used as a key in a dictionary with
            // `RefPtr<T>` keys, and vice versa.
            //
            return Slang::getHashCode(pointer);
        }

        bool operator==(const T * ptr) const
        {
            return pointer == ptr;
        }

        bool operator!=(const T * ptr) const
        {
            return pointer != ptr;
        }

		bool operator==(RefPtr<T> const& ptr) const
		{
			return pointer == ptr.pointer;
		}

		bool operator!=(RefPtr<T> const& ptr) const
		{
			return pointer != ptr.pointer;
		}

        template<typename U>
        RefPtr<U> dynamicCast() const
        {
            return RefPtr<U>(Slang::dynamicCast<U>(pointer));
        }

        template<typename U>
        RefPtr<U> as() const
        {
            return RefPtr<U>(Slang::as<U>(pointer));
        }

        template <typename U>
        bool is() const { return Slang::as<U>(pointer) != nullptr; }

        ~RefPtr()
        {
            releaseReference(static_cast<Slang::RefObject*>(pointer));
        }

        T& operator*() const
        {
            return *pointer;
        }

        T* operator->() const
        {
            return pointer;
        }

		T * Ptr() const
		{
			return pointer;
		}

        operator T*() const
        {
            return pointer;
        }

        void attach(T* p)
        {
            T* old = pointer;
            pointer = p;
            releaseReference(old);
        }

        T* detach()
        {
            auto rs = pointer;
            pointer = nullptr;
            return rs;
        }

        SLANG_FORCE_INLINE void setNull()
        {
            releaseReference(pointer);
            pointer = nullptr;
        }

        /// Get ready for writing (nulls contents)
        SLANG_FORCE_INLINE T** writeRef() { *this = nullptr; return &pointer; }

        /// Get for read access
        SLANG_FORCE_INLINE T*const* readRef() const { return &pointer; }

    private:
        T* pointer;
	};

    // Helper type for implementing weak pointers. The object being pointed at weakly creates a WeakSink object
    // that other objects can reference and share. When the object is destroyed it detaches the sink
    // doing so will make other users call to 'get' return null. Thus any user of the WeakSink, must check if the weakly pointed to
    // things pointer is nullptr before using.
    template <typename T>
    class WeakSink : public RefObject
    {
    public:
        WeakSink(T* ptr):
            m_ptr(ptr)
        {
        }

        SLANG_FORCE_INLINE T* get() const { return m_ptr; }
        SLANG_FORCE_INLINE void detach() { m_ptr = nullptr; }

    private:
        T* m_ptr;
    };

    // A pointer that can be transformed to hold either a weak reference or a strong reference.
    template<typename T>
    class TransformablePtr
    {
    private:
        T* m_weakPtr = nullptr;
        RefPtr<T> m_strongPtr;

    public:
        TransformablePtr() = default;
        TransformablePtr(T* ptr) { *this = ptr; }
        TransformablePtr(RefPtr<T> ptr) { *this = ptr; }
        TransformablePtr(const TransformablePtr<T>& ptr) = default;

        void promoteToStrongReference() { m_strongPtr = m_weakPtr; }
        void demoteToWeakReference() { m_strongPtr = nullptr; }
        bool isStrongReference() const { return m_strongPtr != nullptr; }

        T& operator*() const { return *m_weakPtr; }

        T* operator->() const { return m_weakPtr; }

        T* Ptr() const { return m_weakPtr; }
        T* get() const { return m_weakPtr; }

        operator T*() const { return m_weakPtr; }
        operator RefPtr<T>() const { return m_weakPtr; }


        TransformablePtr<T>& operator=(T* ptr)
        {
            m_weakPtr = ptr;
            m_strongPtr = ptr;
            return *this;
        }
        template<typename U>
        TransformablePtr<T>& operator=(const RefPtr<U>& ptr)
        {
            m_weakPtr = ptr.Ptr();
            m_strongPtr = ptr;
            return *this;
        }
        
        HashCode getHashCode() const
        {
            // Note: We need a `RefPtr<T>` to hash the same as a `T*`,
            // so that a `T*` can be used as a key in a dictionary with
            // `RefPtr<T>` keys, and vice versa.
            //
            return Slang::getHashCode(m_weakPtr);
        }

        bool operator==(const T* ptr) const { return m_weakPtr == ptr; }

        bool operator!=(const T* ptr) const { return m_weakPtr != ptr; }

        bool operator==(RefPtr<T> const& ptr) const { return m_weakPtr == ptr.Ptr(); }

        bool operator!=(RefPtr<T> const& ptr) const { return m_weakPtr != ptr.Ptr(); }

        bool operator==(TransformablePtr<T> const& ptr) const { return m_weakPtr == ptr.m_weakPtr; }

        bool operator!=(TransformablePtr<T> const& ptr) const { return m_weakPtr != ptr.m_weakPtr; }
    };
}
#endif

#ifndef _MSC_VER
#ifndef SLANG_CORE_SECURE_CRT_H
#define SLANG_CORE_SECURE_CRT_H
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include <wchar.h>

inline void memcpy_s(void *dest, size_t numberOfElements, const void * src, size_t count)
{
	memcpy(dest, src, count);
}

#define _TRUNCATE ((size_t)-1)
#define _stricmp strcasecmp

inline void fopen_s(FILE**f, const char * fileName, const char * mode)
{
	*f = fopen(fileName, mode);
}

inline size_t fread_s(void * buffer, size_t bufferSize, size_t elementSize, size_t count, FILE * stream)
{
	return fread(buffer, elementSize, count, stream);
}

inline size_t wcsnlen_s(const wchar_t * str, size_t /*numberofElements*/)
{
	return wcslen(str);
}

inline size_t strnlen_s(const char * str, size_t numberOfElements)
{
#if defined( __CYGWIN__ )
    const char* cur = str;
    if (str)
    {
        const char*const end = str + numberOfElements;
        while (*cur && cur < end) cur++;
    }
    return size_t(cur - str);
#else
	return strnlen(str, numberOfElements);
#endif
}

inline int sprintf_s(char * buffer, size_t sizeOfBuffer, const char * format, ...)
{
	va_list argptr;
	va_start(argptr, format);
	int rs = vsnprintf(buffer, sizeOfBuffer, format, argptr);
	va_end(argptr);
	return rs;
}

inline int swprintf_s(wchar_t * buffer, size_t sizeOfBuffer, const wchar_t * format, ...)
{
	va_list argptr;
	va_start(argptr, format);
	int rs = vswprintf(buffer, sizeOfBuffer, format, argptr);
	va_end(argptr);
	return rs;
}

inline void wcscpy_s(wchar_t * strDestination, size_t /*numberOfElements*/, const wchar_t * strSource)
{
	wcscpy(strDestination, strSource);
}
inline void strcpy_s(char * strDestination, size_t /*numberOfElements*/, const char * strSource)
{
	strcpy(strDestination, strSource);
}

inline void wcsncpy_s(wchar_t * strDestination, size_t /*numberOfElements*/, const wchar_t * strSource, size_t count)
{
	wcscpy(strDestination, strSource);
	//wcsncpy(strDestination, strSource, count);
}
inline void strncpy_s(char * strDestination, size_t /*numberOfElements*/, const char * strSource, size_t count)
{
	strncpy(strDestination, strSource, count);
	//wcsncpy(strDestination, strSource, count);
}
#endif
#endif


#include <new>

namespace Slang
{
    class _EndLine
    {};
    extern _EndLine EndLine;

    // in-place reversion, works only for ascii string
    inline void ReverseInternalAscii(char * buffer, int length)
    {
        int i, j;
        char c;
        for (i = 0, j = length - 1; i<j; i++, j--)
        {
            c = buffer[i];
            buffer[i] = buffer[j];
            buffer[j] = c;
        }
    }
    template<typename IntType>
    inline int IntToAscii(char * buffer, IntType val, int radix)
    {
        int i = 0;
        IntType sign;
        sign = val;
        if (sign < 0)
            val = (IntType)(0 - val);
        do
        {
            int digit = (val % radix);
            if (digit <= 9)
                buffer[i++] = (char)(digit + '0');
            else
                buffer[i++] = (char)(digit - 10 + 'A');
        } while ((val /= radix) > 0);
        if (sign < 0)
            buffer[i++] = '-';
        buffer[i] = '\0';
        return i;
    }

    inline bool IsUtf8LeadingByte(char ch)
    {
        return (((unsigned char)ch) & 0xC0) == 0xC0;
    }

    inline bool IsUtf8ContinuationByte(char ch)
    {
        return (((unsigned char)ch) & 0xC0) == 0x80;
    }

    struct UnownedStringSlice
    {
    public:
        typedef UnownedStringSlice ThisType;

        UnownedStringSlice()
            : m_begin(nullptr)
            , m_end(nullptr)
        {}

        explicit UnownedStringSlice(char const* a) :
            m_begin(a),
            m_end(a ? a + strlen(a) : nullptr)
        {}
        UnownedStringSlice(char const* b, char const* e)
            : m_begin(b)
            , m_end(e)
        {}
        UnownedStringSlice(char const* b, size_t len)
            : m_begin(b)
            , m_end(b + len)
        {}

        char const* begin() const
        {
            return m_begin;
        }

        char const* end() const
        {
            return m_end;
        }

            /// True if slice is strictly contained in memory.
        bool isMemoryContained(const UnownedStringSlice& slice) const
        {
            return slice.m_begin >= m_begin && slice.m_end <= m_end; 
        }
        bool isMemoryContained(const char* pos) const
        {
            return pos >= m_begin && pos <= m_end;
        }

        Index getLength() const
        {
            return Index(m_end - m_begin);
        }

            /// Finds first index of char 'c'. If not found returns -1.
        Index indexOf(char c) const;
            /// Find first index of slice. If not found returns -1
        Index indexOf(const UnownedStringSlice& slice) const;

            /// Returns a substring. idx is the start index, and len
            /// is the amount of characters.
            /// The returned length might be truncated, if len extends beyond slice.
        UnownedStringSlice subString(Index idx, Index len) const;

            /// Return a head of the slice - everything up to the index
        SLANG_FORCE_INLINE UnownedStringSlice head(Index idx) const { SLANG_ASSERT(idx >= 0 && idx <= getLength()); return UnownedStringSlice(m_begin, idx); }
            /// Return a tail of the slice - everything from the index to the end of the slice
        SLANG_FORCE_INLINE UnownedStringSlice tail(Index idx) const { SLANG_ASSERT(idx >= 0 && idx <= getLength()); return UnownedStringSlice(m_begin + idx, m_end); }

            /// True if rhs and this are equal without having to take into account case
            /// Note 'case' here is *not* locale specific - it is only A-Z and a-z
        bool caseInsensitiveEquals(const ThisType& rhs) const;

        Index lastIndexOf(char c) const
        {
            const Index size = Index(m_end - m_begin);
            for (Index i = size - 1; i >= 0; --i)
            {
                if (m_begin[i] == c)
                {
                    return i;
                }
            }
            return -1;
        }

        const char& operator[](Index i) const
        {
            assert(i >= 0 && i < Index(m_end - m_begin));
            return m_begin[i];
        }

        bool operator==(ThisType const& other) const;
        bool operator!=(UnownedStringSlice const& other) const { return !(*this == other);  }

        bool operator==(char const* str) const { return (*this) == UnownedStringSlice(str); }
        bool operator!=(char const* str) const { return !(*this == str); }

            /// True if contents is a single char of c
        SLANG_FORCE_INLINE bool isChar(char c) const { return getLength() == 1 && m_begin[0] == c; }

        bool startsWith(UnownedStringSlice const& other) const;
        bool startsWith(char const* str) const;

        bool endsWith(UnownedStringSlice const& other) const;
        bool endsWith(char const* str) const;

            /// Trims any horizontal whitespace from the start and end and returns as a substring 
        UnownedStringSlice trim() const;
            /// Trims any 'c' from the start or the end, and returns as a substring
        UnownedStringSlice trim(char c) const;

            /// Trims any horizonatl whitespace from start and returns as a substring
        UnownedStringSlice trimStart() const;

        HashCode getHashCode() const
        {
            return Slang::getHashCode(m_begin, size_t(m_end - m_begin)); 
        }

        template <size_t SIZE> 
        SLANG_FORCE_INLINE static UnownedStringSlice fromLiteral(const char (&in)[SIZE]) { return UnownedStringSlice(in, SIZE - 1); }

    private:
        char const* m_begin;
        char const* m_end;
    };

    // A more convenient way to make slices from *string literals*
    template <size_t SIZE>
    SLANG_FORCE_INLINE UnownedStringSlice toSlice(const char (&in)[SIZE]) { return UnownedStringSlice(in, SIZE - 1); }

    // A `StringRepresentation` provides the backing storage for
    // all reference-counted string-related types.
    class StringRepresentation : public RefObject
    {
    public:
        Index length;
        Index capacity;

        SLANG_FORCE_INLINE Index getLength() const
        {
            return length;
        }

        SLANG_FORCE_INLINE char* getData()
        {
            return (char*) (this + 1);
        }
        SLANG_FORCE_INLINE const char* getData() const
        {
            return (const char*)(this + 1);
        }

            /// Set the contents to be the slice. Must be enough capacity to hold the slice. 
        void setContents(const UnownedStringSlice& slice);

        static const char* getData(const StringRepresentation* stringRep)
        {
            return stringRep ? stringRep->getData() : "";
        }

        static UnownedStringSlice asSlice(const StringRepresentation* rep)
        {
            return rep ? UnownedStringSlice(rep->getData(), rep->getLength()) : UnownedStringSlice();
        }

        static bool equal(const StringRepresentation* a, const StringRepresentation* b)
        {
            return (a == b) || asSlice(a) == asSlice(b);
        }

        static StringRepresentation* createWithCapacityAndLength(Index capacity, Index length)
        {
            SLANG_ASSERT(capacity >= length);
            void* allocation = operator new(sizeof(StringRepresentation) + capacity + 1);
            StringRepresentation* obj = new(allocation) StringRepresentation();
            obj->capacity = capacity;
            obj->length = length;
            obj->getData()[length] = 0;
            return obj;
        }

        static StringRepresentation* createWithCapacity(Index capacity)
        {
            return createWithCapacityAndLength(capacity, 0);
        }

        static StringRepresentation* createWithLength(Index length)
        {
            return createWithCapacityAndLength(length, length);
        }

            /// Create a representation from the slice. If slice is empty will return nullptr.
        static StringRepresentation* create(const UnownedStringSlice& slice);
            /// Same as create, but representation will have refcount of 1 (if not nullptr)
        static StringRepresentation* createWithReference(const UnownedStringSlice& slice);

        StringRepresentation* cloneWithCapacity(Index newCapacity)
        {
            StringRepresentation* newObj = createWithCapacityAndLength(newCapacity, length);
            memcpy(getData(), newObj->getData(), length + 1);
            return newObj;
        }

        StringRepresentation* clone()
        {
            return cloneWithCapacity(length);
        }

        StringRepresentation* ensureCapacity(Index required)
        {
            if (capacity >= required) return this;

            Index newCapacity = capacity;
            if (!newCapacity) newCapacity = 16; // TODO: figure out good value for minimum capacity

            while (newCapacity < required)
            {
                newCapacity = 2 * newCapacity;
            }

            return cloneWithCapacity(newCapacity);
        }
    };

    class String;



    struct UnownedTerminatedStringSlice : public UnownedStringSlice
    {
    public:
        UnownedTerminatedStringSlice(char const* b)
            : UnownedStringSlice(b, b + (b?strlen(b):0))
        {}
    };

    struct StringSlice
    {
    public:
        StringSlice();

        StringSlice(String const& str);

        StringSlice(String const& str, UInt beginIndex, UInt endIndex);

        UInt getLength() const
        {
            return endIndex - beginIndex;
        }

        char const* begin() const
        {
            return representation ? representation->getData() + beginIndex : "";
        }

        char const* end() const
        {
            return begin() + getLength();
        }

    private:
        RefPtr<StringRepresentation> representation;
        UInt beginIndex;
        UInt endIndex;

        friend class String;

        StringSlice(RefPtr<StringRepresentation> const& representation, UInt beginIndex, UInt endIndex)
            : representation(representation)
            , beginIndex(beginIndex)
            , endIndex(endIndex)
        {}
    };

    /// String as expected by underlying platform APIs
    class OSString
    {
    public:
            /// Default
        OSString();
            /// NOTE! This assumes that begin is a new wchar_t[] buffer, and it will
            /// now be owned by the OSString
        OSString(wchar_t* begin, wchar_t* end);
            /// Move Ctor
        OSString(OSString&& rhs):
            m_begin(rhs.m_begin),
            m_end(rhs.m_end)
        {
            rhs.m_begin = nullptr;
            rhs.m_end = nullptr;
        }
            // Copy Ctor
        OSString(const OSString& rhs) :
            m_begin(nullptr),
            m_end(nullptr)
        {
            set(rhs.m_begin, rhs.m_end);
        }

            /// =
        void operator=(const OSString& rhs) { set(rhs.m_begin, rhs.m_end); }
        void operator=(OSString&& rhs)
        {
            auto begin = m_begin;
            auto end = m_end;
            m_begin = rhs.m_begin;
            m_end = rhs.m_end;
            rhs.m_begin = begin;
            rhs.m_end = end;
        }

        ~OSString() { _releaseBuffer(); }

        size_t getLength() const { return (m_end - m_begin); }
        void set(const wchar_t* begin, const wchar_t* end);

        operator wchar_t const*() const
        {
            return begin();
        }

        wchar_t const* begin() const;
        wchar_t const* end() const;

    private:

        void _releaseBuffer();

        wchar_t* m_begin;           ///< First character. This is a new wchar_t[] buffer
        wchar_t* m_end;             ///< Points to terminating 0
    };

    /*!
    @brief Represents a UTF-8 encoded string.
    */

    class String
    {
        friend struct StringSlice;
        friend class StringBuilder;
    private:


        char* getData() const
        {
            return m_buffer ? m_buffer->getData() : (char*)"";
        }

     
        void ensureUniqueStorageWithCapacity(Index capacity);
     
        RefPtr<StringRepresentation> m_buffer;

    public:

        explicit String(StringRepresentation* buffer)
            : m_buffer(buffer)
        {}

        static String fromWString(const wchar_t * wstr);
        static String fromWString(const wchar_t * wstr, const wchar_t * wend);
        static String fromWChar(const wchar_t ch);
        static String fromUnicodePoint(Char32 codePoint);
        String()
        {
        }

            /// Returns a buffer which can hold at least count chars
        char* prepareForAppend(Index count);
            /// Append data written to buffer output via 'prepareForAppend' directly written 'inplace'
        void appendInPlace(const char* chars, Index count);

        SLANG_FORCE_INLINE StringRepresentation* getStringRepresentation() const { return m_buffer; }

        const char * begin() const
        {
            return getData();
        }
        const char * end() const
        {
            return getData() + getLength();
        }

        void append(int32_t value, int radix = 10);
        void append(uint32_t value, int radix = 10);
        void append(int64_t value, int radix = 10);
        void append(uint64_t value, int radix = 10);
        void append(float val, const char * format = "%g");
        void append(double val, const char * format = "%g");

        void append(char const* str);
        void append(const char* textBegin, char const* textEnd);
        void append(char chr);
        void append(String const& str);
        void append(StringSlice const& slice);
        void append(UnownedStringSlice const& slice);

            /// Append a character (to remove ambiguity with other integral types)
        void appendChar(char chr);

            /// Append the specified char count times
        void appendRepeatedChar(char chr, Index count);

        String(const char* str)
        {
            append(str);
#if 0
            if (str)
            {
                buffer = StringRepresentation::createWithLength(strlen(str));
                memcpy(buffer.Ptr(), str, getLength() + 1);
            }
#endif
        }
        String(const char* textBegin, char const* textEnd)
        {
            append(textBegin, textEnd);
#if 0
            if (textBegin != textEnd)
            {
                buffer = StringRepresentation::createWithLength(textEnd - textBegin);
                memcpy(buffer.Ptr(), textBegin, getLength());
                buffer->getData()[getLength()] = 0;
            }
#endif
        }

        // Make all String ctors from a numeric explicit, to avoid unexpected/unnecessary conversions
        explicit String(int32_t val, int radix = 10)
        {
            append(val, radix);
        }
        explicit String(uint32_t val, int radix = 10)
        {
            append(val, radix);
        }
        explicit String(int64_t val, int radix = 10)
        {
            append(val, radix);
        }
        explicit String(uint64_t val, int radix = 10)
        {
            append(val, radix);
        }
        explicit String(float val, const char * format = "%g")
        {
            append(val, format);
        }
        explicit String(double val, const char * format = "%g")
        {
            append(val, format);
        }

        explicit String(char chr)
        {
            append(chr);
#if 0
            if (chr)
            {
                buffer = StringRepresentation::createWithLength(1);
                buffer->getData()[0] = chr;
                buffer->getData()[1] = 0;
            }
#endif
        }
        String(String const& str)
        {
            m_buffer = str.m_buffer;
#if 0
            this->operator=(str);
#endif
        }
        String(String&& other)
        {
            m_buffer = _Move(other.m_buffer);
        }

        String(StringSlice const& slice)
        {
            append(slice);
        }

        String(UnownedStringSlice const& slice)
        {
            append(slice);
        }

        ~String()
        {
            m_buffer.setNull(); 
        }

        String & operator=(const String & str)
        {
            m_buffer = str.m_buffer;
            return *this;
        }
        String & operator=(String&& other)
        {
            m_buffer = _Move(other.m_buffer);
            return *this;
        }
        char operator[](Index id) const
        {
            SLANG_ASSERT(id >= 0 && id < getLength());
            return begin()[id];
        }

        Index getLength() const
        {
            return m_buffer ? m_buffer->getLength() : 0;
        }
            /// Make the length of the string the amount specified. Must be less than current size
        void reduceLength(Index length);
        
        friend String operator+(const char*op1, const String & op2);
        friend String operator+(const String & op1, const char * op2);
        friend String operator+(const String & op1, const String & op2);

        StringSlice trimStart() const
        {
            if (!m_buffer)
                return StringSlice();
            Index startIndex = 0;
            const char*const data = getData();
            while (startIndex < getLength() &&
                (data[startIndex] == ' ' || data[startIndex] == '\t' || data[startIndex] == '\r' || data[startIndex] == '\n'))
                startIndex++;
            return StringSlice(m_buffer, startIndex, getLength());
        }

        StringSlice trimEnd() const
        {
            if (!m_buffer)
                return StringSlice();

            Index endIndex = getLength();
            const char*const data = getData();
            while (endIndex > 0 &&
                (data[endIndex-1] == ' ' || data[endIndex-1] == '\t' || data[endIndex-1] == '\r' || data[endIndex-1] == '\n'))
                endIndex--;

            return StringSlice(m_buffer, 0, endIndex);
        }

        StringSlice trim() const
        {
            if (!m_buffer)
                return StringSlice();

            Index startIndex = 0;
            const char*const data = getData();
            while (startIndex < getLength() &&
                (data[startIndex] == ' ' || data[startIndex] == '\t'))
                startIndex++;
            Index endIndex = getLength();
            while (endIndex > startIndex &&
                (data[endIndex-1] == ' ' || data[endIndex-1] == '\t'))
                endIndex--;

            return StringSlice(m_buffer, startIndex, endIndex);
        }

        StringSlice subString(Index id, Index len) const
        {
            if (len == 0)
                return StringSlice();

            if (id + len > getLength())
                len = getLength() - id;
#if _DEBUG
            if (id < 0 || id >= getLength() || (id + len) > getLength())
                SLANG_ASSERT_FAILURE("SubString: index out of range.");
            if (len < 0)
                SLANG_ASSERT_FAILURE("SubString: length less than zero.");
#endif
            return StringSlice(m_buffer, id, id + len);
        }

        char const* getBuffer() const
        {
            return getData();
        }

        OSString toWString(Index* len = 0) const;

        bool equals(const String & str, bool caseSensitive = true)
        {
            if (caseSensitive)
                return (strcmp(begin(), str.begin()) == 0);
            else
            {
#ifdef _MSC_VER
                return (_stricmp(begin(), str.begin()) == 0);
#else
                return (strcasecmp(begin(), str.begin()) == 0);
#endif
            }
        }
        bool operator==(const char * strbuffer) const
        {
            return (strcmp(begin(), strbuffer) == 0);
        }

        bool operator==(const String & str) const
        {
            return (strcmp(begin(), str.begin()) == 0);
        }
        bool operator!=(const char * strbuffer) const
        {
            return (strcmp(begin(), strbuffer) != 0);
        }
        bool operator!=(const String & str) const
        {
            return (strcmp(begin(), str.begin()) != 0);
        }
        bool operator>(const String & str) const
        {
            return (strcmp(begin(), str.begin()) > 0);
        }
        bool operator<(const String & str) const
        {
            return (strcmp(begin(), str.begin()) < 0);
        }
        bool operator>=(const String & str) const
        {
            return (strcmp(begin(), str.begin()) >= 0);
        }
        bool operator<=(const String & str) const
        {
            return (strcmp(begin(), str.begin()) <= 0);
        }

        SLANG_FORCE_INLINE bool operator==(const UnownedStringSlice& slice) const { return getUnownedSlice() == slice; }
        SLANG_FORCE_INLINE bool operator!=(const UnownedStringSlice& slice) const { return getUnownedSlice() != slice; }

        String toUpper() const
        {
            String result;
            for (auto c : *this)
            {
                char d = (c >= 'a' && c <= 'z') ? (c - ('a' - 'A')) : c;
                result.append(d);
            }
            return result;
        }

        String toLower() const
        {
            String result;
            for (auto c : *this)
            {
                char d = (c >= 'A' && c <= 'Z') ? (c - ('A' - 'a')) : c;
                result.append(d);
            }
            return result;
        }

        Index indexOf(const char * str, Index id) const // String str
        {
            if (id >= getLength())
                return Index(-1);
            auto findRs = strstr(begin() + id, str);
            Index res = findRs ? findRs - begin() : Index(-1);
            return res;
        }

        Index indexOf(const String & str, Index id) const
        {
            return indexOf(str.begin(), id);
        }

        Index indexOf(const char * str) const
        {
            return indexOf(str, 0);
        }

        Index indexOf(const String & str) const
        {
            return indexOf(str.begin(), 0);
        }

        Index indexOf(char ch, Index id) const
        {
            const Index length = getLength();
            SLANG_ASSERT(id >= 0 && id <= length);

            if (!m_buffer)
                return Index(-1);

            const char* data = getData();
            for (Index i = id; i < length; i++)
                if (data[i] == ch)
                    return i;
            return Index(-1);
        }

        Index indexOf(char ch) const
        {
            return indexOf(ch, 0);
        }

        Index lastIndexOf(char ch) const
        {            
            const Index length = getLength();
            const char* data = getData();

            // TODO(JS): If we know Index is signed we can do this a bit more simply

            for (Index i = length; i > 0; i--)
                if (data[i - 1] == ch)
                    return i - 1;
            return Index(-1);
        }

        bool startsWith(const char * str) const // String str
        {
            if (!m_buffer)
                return false;
            Index strLen = Index(::strlen(str));
            if (strLen > getLength())
                return false;

            const char*const data = getData();

            for (Index i = 0; i < strLen; i++)
                if (str[i] != data[i])
                    return false;
            return true;
        }

        bool startsWith(const String& str) const
        {
            return startsWith(str.begin());
        }

        bool endsWith(char const * str)  const // String str
        {
            if (!m_buffer)
                return false;

            const Index strLen = Index(::strlen(str));
            const Index len = getLength();

            if (strLen > len)
                return false;
            const char* data = getData();
            for (Index i = strLen; i > 0; i--)
                if (str[i-1] != data[len - strLen + i-1])
                    return false;
            return true;
        }

        bool endsWith(const String & str) const
        {
            return endsWith(str.begin());
        }

        bool contains(const char * str) const // String str
        {
            return m_buffer && indexOf(str) != Index(-1); 
        }

        bool contains(const String & str) const
        {
            return contains(str.begin());
        }

        HashCode getHashCode() const
        {
            return Slang::getHashCode(StringRepresentation::asSlice(m_buffer));
        }

        UnownedStringSlice getUnownedSlice() const
        {
            return StringRepresentation::asSlice(m_buffer);
        }
    };

    class StringBuilder : public String
    {
    private:
        enum { InitialSize = 1024 };
    public:
        explicit StringBuilder(UInt bufferSize = InitialSize)
        {
            ensureUniqueStorageWithCapacity(bufferSize);
        }
        void EnsureCapacity(UInt size)
        {
            ensureUniqueStorageWithCapacity(size);
        }
        StringBuilder & operator << (char ch)
        {
            Append(&ch, 1);
            return *this;
        }
        StringBuilder & operator << (Int32 val)
        {
            Append(val);
            return *this;
        }
        StringBuilder & operator << (UInt32 val)
        {
            Append(val);
            return *this;
        }
        StringBuilder & operator << (Int64 val)
        {
            Append(val);
            return *this;
        }
        StringBuilder & operator << (UInt64 val)
        {
            Append(val);
            return *this;
        }
        StringBuilder & operator << (float val)
        {
            Append(val);
            return *this;
        }
        StringBuilder & operator << (double val)
        {
            Append(val);
            return *this;
        }
        StringBuilder & operator << (const char * str)
        {
            Append(str, strlen(str));
            return *this;
        }
        StringBuilder & operator << (const String & str)
        {
            Append(str);
            return *this;
        }
        StringBuilder & operator << (UnownedStringSlice const& str)
        {
            append(str);
            return *this;
        }
        StringBuilder & operator << (const _EndLine)
        {
            Append('\n');
            return *this;
        }
        void Append(char ch)
        {
            Append(&ch, 1);
        }
        void Append(float val)
        {
            char buf[128];
            sprintf_s(buf, 128, "%g", val);
            int len = (int)strnlen_s(buf, 128);
            Append(buf, len);
        }
        void Append(double val)
        {
            char buf[128];
            sprintf_s(buf, 128, "%g", val);
            int len = (int)strnlen_s(buf, 128);
            Append(buf, len);
        }
        void Append(Int32 value, int radix = 10)
        {
            char vBuffer[33];
            int len = IntToAscii(vBuffer, value, radix);
            ReverseInternalAscii(vBuffer, len);
            Append(vBuffer);
        }
        void Append(UInt32 value, int radix = 10)
        {
            char vBuffer[33];
            int len = IntToAscii(vBuffer, value, radix);
            ReverseInternalAscii(vBuffer, len);
            Append(vBuffer);
        }
        void Append(Int64 value, int radix = 10)
        {
            char vBuffer[65];
            int len = IntToAscii(vBuffer, value, radix);
            ReverseInternalAscii(vBuffer, len);
            Append(vBuffer);
        }
        void Append(UInt64 value, int radix = 10)
        {
            char vBuffer[65];
            int len = IntToAscii(vBuffer, value, radix);
            ReverseInternalAscii(vBuffer, len);
            Append(vBuffer);
        }
        void Append(const String & str)
        {
            Append(str.getBuffer(), str.getLength());
        }
        void Append(const char * str)
        {
            Append(str, strlen(str));
        }
        void Append(const char * str, UInt strLen)
        {
            append(str, str + strLen);
        }

#if 0
        int Capacity()
        {
            return bufferSize;
        }

        char * Buffer()
        {
            return buffer;
        }

        int Length()
        {
            return length;
        }
#endif

        String ToString()
        {
            return *this;
        }

        String ProduceString()
        {
            return *this;
        }

#if 0
        String GetSubString(int start, int count)
        {
            String rs;
            rs.buffer = new char[count + 1];
            rs.length = count;
            strncpy_s(rs.buffer.Ptr(), count + 1, buffer + start, count);
            rs.buffer[count] = 0;
            return rs;
        }
#endif

#if 0
        void Remove(int id, int len)
        {
#if _DEBUG
            if (id >= length || id < 0)
                SLANG_ASSERT_FAILURE("Remove: Index out of range.");
            if (len < 0)
                SLANG_ASSERT_FAILURE("Remove: remove length smaller than zero.");
#endif
            int actualDelLength = ((id + len) >= length) ? (length - id) : len;
            for (int i = id + actualDelLength; i <= length; i++)
                buffer[i - actualDelLength] = buffer[i];
            length -= actualDelLength;
        }
#endif

        void Clear()
        {
            m_buffer.setNull();
        }
    };

    int StringToInt(const String & str, int radix = 10);
    unsigned int StringToUInt(const String & str, int radix = 10);
    double StringToDouble(const String & str);
    float StringToFloat(const String & str);
}

#endif

#ifndef SLANG_COM_PTR_H
#define SLANG_COM_PTR_H

#ifndef SLANG_COM_HELPER_H
#define SLANG_COM_HELPER_H

/** \file slang-com-helper.h
*/

#include <atomic>

/* !!!!!!!!!!!!!!!!!!!!! Macros to help checking SlangResult !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

/*! Set SLANG_HANDLE_RESULT_FAIL(x) to code to be executed whenever an error occurs, and is detected by one of the macros */
#ifndef SLANG_HANDLE_RESULT_FAIL
#	define SLANG_HANDLE_RESULT_FAIL(x)
#endif

//! Helper macro, that makes it easy to add result checking to calls in functions/methods that themselves return Result. 
#define SLANG_RETURN_ON_FAIL(x) { SlangResult _res = (x); if (SLANG_FAILED(_res)) { SLANG_HANDLE_RESULT_FAIL(_res); return _res; } }
//! Helper macro that can be used to test the return value from a call, and will return in a void method/function
#define SLANG_RETURN_VOID_ON_FAIL(x) { SlangResult _res = (x); if (SLANG_FAILED(_res)) { SLANG_HANDLE_RESULT_FAIL(_res); return; } }
//! Helper macro that will return false on failure.
#define SLANG_RETURN_FALSE_ON_FAIL(x) { SlangResult _res = (x); if (SLANG_FAILED(_res)) { SLANG_HANDLE_RESULT_FAIL(_res); return false; } }
//! Helper macro that will return nullptr on failure.
#define SLANG_RETURN_NULL_ON_FAIL(x) { SlangResult _res = (x); if (SLANG_FAILED(_res)) { SLANG_HANDLE_RESULT_FAIL(_res); return nullptr; } }

//! Helper macro that will assert if the return code from a call is failure, also returns the failure.
#define SLANG_ASSERT_ON_FAIL(x) { SlangResult _res = (x); if (SLANG_FAILED(_res)) { assert(false); return _res; } }
//! Helper macro that will assert if the result from a call is a failure, also returns. 
#define SLANG_ASSERT_VOID_ON_FAIL(x) { SlangResult _res = (x); if (SLANG_FAILED(_res)) { assert(false); return; } }

/* !!!!!!!!!!!!!!!!!!!!!!! C++ helpers !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

#if defined(__cplusplus)
namespace Slang {

// Alias SlangResult to Slang::Result
typedef SlangResult Result;
// Alias SlangUUID to Slang::Guid
typedef SlangUUID Guid;

} // namespace Slang

// Operator == and != for Guid/SlangUUID

SLANG_FORCE_INLINE bool operator==(const Slang::Guid& aIn, const Slang::Guid& bIn)
{
    using namespace Slang;
    // Use the largest type the honors the alignment of Guid
    typedef uint32_t CmpType;
    union GuidCompare
    {
        Guid guid;
        CmpType data[sizeof(Guid) / sizeof(CmpType)];
    };
    // Type pun - so compiler can 'see' the pun and not break aliasing rules
    const CmpType* a = reinterpret_cast<const GuidCompare&>(aIn).data;
    const CmpType* b = reinterpret_cast<const GuidCompare&>(bIn).data;
    // Make the guid comparison a single branch, by not using short circuit
    return ((a[0] ^ b[0]) | (a[1] ^ b[1]) | (a[2] ^ b[2]) | (a[3] ^ b[3])) == 0;
}

SLANG_FORCE_INLINE bool operator!=(const Slang::Guid& a, const Slang::Guid& b)
{
    return !(a == b);
}

/* !!!!!!!! Macros to simplify implementing COM interfaces !!!!!!!!!!!!!!!!!!!!!!!!!!!! */

/* Assumes underlying implementation has a member m_refCount that is initialized to 0 and can have ++ and -- operate on it. 
For SLANG_IUNKNOWN_QUERY_INTERFACE to work - must have a method 'getInterface' that returns valid pointers for the Guid, or nullptr 
if not found. */

#define SLANG_IUNKNOWN_QUERY_INTERFACE \
SLANG_NO_THROW SlangResult SLANG_MCALL queryInterface(SlangUUID const& uuid, void** outObject) SLANG_OVERRIDE \
{ \
    ISlangUnknown* intf = getInterface(uuid); \
    if (intf) \
    { \
        addRef(); \
        *outObject = intf; \
        return SLANG_OK;\
    } \
    return SLANG_E_NO_INTERFACE;\
}

#define SLANG_IUNKNOWN_ADD_REF \
SLANG_NO_THROW uint32_t SLANG_MCALL addRef() \
{ \
    return ++m_refCount; \
}

#define SLANG_IUNKNOWN_RELEASE \
SLANG_NO_THROW uint32_t SLANG_MCALL release() \
{ \
    --m_refCount; \
    if (m_refCount == 0) \
    { \
        delete this; \
        return 0; \
    } \
    return m_refCount; \
} 

#define SLANG_IUNKNOWN_ALL \
    SLANG_IUNKNOWN_QUERY_INTERFACE \
    SLANG_IUNKNOWN_ADD_REF \
    SLANG_IUNKNOWN_RELEASE 

// ------------------------ RefObject IUnknown -----------------------------

#define SLANG_REF_OBJECT_IUNKNOWN_QUERY_INTERFACE \
SLANG_NO_THROW SlangResult SLANG_MCALL queryInterface(SlangUUID const& uuid, void** outObject) SLANG_OVERRIDE \
{ \
    void* intf = getInterface(uuid); \
    if (intf) \
    { \
        addReference(); \
        *outObject = intf; \
        return SLANG_OK;\
    } \
    return SLANG_E_NO_INTERFACE;\
}

#define SLANG_REF_OBJECT_IUNKNOWN_ADD_REF SLANG_NO_THROW uint32_t SLANG_MCALL addRef() SLANG_OVERRIDE { return (uint32_t)addReference(); }
#define SLANG_REF_OBJECT_IUNKNOWN_RELEASE SLANG_NO_THROW uint32_t SLANG_MCALL release() SLANG_OVERRIDE { return (uint32_t)releaseReference(); }

#    define SLANG_REF_OBJECT_IUNKNOWN_ALL         \
        SLANG_REF_OBJECT_IUNKNOWN_QUERY_INTERFACE \
        SLANG_REF_OBJECT_IUNKNOWN_ADD_REF         \
        SLANG_REF_OBJECT_IUNKNOWN_RELEASE

#endif // defined(__cplusplus)

#endif


#include <assert.h>
#include <cstddef>

namespace Slang {

/*! \brief ComPtr is a simple smart pointer that manages types which implement COM based interfaces.
\details A class that implements a COM, must derive from the IUnknown interface or a type that matches
it's layout exactly (such as ISlangUnknown). Trying to use this template with a class that doesn't follow
these rules, will lead to undefined behavior.
This is a 'strong' pointer type, and will AddRef when a non null pointer is set and Release when the pointer
leaves scope.
Using 'detach' allows a pointer to be removed from the management of the ComPtr.
To set the smart pointer to null, there is the method setNull, or alternatively just assign SLANG_NULL/nullptr.

One edge case using the template is that sometimes you want access as a pointer to a pointer. Sometimes this
is to write into the smart pointer, other times to pass as an array. To handle these different behaviors
there are the methods readRef and writeRef, which are used instead of the & (ref) operator. For example

\code
Void doSomething(ID3D12Resource** resources, IndexT numResources);
// ...
ComPtr<ID3D12Resource> resources[3];
doSomething(resources[0].readRef(), SLANG_COUNT_OF(resource));
\endcode

A more common scenario writing to the pointer

\code
IUnknown* unk = ...;

ComPtr<ID3D12Resource> resource;
Result res = unk->QueryInterface(resource.writeRef());
\endcode
*/

// Enum to force initializing as an attach (without adding a reference)
enum InitAttach
{
    INIT_ATTACH
};

template <class T>
class ComPtr
{
public:
	typedef T Type;
	typedef ComPtr ThisType;
	typedef ISlangUnknown* Ptr;

		/// Constructors
		/// Default Ctor. Sets to nullptr
	SLANG_FORCE_INLINE ComPtr() :m_ptr(nullptr) {}
    SLANG_FORCE_INLINE ComPtr(std::nullptr_t) : m_ptr(nullptr) {}
		/// Sets, and ref counts.
	SLANG_FORCE_INLINE explicit ComPtr(T* ptr) :m_ptr(ptr) { if (ptr) ((Ptr)ptr)->addRef(); }
		/// The copy ctor
	SLANG_FORCE_INLINE ComPtr(const ThisType& rhs) : m_ptr(rhs.m_ptr) { if (m_ptr) ((Ptr)m_ptr)->addRef(); }

        /// Ctor without adding to ref count.
    SLANG_FORCE_INLINE explicit ComPtr(InitAttach, T* ptr) :m_ptr(ptr) { }
        /// Ctor without adding to ref count
    SLANG_FORCE_INLINE ComPtr(InitAttach, const ThisType& rhs) : m_ptr(rhs.m_ptr) { }

#ifdef SLANG_HAS_MOVE_SEMANTICS
		/// Move Ctor
	SLANG_FORCE_INLINE ComPtr(ThisType&& rhs) : m_ptr(rhs.m_ptr) { rhs.m_ptr = nullptr; }
		/// Move assign
	SLANG_FORCE_INLINE ComPtr& operator=(ThisType&& rhs) { T* swap = m_ptr; m_ptr = rhs.m_ptr; rhs.m_ptr = swap; return *this; }
#endif

	/// Destructor releases the pointer, assuming it is set
	SLANG_FORCE_INLINE ~ComPtr() { if (m_ptr) ((Ptr)m_ptr)->release(); }

	// !!! Operators !!!

	  /// Returns the dumb pointer
	SLANG_FORCE_INLINE operator T *() const { return m_ptr; }

	SLANG_FORCE_INLINE T& operator*() { return *m_ptr; }
		/// For making method invocations through the smart pointer work through the dumb pointer
	SLANG_FORCE_INLINE T* operator->() const { return m_ptr; }

		/// Assign
	SLANG_FORCE_INLINE const ThisType &operator=(const ThisType& rhs);
		/// Assign from dumb ptr
	SLANG_FORCE_INLINE T* operator=(T* in);

		/// Get the pointer and don't ref
	SLANG_FORCE_INLINE T* get() const { return m_ptr; }
		/// Release a contained nullptr pointer if set
	SLANG_FORCE_INLINE void setNull();

		/// Detach
	SLANG_FORCE_INLINE T* detach() { T* ptr = m_ptr; m_ptr = nullptr; return ptr; }
		/// Set to a pointer without changing the ref count
	SLANG_FORCE_INLINE void attach(T* in) { m_ptr = in; }

		/// Get ready for writing (nulls contents)
	SLANG_FORCE_INLINE T** writeRef() { setNull(); return &m_ptr; }
		/// Get for read access
	SLANG_FORCE_INLINE T*const* readRef() const { return &m_ptr; }

		/// Swap
	void swap(ThisType& rhs);

protected:
	/// Gets the address of the dumb pointer.
    // Disabled: use writeRef and readRef to get a reference based on usage.
	SLANG_FORCE_INLINE T** operator&() = delete;

	T* m_ptr;
};

//----------------------------------------------------------------------------
template <typename T>
void ComPtr<T>::setNull()
{
	if (m_ptr)
	{
		((Ptr)m_ptr)->release();
		m_ptr = nullptr;
	}
}
//----------------------------------------------------------------------------
template <typename T>
const ComPtr<T>& ComPtr<T>::operator=(const ThisType& rhs)
{
	if (rhs.m_ptr) ((Ptr)rhs.m_ptr)->addRef();
	if (m_ptr) ((Ptr)m_ptr)->release();
	m_ptr = rhs.m_ptr;
	return *this;
}
//----------------------------------------------------------------------------
template <typename T>
T* ComPtr<T>::operator=(T* ptr)
{
	if (ptr) ((Ptr)ptr)->addRef();
	if (m_ptr) ((Ptr)m_ptr)->release();
	m_ptr = ptr;
	return m_ptr;
}
//----------------------------------------------------------------------------
template <typename T>
void ComPtr<T>::swap(ThisType& rhs)
{
	T* tmp = m_ptr;
	m_ptr = rhs.m_ptr;
	rhs.m_ptr = tmp;
}

} // namespace Slang

#endif // SLANG_COM_PTR_H

// render.h
#pragma once

#include <float.h>
#include <assert.h>



#if defined(SLANG_GFX_DYNAMIC)
#    if defined(_MSC_VER)
#        ifdef SLANG_GFX_DYNAMIC_EXPORT
#            define SLANG_GFX_API SLANG_DLL_EXPORT
#        else
#            define SLANG_GFX_API __declspec(dllimport)
#        endif
#    else
// TODO: need to consider compiler capabilities
//#     ifdef SLANG_DYNAMIC_EXPORT
#        define SLANG_GFX_API SLANG_DLL_EXPORT
//#     endif
#    endif
#endif

#ifndef SLANG_GFX_API
#    define SLANG_GFX_API
#endif

// Needed for building on cygwin with gcc
#undef Always
#undef None

namespace gfx {

using Slang::ComPtr;

typedef SlangResult Result;

// Had to move here, because Options needs types defined here
typedef SlangInt Int;
typedef SlangUInt UInt;
typedef uint64_t DeviceAddress;

const uint64_t kTimeoutInfinite = 0xFFFFFFFFFFFFFFFF;

enum class StructType
{
    D3D12ExtendedDesc,
};

enum class StageType
{
    Unknown,
    Vertex,
    Hull,
    Domain,
    Geometry,
    Fragment,
    Compute,
    RayGeneration,
    Intersection,
    AnyHit,
    ClosestHit,
    Miss,
    Callable,
    Amplification,
    Mesh,
    CountOf,
};

enum class DeviceType
{
    Unknown,
    Default,
    DirectX11,
    DirectX12,
    OpenGl,
    Vulkan,
    CPU,
    CUDA,
    CountOf,
};

enum class ProjectionStyle
{
    Unknown,
    OpenGl, 
    DirectX,
    Vulkan,
    CountOf,
};

/// The style of the binding
enum class BindingStyle
{
    Unknown,
    DirectX,
    OpenGl,
    Vulkan,
    CPU,
    CUDA,
    CountOf,
};

enum class AccessFlag
{
    None,
    Read,
    Write,
};

const uint32_t kMaxRenderTargetCount = 8;

class ITransientResourceHeap;

class IShaderProgram: public ISlangUnknown
{
public:
    // Defines how linking should be performed for a shader program.
    enum class LinkingStyle
    {
        // Compose all entry-points in a single program, then compile all entry-points together with the same
        // set of root shader arguments.
        SingleProgram,

        // Link and compile each entry-point individually, potentially with different specializations.
        SeparateEntryPointCompilation
    };

    struct Desc
    {
        // The linking style of this program.
        LinkingStyle linkingStyle = LinkingStyle::SingleProgram;

        // The global scope or a Slang composite component that represents the entire program.
        slang::IComponentType*  slangGlobalScope;

        // Number of separate entry point components in the `slangEntryPoints` array to link in.
        // If set to 0, then `slangGlobalScope` must contain Slang EntryPoint components.
        // If not 0, then `slangGlobalScope` must not contain any EntryPoint components.
        uint32_t entryPointCount = 0;

        // An array of Slang entry points. The size of the array must be `entryPointCount`.
        // Each element must define only 1 Slang EntryPoint.
        slang::IComponentType** slangEntryPoints = nullptr;
    };
};
#define SLANG_UUID_IShaderProgram                                                       \
    {                                                                                  \
        0x9d32d0ad, 0x915c, 0x4ffd, { 0x91, 0xe2, 0x50, 0x85, 0x54, 0xa0, 0x4a, 0x76 } \
    }

// Dont' change without keeping in sync with Format
#define GFX_FORMAT(x) \
    x( Unknown, 0, 0) \
    \
    x(R32G32B32A32_TYPELESS, 16, 1) \
    x(R32G32B32_TYPELESS, 12, 1) \
    x(R32G32_TYPELESS, 8, 1) \
    x(R32_TYPELESS, 4, 1) \
    \
    x(R16G16B16A16_TYPELESS, 8, 1) \
    x(R16G16_TYPELESS, 4, 1) \
    x(R16_TYPELESS, 2, 1) \
    \
    x(R8G8B8A8_TYPELESS, 4, 1) \
    x(R8G8_TYPELESS, 2, 1) \
    x(R8_TYPELESS, 1, 1) \
    x(B8G8R8A8_TYPELESS, 4, 1) \
    \
    x(R32G32B32A32_FLOAT, 16, 1) \
    x(R32G32B32_FLOAT, 12, 1) \
    x(R32G32_FLOAT, 8, 1) \
    x(R32_FLOAT, 4, 1) \
    \
    x(R16G16B16A16_FLOAT, 8, 1) \
    x(R16G16_FLOAT, 4, 1) \
    x(R16_FLOAT, 2, 1) \
    \
    x(R32G32B32A32_UINT, 16, 1) \
    x(R32G32B32_UINT, 12, 1) \
    x(R32G32_UINT, 8, 1) \
    x(R32_UINT, 4, 1) \
    \
    x(R16G16B16A16_UINT, 8, 1) \
    x(R16G16_UINT, 4, 1) \
    x(R16_UINT, 2, 1) \
    \
    x(R8G8B8A8_UINT, 4, 1) \
    x(R8G8_UINT, 2, 1) \
    x(R8_UINT, 1, 1) \
    \
    x(R32G32B32A32_SINT, 16, 1) \
    x(R32G32B32_SINT, 12, 1) \
    x(R32G32_SINT, 8, 1) \
    x(R32_SINT, 4, 1) \
    \
    x(R16G16B16A16_SINT, 8, 1) \
    x(R16G16_SINT, 4, 1) \
    x(R16_SINT, 2, 1) \
    \
    x(R8G8B8A8_SINT, 4, 1) \
    x(R8G8_SINT, 2, 1) \
    x(R8_SINT, 1, 1) \
    \
    x(R16G16B16A16_UNORM, 8, 1) \
    x(R16G16_UNORM, 4, 1) \
    x(R16_UNORM, 2, 1) \
    \
    x(R8G8B8A8_UNORM, 4, 1) \
    x(R8G8B8A8_UNORM_SRGB, 4, 1) \
    x(R8G8_UNORM, 2, 1) \
    x(R8_UNORM, 1, 1) \
    x(B8G8R8A8_UNORM, 4, 1) \
    x(B8G8R8A8_UNORM_SRGB, 4, 1) \
    x(B8G8R8X8_UNORM, 4, 1) \
    x(B8G8R8X8_UNORM_SRGB, 4, 1) \
    \
    x(R16G16B16A16_SNORM, 8, 1) \
    x(R16G16_SNORM, 4, 1) \
    x(R16_SNORM, 2, 1) \
    \
    x(R8G8B8A8_SNORM, 4, 1) \
    x(R8G8_SNORM, 2, 1) \
    x(R8_SNORM, 1, 1) \
    \
    x(D32_FLOAT, 4, 1) \
    x(D16_UNORM, 2, 1) \
    \
    x(B4G4R4A4_UNORM, 2, 1) \
    x(B5G6R5_UNORM, 2, 1) \
    x(B5G5R5A1_UNORM, 2, 1) \
    \
    x(R9G9B9E5_SHAREDEXP, 4, 1) \
    x(R10G10B10A2_TYPELESS, 4, 1) \
    x(R10G10B10A2_UNORM, 4, 1) \
    x(R10G10B10A2_UINT, 4, 1) \
    x(R11G11B10_FLOAT, 4, 1) \
    \
    x(BC1_UNORM, 8, 16) \
    x(BC1_UNORM_SRGB, 8, 16) \
    x(BC2_UNORM, 16, 16) \
    x(BC2_UNORM_SRGB, 16, 16) \
    x(BC3_UNORM, 16, 16) \
    x(BC3_UNORM_SRGB, 16, 16) \
    x(BC4_UNORM, 8, 16) \
    x(BC4_SNORM, 8, 16) \
    x(BC5_UNORM, 16, 16) \
    x(BC5_SNORM, 16, 16) \
    x(BC6H_UF16, 16, 16) \
    x(BC6H_SF16, 16, 16) \
    x(BC7_UNORM, 16, 16) \
    x(BC7_UNORM_SRGB, 16, 16)

/// Different formats of things like pixels or elements of vertices
/// NOTE! Any change to this type (adding, removing, changing order) - must also be reflected in changes GFX_FORMAT
enum class Format
{
    // D3D formats omitted: 19-22, 44-47, 65-66, 68-70, 73, 76, 79, 82, 88-89, 92-94, 97, 100-114
    // These formats are omitted due to lack of a corresponding Vulkan format. D24_UNORM_S8_UINT (DXGI_FORMAT 45)
    // has a matching Vulkan format but is also omitted as it is only supported by Nvidia.
    Unknown,

    R32G32B32A32_TYPELESS,
    R32G32B32_TYPELESS,
    R32G32_TYPELESS,
    R32_TYPELESS,

    R16G16B16A16_TYPELESS,
    R16G16_TYPELESS,
    R16_TYPELESS,

    R8G8B8A8_TYPELESS,
    R8G8_TYPELESS,
    R8_TYPELESS,
    B8G8R8A8_TYPELESS,

    R32G32B32A32_FLOAT,
    R32G32B32_FLOAT,
    R32G32_FLOAT,
    R32_FLOAT,

    R16G16B16A16_FLOAT,
    R16G16_FLOAT,
    R16_FLOAT,

    R32G32B32A32_UINT,
    R32G32B32_UINT,
    R32G32_UINT,
    R32_UINT,

    R16G16B16A16_UINT,
    R16G16_UINT,
    R16_UINT,

    R8G8B8A8_UINT,
    R8G8_UINT,
    R8_UINT,

    R32G32B32A32_SINT,
    R32G32B32_SINT,
    R32G32_SINT,
    R32_SINT,

    R16G16B16A16_SINT,
    R16G16_SINT,
    R16_SINT,

    R8G8B8A8_SINT,
    R8G8_SINT,
    R8_SINT,

    R16G16B16A16_UNORM,
    R16G16_UNORM,
    R16_UNORM,

    R8G8B8A8_UNORM,
    R8G8B8A8_UNORM_SRGB,
    R8G8_UNORM,
    R8_UNORM,
    B8G8R8A8_UNORM,
    B8G8R8A8_UNORM_SRGB,
    B8G8R8X8_UNORM,
    B8G8R8X8_UNORM_SRGB,

    R16G16B16A16_SNORM,
    R16G16_SNORM,
    R16_SNORM,

    R8G8B8A8_SNORM,
    R8G8_SNORM,
    R8_SNORM,

    D32_FLOAT,
    D16_UNORM,

    B4G4R4A4_UNORM,
    B5G6R5_UNORM,
    B5G5R5A1_UNORM,

    R9G9B9E5_SHAREDEXP,
    R10G10B10A2_TYPELESS,
    R10G10B10A2_UNORM,
    R10G10B10A2_UINT,
    R11G11B10_FLOAT,

    BC1_UNORM,
    BC1_UNORM_SRGB,
    BC2_UNORM,
    BC2_UNORM_SRGB,
    BC3_UNORM,
    BC3_UNORM_SRGB,
    BC4_UNORM,
    BC4_SNORM,
    BC5_UNORM,
    BC5_SNORM,
    BC6H_UF16,
    BC6H_SF16,
    BC7_UNORM,
    BC7_UNORM_SRGB,

    CountOf,
};

struct FormatInfo
{
    uint8_t channelCount;          ///< The amount of channels in the format. Only set if the channelType is set 
    uint8_t channelType;           ///< One of SlangScalarType None if type isn't made up of elements of type.

    uint32_t blockSizeInBytes;     ///< The size of a block in bytes.
    uint32_t pixelsPerBlock;       ///< The number of pixels contained in a block.
    uint32_t blockWidth;
    uint32_t blockHeight;
};

enum class InputSlotClass
{
    PerVertex, PerInstance
};

struct InputElementDesc
{
    char const* semanticName;
    UInt semanticIndex;
    Format format;
    UInt offset;
    UInt bufferSlotIndex;
};

struct VertexStreamDesc
{
    uint32_t stride;
    InputSlotClass slotClass;
    UInt instanceDataStepRate;
};

enum class PrimitiveType
{
    Point, Line, Triangle, Patch
};

enum class PrimitiveTopology
{
    TriangleList, TriangleStrip, PointList, LineList, LineStrip
};

enum class ResourceState
{
    Undefined,
    General,
    PreInitialized,
    VertexBuffer,
    IndexBuffer,
    ConstantBuffer,
    StreamOutput,
    ShaderResource,
    UnorderedAccess,
    RenderTarget,
    DepthRead,
    DepthWrite,
    Present,
    IndirectArgument,
    CopySource,
    CopyDestination,
    ResolveSource,
    ResolveDestination,
    AccelerationStructure,
    AccelerationStructureBuildInput,
    _Count
};

struct ResourceStateSet
{
public:
    void add(ResourceState state) { m_bitFields |= (1LL << (uint32_t)state); }
    template <typename... TResourceState> void add(ResourceState s, TResourceState... states)
    {
        add(s);
        add(states...);
    }
    bool contains(ResourceState state) const { return (m_bitFields & (1LL << (uint32_t)state)) != 0; }
    ResourceStateSet()
        : m_bitFields(0)
    {}
    ResourceStateSet(const ResourceStateSet& other) = default;
    ResourceStateSet(ResourceState state) { add(state); }
    template <typename... TResourceState> ResourceStateSet(TResourceState... states)
    {
        add(states...);
    }

    ResourceStateSet operator&(const ResourceStateSet& that) const
    {
        ResourceStateSet result;
        result.m_bitFields = this->m_bitFields & that.m_bitFields;
        return result;
    }

private:
    uint64_t m_bitFields = 0;
    void add() {}
};


/// Describes how memory for the resource should be allocated for CPU access.
enum class MemoryType
{
    DeviceLocal,
    Upload,
    ReadBack,
};

enum class InteropHandleAPI
{
    Unknown,
    D3D12, // A D3D12 object pointer.
    Vulkan, // A general Vulkan object handle.
    CUDA, // A general CUDA object handle.
    Win32, // A general Win32 HANDLE.
    FileDescriptor, // A file descriptor.
    DeviceAddress, // A device address.
    D3D12CpuDescriptorHandle, // A D3D12_CPU_DESCRIPTOR_HANDLE value.
};

struct InteropHandle
{
    InteropHandleAPI api = InteropHandleAPI::Unknown;
    uint64_t handleValue = 0;
};

// Declare opaque type
class IInputLayout : public ISlangUnknown
{
public:
    struct Desc
    {
        InputElementDesc const* inputElements = nullptr;
        Int inputElementCount = 0;
        VertexStreamDesc const* vertexStreams = nullptr;
        Int vertexStreamCount = 0;
    };
};
#define SLANG_UUID_IInputLayout                                                         \
    {                                                                                  \
        0x45223711, 0xa84b, 0x455c, { 0xbe, 0xfa, 0x49, 0x37, 0x42, 0x1e, 0x8e, 0x2e } \
    }

class IResource: public ISlangUnknown
{
public:
        /// The type of resource.
        /// NOTE! The order needs to be such that all texture types are at or after Texture1D (otherwise isTexture won't work correctly)
    enum class Type
    {
        Unknown,            ///< Unknown
        Buffer,             ///< A buffer (like a constant/index/vertex buffer)
        Texture1D,          ///< A 1d texture
        Texture2D,          ///< A 2d texture
        Texture3D,          ///< A 3d texture
        TextureCube,        ///< A cubemap consists of 6 Texture2D like faces
        CountOf,
    };

        /// Base class for Descs
    struct DescBase
    {
        Type type = Type::Unknown;
        ResourceState defaultState = ResourceState::Undefined;
        ResourceStateSet allowedStates = ResourceStateSet();
        MemoryType memoryType = MemoryType::DeviceLocal;
        InteropHandle existingHandle = {};
        bool isShared = false;
    };

    virtual SLANG_NO_THROW Type SLANG_MCALL getType() = 0;
    virtual SLANG_NO_THROW Result SLANG_MCALL getNativeResourceHandle(InteropHandle* outHandle) = 0;
    virtual SLANG_NO_THROW Result SLANG_MCALL getSharedHandle(InteropHandle* outHandle) = 0;
	
    virtual SLANG_NO_THROW Result SLANG_MCALL setDebugName(const char* name) = 0;
    virtual SLANG_NO_THROW const char* SLANG_MCALL getDebugName() = 0;

};
#define SLANG_UUID_IResource                                                           \
    {                                                                                  \
        0xa0e39f34, 0x8398, 0x4522, { 0x95, 0xc2, 0xeb, 0xc0, 0xf9, 0x84, 0xef, 0x3f } \
    }

struct MemoryRange
{
    uint64_t offset;
    uint64_t size;
};

class IBufferResource: public IResource
{
public:
    struct Desc: public DescBase
    {
        size_t sizeInBytes = 0;     ///< Total size in bytes
        int elementSize = 0;        ///< Get the element stride. If > 0, this is a structured buffer
        Format format = Format::Unknown;
    };

    virtual SLANG_NO_THROW Desc* SLANG_MCALL getDesc() = 0;
    virtual SLANG_NO_THROW DeviceAddress SLANG_MCALL getDeviceAddress() = 0;
    virtual SLANG_NO_THROW Result SLANG_MCALL map(MemoryRange* rangeToRead, void** outPointer) = 0;
    virtual SLANG_NO_THROW Result SLANG_MCALL unmap(MemoryRange* writtenRange) = 0;
};
#define SLANG_UUID_IBufferResource                                                     \
    {                                                                                  \
        0x1b274efe, 0x5e37, 0x492b, { 0x82, 0x6e, 0x7e, 0xe7, 0xe8, 0xf5, 0xa4, 0x9b } \
    }

struct DepthStencilClearValue
{
    float depth = 1.0f;
    uint32_t stencil = 0;
};
union ColorClearValue
{
    float floatValues[4];
    uint32_t uintValues[4];
};
struct ClearValue
{
    ColorClearValue color = {{0.0f, 0.0f, 0.0f, 0.0f}};
    DepthStencilClearValue depthStencil;
};

struct BufferRange
{
    uint64_t firstElement;
    uint64_t elementCount;
};

enum class TextureAspect : uint32_t
{
    Default = 0,
    Color = 0x00000001,
    Depth = 0x00000002,
    Stencil = 0x00000004,
    MetaData = 0x00000008,
    Plane0 = 0x00000010,
    Plane1 = 0x00000020,
    Plane2 = 0x00000040,

    DepthStencil = Depth | Stencil,
};

struct SubresourceRange
{
    TextureAspect aspectMask;
    uint32_t mipLevel;
    uint32_t mipLevelCount;
    uint32_t baseArrayLayer; // For Texture3D, this is WSlice.
    uint32_t layerCount; // For cube maps, this is a multiple of 6.
};

class ITextureResource: public IResource
{
public:
    static const uint32_t kRemainingTextureSize = 0xFFFFFFFF;
    struct Offset3D
    {
        uint32_t x = 0;
        uint32_t y = 0;
        uint32_t z = 0;
        Offset3D() = default;
        Offset3D(uint32_t _x, uint32_t _y, uint32_t _z) :x(_x), y(_y), z(_z) {}
    };

    struct SampleDesc
    {
        int numSamples = 1;                     ///< Number of samples per pixel
        int quality = 0;                        ///< The quality measure for the samples
    };

    struct Size
    {
        int width = 0;              ///< Width in pixels
        int height = 0;             ///< Height in pixels (if 2d or 3d)
        int depth = 0;              ///< Depth (if 3d)
    };

    struct Desc: public DescBase
    {
        Size size;

        int arraySize = 0;          ///< Array size

        int numMipLevels = 0;       ///< Number of mip levels - if 0 will create all mip levels
        Format format;              ///< The resources format
        SampleDesc sampleDesc;      ///< How the resource is sampled
        ClearValue optimalClearValue;
    };

        /// Data for a single subresource of a texture.
        ///
        /// Each subresource is a tensor with `1 <= rank <= 3`,
        /// where the rank is deterined by the base shape of the
        /// texture (Buffer, 1D, 2D, 3D, or Cube). For the common
        /// case of a 2D texture, `rank == 2` and each subresource
        /// is a 2D image.
        ///
        /// Subresource tensors must be stored in a row-major layout,
        /// so that the X axis strides over texels, the Y axis strides
        /// over 1D rows of texels, and the Z axis strides over 2D
        /// "layers" of texels.
        ///
        /// For a texture with multiple mip levels or array elements,
        /// each mip level and array element is stores as a distinct
        /// subresource. When indexing into an array of subresources,
        /// the index of a subresoruce for mip level `m` and array
        /// index `a` is `m + a*mipLevelCount`.
        ///
    struct SubresourceData
    {
            /// Pointer to texel data for the subresource tensor.
        void const* data;

            /// Stride in bytes between rows of the subresource tensor.
            ///
            /// This is the number of bytes to add to a pointer to a texel
            /// at (X,Y,Z) to get to a texel at (X,Y+1,Z).
            ///
            /// Devices may not support all possible values for `strideY`.
            /// In particular, they may only support strictly positive strides.
            ///
        int64_t     strideY;

            /// Stride in bytes between layers of the subresource tensor.
            ///
            /// This is the number of bytes to add to a pointer to a texel
            /// at (X,Y,Z) to get to a texel at (X,Y,Z+1).
            ///
            /// Devices may not support all possible values for `strideZ`.
            /// In particular, they may only support strictly positive strides.
            ///
        int64_t     strideZ;
    };

    virtual SLANG_NO_THROW Desc* SLANG_MCALL getDesc() = 0;
};
#define SLANG_UUID_ITextureResource                                                    \
    {                                                                                  \
        0xcf88a31c, 0x6187, 0x46c5, { 0xa4, 0xb7, 0xeb, 0x58, 0xc7, 0x33, 0x40, 0x17 } \
    }


enum class ComparisonFunc : uint8_t
{
    Never           = 0x0,
    Less            = 0x1,
    Equal           = 0x2,
    LessEqual       = 0x3,
    Greater         = 0x4,
    NotEqual        = 0x5,
    GreaterEqual    = 0x6,
    Always          = 0x7,
};

enum class TextureFilteringMode
{
    Point,
    Linear,
};

enum class TextureAddressingMode
{
    Wrap,
    ClampToEdge,
    ClampToBorder,
    MirrorRepeat,
    MirrorOnce,
};

enum class TextureReductionOp
{
    Average,
    Comparison,
    Minimum,
    Maximum,
};

class ISamplerState : public ISlangUnknown
{
public:
    struct Desc
    {
        TextureFilteringMode    minFilter       = TextureFilteringMode::Linear;
        TextureFilteringMode    magFilter       = TextureFilteringMode::Linear;
        TextureFilteringMode    mipFilter       = TextureFilteringMode::Linear;
        TextureReductionOp      reductionOp     = TextureReductionOp::Average;
        TextureAddressingMode   addressU        = TextureAddressingMode::Wrap;
        TextureAddressingMode   addressV        = TextureAddressingMode::Wrap;
        TextureAddressingMode   addressW        = TextureAddressingMode::Wrap;
        float                   mipLODBias      = 0.0f;
        uint32_t                maxAnisotropy   = 1;
        ComparisonFunc          comparisonFunc  = ComparisonFunc::Never;
        float                   borderColor[4]  = { 1.0f, 1.0f, 1.0f, 1.0f };
        float                   minLOD          = -FLT_MAX;
        float                   maxLOD          = FLT_MAX;
    };

    /// Returns a native API handle representing this sampler state object.
    /// When using D3D12, this will be a D3D12_CPU_DESCRIPTOR_HANDLE.
    /// When using Vulkan, this will be a VkSampler.
    virtual SLANG_NO_THROW Result SLANG_MCALL getNativeHandle(InteropHandle* outNativeHandle) = 0;
};
#define SLANG_UUID_ISamplerState                                                        \
    {                                                                                  \
        0x8b8055df, 0x9377, 0x401d, { 0x91, 0xff, 0x3f, 0xa3, 0xbf, 0x66, 0x64, 0xf4 } \
    }

class IResourceView : public ISlangUnknown
{
public:
    enum class Type
    {
        Unknown,

        RenderTarget,
        DepthStencil,
        ShaderResource,
        UnorderedAccess,
        AccelerationStructure,
    };

    struct RenderTargetDesc
    {
        // The resource shape of this render target view.
        IResource::Type shape;
    };

    struct Desc
    {
        Type    type;
        Format  format;

        // Required fields for `RenderTarget` and `DepthStencil` views.
        RenderTargetDesc renderTarget;
        // Specifies the range of a texture resource for a ShaderRsource/UnorderedAccess/RenderTarget/DepthStencil view.
        SubresourceRange subresourceRange;
        // Specifies the range of a buffer resource for a ShaderResource/UnorderedAccess view.
        BufferRange bufferRange;
        // Specifies the element size of a structured buffer. Pass 0 for a raw buffer view.
        uint32_t bufferElementSize;
    };
    virtual SLANG_NO_THROW Desc* SLANG_MCALL getViewDesc() = 0;

    /// Returns a native API handle representing this resource view object.
    /// When using D3D12, this will be a D3D12_CPU_DESCRIPTOR_HANDLE or a buffer device address depending
    /// on the type of the resource view.
    /// When using Vulkan, this will be a VkImageView, VkBufferView, VkAccelerationStructure or a VkBuffer
    /// depending on the type of the resource view.
    virtual SLANG_NO_THROW Result SLANG_MCALL getNativeHandle(InteropHandle* outNativeHandle) = 0;
};
#define SLANG_UUID_IResourceView                                                      \
    {                                                                                 \
        0x7b6c4926, 0x884, 0x408c, { 0xad, 0x8a, 0x50, 0x3a, 0x8e, 0x23, 0x98, 0xa4 } \
    }

class IAccelerationStructure : public IResourceView
{
public:
    enum class Kind
    {
        TopLevel,
        BottomLevel
    };

    struct BuildFlags
    {
        // The enum values are intentionally consistent with
        // D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS.
        enum Enum
        {
            None,
            AllowUpdate = 1,
            AllowCompaction = 2,
            PreferFastTrace = 4,
            PreferFastBuild = 8,
            MinimizeMemory = 16,
            PerformUpdate = 32
        };
    };

    enum class GeometryType
    {
        Triangles, ProcedurePrimitives
    };

    struct GeometryFlags
    {
        // The enum values are intentionally consistent with
        // D3D12_RAYTRACING_GEOMETRY_FLAGS.
        enum Enum
        {
            None,
            Opaque = 1,
            NoDuplicateAnyHitInvocation = 2
        };
    };

    struct TriangleDesc
    {
        DeviceAddress transform3x4;
        Format indexFormat;
        Format vertexFormat;
        uint32_t indexCount;
        uint32_t vertexCount;
        DeviceAddress indexData;
        DeviceAddress vertexData;
        uint64_t vertexStride;
    };

    struct ProceduralAABB
    {
        float minX;
        float minY;
        float minZ;
        float maxX;
        float maxY;
        float maxZ;
    };

    struct ProceduralAABBDesc
    {
        /// Number of AABBs.
        uint64_t count;

        /// Pointer to an array of `ProceduralAABB` values in device memory.
        DeviceAddress data;

        /// Stride in bytes of the AABB values array.
        uint64_t stride;
    };

    struct GeometryDesc
    {
        GeometryType type;
        GeometryFlags::Enum flags;
        union
        {
            TriangleDesc triangles;
            ProceduralAABBDesc proceduralAABBs;
        } content;
    };

    struct GeometryInstanceFlags
    {
        // The enum values are kept consistent with D3D12_RAYTRACING_INSTANCE_FLAGS
        // and VkGeometryInstanceFlagBitsKHR.
        enum Enum : uint32_t
        {
            None = 0,
            TriangleFacingCullDisable = 0x00000001,
            TriangleFrontCounterClockwise = 0x00000002,
            ForceOpaque = 0x00000004,
            NoOpaque = 0x00000008
        };
    };

    // The layout of this struct is intentionally consistent with D3D12_RAYTRACING_INSTANCE_DESC
    // and VkAccelerationStructureInstanceKHR.
    struct InstanceDesc
    {
        float transform[3][4];
        uint32_t instanceID : 24;
        uint32_t instanceMask : 8;
        uint32_t instanceContributionToHitGroupIndex : 24;
        uint32_t flags : 8; // Combination of GeometryInstanceFlags::Enum values.
        DeviceAddress accelerationStructure;
    };

    struct PrebuildInfo
    {
        uint64_t resultDataMaxSize;
        uint64_t scratchDataSize;
        uint64_t updateScratchDataSize;
    };

    struct BuildInputs
    {
        Kind kind;

        BuildFlags::Enum flags;

        int32_t descCount;

        /// Array of `InstanceDesc` values in device memory.
        /// Used when `kind` is `TopLevel`.
        DeviceAddress instanceDescs;

        /// Array of `GeometryDesc` values.
        /// Used when `kind` is `BottomLevel`.
        const GeometryDesc* geometryDescs;
    };

    struct CreateDesc
    {
        Kind kind;
        IBufferResource* buffer;
        uint64_t offset;
        uint64_t size;
    };

    struct BuildDesc
    {
        BuildInputs inputs;
        IAccelerationStructure* source;
        IAccelerationStructure* dest;
        DeviceAddress scratchData;
    };

    virtual SLANG_NO_THROW DeviceAddress SLANG_MCALL getDeviceAddress() = 0;
};
#define SLANG_UUID_IAccelerationStructure                                             \
    {                                                                                 \
        0xa5cdda3c, 0x1d4e, 0x4df7, { 0x8e, 0xf2, 0xb7, 0x3f, 0xce, 0x4, 0xde, 0x3b } \
    }

class IFence : public ISlangUnknown
{
public:
    struct Desc
    {
        uint64_t initialValue = 0;
        bool isShared = false;
    };

    /// Returns the currently signaled value on the device.
    virtual SLANG_NO_THROW Result SLANG_MCALL getCurrentValue(uint64_t* outValue) = 0;

    /// Signals the fence from the host with the specified value.
    virtual SLANG_NO_THROW Result SLANG_MCALL setCurrentValue(uint64_t value) = 0;

    virtual SLANG_NO_THROW Result SLANG_MCALL getSharedHandle(InteropHandle* outHandle) = 0;
    virtual SLANG_NO_THROW Result SLANG_MCALL getNativeHandle(InteropHandle* outNativeHandle) = 0;
};
#define SLANG_UUID_IFence                                                             \
    {                                                                                 \
        0x7fe1c283, 0xd3f4, 0x48ed, { 0xaa, 0xf3, 0x1, 0x51, 0x96, 0x4e, 0x7c, 0xb5 } \
    }

struct ShaderOffset
{
    SlangInt uniformOffset = 0;
    SlangInt bindingRangeIndex = 0;
    SlangInt bindingArrayIndex = 0;
    uint32_t getHashCode() const
    {
        return (uint32_t)(((bindingRangeIndex << 20) + bindingArrayIndex) ^ uniformOffset);
    }
    bool operator==(const ShaderOffset& other) const
    {
        return uniformOffset == other.uniformOffset
            && bindingRangeIndex == other.bindingRangeIndex
            && bindingArrayIndex == other.bindingArrayIndex;
    }
    bool operator!=(const ShaderOffset& other) const
    {
        return !this->operator==(other);
    }
    bool operator<(const ShaderOffset& other) const
    {
        if (bindingRangeIndex < other.bindingRangeIndex)
            return true;
        if (bindingRangeIndex > other.bindingRangeIndex)
            return false;
        if (bindingArrayIndex < other.bindingArrayIndex)
            return true;
        if (bindingArrayIndex > other.bindingArrayIndex)
            return false;
        return uniformOffset < other.uniformOffset;
    }
    bool operator<=(const ShaderOffset& other) const { return (*this == other) || (*this) < other; }
    bool operator>(const ShaderOffset& other) const { return other < *this; }
    bool operator>=(const ShaderOffset& other) const { return other <= *this; }
};

enum class ShaderObjectContainerType
{
    None, Array, StructuredBuffer
};

class IShaderObject : public ISlangUnknown
{
public:
    SLANG_NO_THROW ComPtr<IShaderObject> SLANG_MCALL getObject(ShaderOffset const& offset)
    {
        ComPtr<IShaderObject> object = nullptr;
        SLANG_RETURN_NULL_ON_FAIL(getObject(offset, object.writeRef()));
        return object;
    }

    virtual SLANG_NO_THROW slang::TypeLayoutReflection* SLANG_MCALL getElementTypeLayout() = 0;
    virtual SLANG_NO_THROW ShaderObjectContainerType SLANG_MCALL getContainerType() = 0;
    virtual SLANG_NO_THROW UInt SLANG_MCALL getEntryPointCount() = 0;

    ComPtr<IShaderObject> getEntryPoint(UInt index)
    {
        ComPtr<IShaderObject> entryPoint = nullptr;
        SLANG_RETURN_NULL_ON_FAIL(getEntryPoint(index, entryPoint.writeRef()));
        return entryPoint;
    }
    virtual SLANG_NO_THROW Result SLANG_MCALL
        getEntryPoint(UInt index, IShaderObject** entryPoint) = 0;
    virtual SLANG_NO_THROW Result SLANG_MCALL
        setData(ShaderOffset const& offset, void const* data, size_t size) = 0;
    virtual SLANG_NO_THROW Result SLANG_MCALL
        getObject(ShaderOffset const& offset, IShaderObject** object) = 0;
    virtual SLANG_NO_THROW Result SLANG_MCALL
        setObject(ShaderOffset const& offset, IShaderObject* object) = 0;
    virtual SLANG_NO_THROW Result SLANG_MCALL
        setResource(ShaderOffset const& offset, IResourceView* resourceView) = 0;
    virtual SLANG_NO_THROW Result SLANG_MCALL
        setSampler(ShaderOffset const& offset, ISamplerState* sampler) = 0;
    virtual SLANG_NO_THROW Result SLANG_MCALL setCombinedTextureSampler(
        ShaderOffset const& offset, IResourceView* textureView, ISamplerState* sampler) = 0;

        /// Manually overrides the specialization argument for the sub-object binding at `offset`.
        /// Specialization arguments are passed to the shader compiler to specialize the type
        /// of interface-typed shader parameters.
    virtual SLANG_NO_THROW Result SLANG_MCALL setSpecializationArgs(
        ShaderOffset const& offset,
        const slang::SpecializationArg* args,
        uint32_t count) = 0;

    virtual SLANG_NO_THROW Result SLANG_MCALL getCurrentVersion(
        ITransientResourceHeap* transientHeap,
        IShaderObject** outObject) = 0;

    virtual SLANG_NO_THROW const void* SLANG_MCALL getRawData() = 0;

    virtual SLANG_NO_THROW size_t SLANG_MCALL getSize() = 0;

        /// Use the provided constant buffer instead of the internally created one.
    virtual SLANG_NO_THROW Result SLANG_MCALL setConstantBufferOverride(IBufferResource* constantBuffer) = 0;
};
#define SLANG_UUID_IShaderObject                                                       \
    {                                                                                 \
        0xc1fa997e, 0x5ca2, 0x45ae, { 0x9b, 0xcb, 0xc4, 0x35, 0x9e, 0x85, 0x5, 0x85 } \
    }

enum class StencilOp : uint8_t
{
    Keep,
    Zero,
    Replace,
    IncrementSaturate,
    DecrementSaturate,
    Invert,
    IncrementWrap,
    DecrementWrap,
};

enum class FillMode : uint8_t
{
    Solid,
    Wireframe,
};

enum class CullMode : uint8_t
{
    None,
    Front,
    Back,
};

enum class FrontFaceMode : uint8_t
{
    CounterClockwise,
    Clockwise,
};

struct DepthStencilOpDesc
{
    StencilOp       stencilFailOp       = StencilOp::Keep;
    StencilOp       stencilDepthFailOp  = StencilOp::Keep;
    StencilOp       stencilPassOp       = StencilOp::Keep;
    ComparisonFunc  stencilFunc         = ComparisonFunc::Always;
};

struct DepthStencilDesc
{
    bool            depthTestEnable     = false;
    bool            depthWriteEnable    = true;
    ComparisonFunc  depthFunc           = ComparisonFunc::Less;

    bool                stencilEnable       = false;
    uint32_t             stencilReadMask     = 0xFFFFFFFF;
    uint32_t             stencilWriteMask    = 0xFFFFFFFF;
    DepthStencilOpDesc  frontFace;
    DepthStencilOpDesc  backFace;

    uint32_t stencilRef = 0;
};

struct RasterizerDesc
{
    FillMode fillMode = FillMode::Solid;
    CullMode cullMode = CullMode::None;
    FrontFaceMode frontFace = FrontFaceMode::CounterClockwise;
    int32_t depthBias = 0;
    float depthBiasClamp = 0.0f;
    float slopeScaledDepthBias = 0.0f;
    bool depthClipEnable = true;
    bool scissorEnable = false;
    bool multisampleEnable = false;
    bool antialiasedLineEnable = false;
    bool enableConservativeRasterization = false;
    uint32_t forcedSampleCount = 0;
};

enum class LogicOp
{
    NoOp,
};

enum class BlendOp
{
    Add,
    Subtract,
    ReverseSubtract,
    Min,
    Max,
};

enum class BlendFactor
{
    Zero,
    One,
    SrcColor,
    InvSrcColor,
    SrcAlpha,
    InvSrcAlpha,
    DestAlpha,
    InvDestAlpha,
    DestColor,
    InvDestColor,
    SrcAlphaSaturate,
    BlendColor,
    InvBlendColor,
    SecondarySrcColor,
    InvSecondarySrcColor,
    SecondarySrcAlpha,
    InvSecondarySrcAlpha,
};

namespace RenderTargetWriteMask
{
    typedef uint8_t Type;
    enum
    {
        EnableNone  = 0,
        EnableRed   = 0x01,
        EnableGreen = 0x02,
        EnableBlue  = 0x04,
        EnableAlpha = 0x08,
        EnableAll   = 0x0F,
    };
};
typedef RenderTargetWriteMask::Type RenderTargetWriteMaskT;

struct AspectBlendDesc
{
    BlendFactor     srcFactor   = BlendFactor::One;
    BlendFactor     dstFactor   = BlendFactor::Zero;
    BlendOp         op          = BlendOp::Add;
};

struct TargetBlendDesc
{
    AspectBlendDesc color;
    AspectBlendDesc alpha;
    bool enableBlend = false;
    LogicOp                 logicOp     = LogicOp::NoOp;
    RenderTargetWriteMaskT  writeMask   = RenderTargetWriteMask::EnableAll;
};

struct BlendDesc
{
    TargetBlendDesc         targets[kMaxRenderTargetCount];
    UInt                    targetCount = 0;

    bool alphaToCoverageEnable  = false;
};

class IFramebufferLayout : public ISlangUnknown
{
public:
    struct AttachmentLayout
    {
        Format format;
        int sampleCount;
    };
    struct Desc
    {
        uint32_t renderTargetCount;
        AttachmentLayout* renderTargets = nullptr;
        AttachmentLayout* depthStencil = nullptr;
    };
};
#define SLANG_UUID_IFramebufferLayout                                                \
    {                                                                                \
        0xa838785, 0xc13a, 0x4832, { 0xad, 0x88, 0x64, 0x6, 0xb5, 0x4b, 0x5e, 0xba } \
    }

struct GraphicsPipelineStateDesc
{
    IShaderProgram*      program = nullptr;

    IInputLayout*       inputLayout = nullptr;
    IFramebufferLayout* framebufferLayout = nullptr;
    PrimitiveType       primitiveType = PrimitiveType::Triangle;
    DepthStencilDesc    depthStencil;
    RasterizerDesc      rasterizer;
    BlendDesc           blend;
};

struct ComputePipelineStateDesc
{
    IShaderProgram*  program = nullptr;
    void* d3d12RootSignatureOverride = nullptr;
};

struct RayTracingPipelineFlags
{
    enum Enum : uint32_t
    {
        None = 0,
        SkipTriangles = 1,
        SkipProcedurals = 2,
    };
};

struct HitGroupDesc
{
    const char* hitGroupName = nullptr;
    const char* closestHitEntryPoint = nullptr;
    const char* anyHitEntryPoint = nullptr;
    const char* intersectionEntryPoint = nullptr;
};

struct RayTracingPipelineStateDesc
{
    IShaderProgram* program = nullptr;
    int32_t hitGroupCount = 0;
    const HitGroupDesc* hitGroups = nullptr;
    int maxRecursion = 0;
    int maxRayPayloadSize = 0;
    int maxAttributeSizeInBytes = 8;
    RayTracingPipelineFlags::Enum flags = RayTracingPipelineFlags::None;
};

class IShaderTable : public ISlangUnknown
{
public:
    // Specifies the bytes to overwrite into a record in the shader table.
    struct ShaderRecordOverwrite
    {
        uint32_t offset; // Offset within the shader record.
        uint32_t size; // Number of bytes to overwrite.
        uint8_t data[8]; // Content to overwrite.
    };

    struct Desc
    {
        uint32_t rayGenShaderCount;
        const char** rayGenShaderEntryPointNames;
        const ShaderRecordOverwrite* rayGenShaderRecordOverwrites;

        uint32_t missShaderCount;
        const char** missShaderEntryPointNames;
        const ShaderRecordOverwrite* missShaderRecordOverwrites;

        uint32_t hitGroupCount;
        const char** hitGroupNames;
        const ShaderRecordOverwrite* hitGroupRecordOverwrites;

        IShaderProgram* program;
    };
};
#define SLANG_UUID_IShaderTable                                                        \
    {                                                                                  \
        0xa721522c, 0xdf31, 0x4c2f, { 0xa5, 0xe7, 0x3b, 0xe0, 0x12, 0x4b, 0x31, 0x78 } \
    }

class IPipelineState : public ISlangUnknown
{
public:
    virtual SLANG_NO_THROW Result SLANG_MCALL getNativeHandle(InteropHandle* outHandle) = 0;
};
#define SLANG_UUID_IPipelineState                                                      \
    {                                                                                 \
        0xca7e57d, 0x8a90, 0x44f3, { 0xbd, 0xb1, 0xfe, 0x9b, 0x35, 0x3f, 0x5a, 0x72 } \
    }


struct ScissorRect
{
    int32_t minX;
    int32_t minY;
    int32_t maxX;
    int32_t maxY;
};

struct Viewport
{
    float originX = 0.0f;
    float originY = 0.0f;
    float extentX = 0.0f;
    float extentY = 0.0f;
    float minZ    = 0.0f;
    float maxZ    = 1.0f;
};

class IFramebuffer : public ISlangUnknown
{
public:
    struct Desc
    {
        uint32_t renderTargetCount;
        IResourceView* const* renderTargetViews;
        IResourceView* depthStencilView;
        IFramebufferLayout* layout;
    };
};
#define SLANG_UUID_IFrameBuffer                                                       \
    {                                                                                 \
        0xf0c0d9a, 0x4ef3, 0x4e18, { 0x9b, 0xa9, 0x34, 0x60, 0xea, 0x69, 0x87, 0x95 } \
    }

struct WindowHandle
{
    enum class Type
    {
        Unknown,
        Win32Handle,
        XLibHandle,
    };
    Type type;
    intptr_t handleValues[2];
    static WindowHandle FromHwnd(void* hwnd)
    {
        WindowHandle handle = {};
        handle.type = WindowHandle::Type::Win32Handle;
        handle.handleValues[0] = (intptr_t)(hwnd);
        return handle;
    }
    static WindowHandle FromXWindow(void* xdisplay, uint32_t xwindow)
    {
        WindowHandle handle = {};
        handle.type = WindowHandle::Type::XLibHandle;
        handle.handleValues[0] = (intptr_t)(xdisplay);
        handle.handleValues[1] = xwindow;
        return handle;
    }
};

struct FaceMask
{
    enum Enum
    {
        Front = 1, Back = 2
    };
};

class IRenderPassLayout : public ISlangUnknown
{
public:
    enum class AttachmentLoadOp
    {
        Load, Clear, DontCare
    };
    enum class AttachmentStoreOp
    {
        Store, DontCare
    };
    struct AttachmentAccessDesc
    {
        AttachmentLoadOp loadOp;
        AttachmentLoadOp stencilLoadOp;
        AttachmentStoreOp storeOp;
        AttachmentStoreOp stencilStoreOp;
        ResourceState initialState;
        ResourceState finalState;
    };
    struct Desc
    {
        IFramebufferLayout* framebufferLayout = nullptr;
        uint32_t renderTargetCount;
        AttachmentAccessDesc* renderTargetAccess = nullptr;
        AttachmentAccessDesc* depthStencilAccess = nullptr;
    };
};
#define SLANG_UUID_IRenderPassLayout                                                   \
    {                                                                                  \
        0xdaab0b1a, 0xf45d, 0x4ae9, { 0xbf, 0x2c, 0xe0, 0xbb, 0x76, 0x7d, 0xfa, 0xd1 } \
    }

enum class QueryType
{
    Timestamp,
    AccelerationStructureCompactedSize,
    AccelerationStructureSerializedSize,
    AccelerationStructureCurrentSize,
};

class IQueryPool : public ISlangUnknown
{
public:
    struct Desc
    {
        QueryType type;
        SlangInt count;
    };
public:
    virtual SLANG_NO_THROW Result SLANG_MCALL getResult(SlangInt queryIndex, SlangInt count, uint64_t* data) = 0;
    virtual SLANG_NO_THROW Result SLANG_MCALL reset() = 0;
};
#define SLANG_UUID_IQueryPool                                                         \
    { 0xc2cc3784, 0x12da, 0x480a, { 0xa8, 0x74, 0x8b, 0x31, 0x96, 0x1c, 0xa4, 0x36 } }


class ICommandEncoder
{
public:
    virtual SLANG_NO_THROW void SLANG_MCALL endEncoding() = 0;
    virtual SLANG_NO_THROW void SLANG_MCALL writeTimestamp(IQueryPool* queryPool, SlangInt queryIndex) = 0;
};

struct IndirectDispatchArguments
{
    uint32_t ThreadGroupCountX;
    uint32_t ThreadGroupCountY;
    uint32_t ThreadGroupCountZ;
};

struct IndirectDrawArguments
{
    uint32_t VertexCountPerInstance;
    uint32_t InstanceCount;
    uint32_t StartVertexLocation;
    uint32_t StartInstanceLocation;
};

struct IndirectDrawIndexedArguments
{
    uint32_t IndexCountPerInstance;
    uint32_t InstanceCount;
    uint32_t StartIndexLocation;
    int32_t  BaseVertexLocation;
    uint32_t StartInstanceLocation;
};

struct SamplePosition
{
    int8_t x;
    int8_t y;
};

struct ClearResourceViewFlags
{
    enum Enum : uint32_t
    {
        None = 0,
        ClearDepth = 1,
        ClearStencil = 2,
        FloatClearValues = 4
    };
};

class IResourceCommandEncoder : public ICommandEncoder
{
public:
    virtual SLANG_NO_THROW void SLANG_MCALL copyBuffer(
        IBufferResource* dst,
        size_t dstOffset,
        IBufferResource* src,
        size_t srcOffset,
        size_t size) = 0;

    /// Copies texture from src to dst. If dstSubresource and srcSubresource has mipLevelCount = 0
    /// and layerCount = 0, the entire resource is being copied and dstOffset, srcOffset and extent
    /// arguments are ignored.
    virtual SLANG_NO_THROW void SLANG_MCALL copyTexture(
        ITextureResource* dst,
        ResourceState dstState,
        SubresourceRange dstSubresource,
        ITextureResource::Offset3D dstOffset,
        ITextureResource* src,
        ResourceState srcState,
        SubresourceRange srcSubresource,
        ITextureResource::Offset3D srcOffset,
        ITextureResource::Size extent) = 0;

    /// Copies texture to a buffer. Each row is aligned to kTexturePitchAlignment.
    virtual SLANG_NO_THROW void SLANG_MCALL copyTextureToBuffer(
        IBufferResource* dst,
        size_t dstOffset,
        size_t dstSize,
        size_t dstRowStride,
        ITextureResource* src,
        ResourceState srcState,
        SubresourceRange srcSubresource,
        ITextureResource::Offset3D srcOffset,
        ITextureResource::Size extent) = 0;
    virtual SLANG_NO_THROW void SLANG_MCALL uploadTextureData(
        ITextureResource* dst,
        SubresourceRange subResourceRange,
        ITextureResource::Offset3D offset,
        ITextureResource::Size extent,
        ITextureResource::SubresourceData* subResourceData,
        size_t subResourceDataCount) = 0;
    virtual SLANG_NO_THROW void SLANG_MCALL
        uploadBufferData(IBufferResource* dst, size_t offset, size_t size, void* data) = 0;

    virtual SLANG_NO_THROW void SLANG_MCALL textureBarrier(
        size_t count, ITextureResource* const* textures, ResourceState src, ResourceState dst) = 0;
    void textureBarrier(ITextureResource* texture, ResourceState src, ResourceState dst)
    {
        textureBarrier(1, &texture, src, dst);
    }
    virtual SLANG_NO_THROW void SLANG_MCALL textureSubresourceBarrier(
        ITextureResource* texture,
        SubresourceRange subresourceRange,
        ResourceState src,
        ResourceState dst) = 0;
    virtual SLANG_NO_THROW void SLANG_MCALL bufferBarrier(
        size_t count, IBufferResource* const* buffers, ResourceState src, ResourceState dst) = 0;
    void bufferBarrier(IBufferResource* buffer, ResourceState src, ResourceState dst)
    {
        bufferBarrier(1, &buffer, src, dst);
    }
    virtual SLANG_NO_THROW void SLANG_MCALL clearResourceView(
        IResourceView* view, ClearValue* clearValue, ClearResourceViewFlags::Enum flags) = 0;
    virtual SLANG_NO_THROW void SLANG_MCALL resolveResource(
        ITextureResource* source,
        ResourceState sourceState,
        SubresourceRange sourceRange,
        ITextureResource* dest,
        ResourceState destState,
        SubresourceRange destRange) = 0;
    virtual SLANG_NO_THROW void SLANG_MCALL resolveQuery(
        IQueryPool* queryPool,
        uint32_t index,
        uint32_t count,
        IBufferResource* buffer,
        uint64_t offset) = 0;
    virtual SLANG_NO_THROW void SLANG_MCALL beginDebugEvent(const char* name, float rgbColor[3]) = 0;
    virtual SLANG_NO_THROW void SLANG_MCALL endDebugEvent() = 0;
};

class IRenderCommandEncoder : public IResourceCommandEncoder
{
public:
    // Sets the current pipeline state. This method returns a transient shader object for
    // writing shader parameters. This shader object will not retain any resources or
    // sub-shader-objects bound to it. The user must be responsible for ensuring that any
    // resources or shader objects that is set into `outRootShaderObject` stays alive during
    // the execution of the command buffer.
    virtual SLANG_NO_THROW Result SLANG_MCALL
        bindPipeline(IPipelineState* state, IShaderObject** outRootShaderObject) = 0;
    inline IShaderObject* bindPipeline(IPipelineState* state)
    {
        IShaderObject* rootObject = nullptr;
        SLANG_RETURN_NULL_ON_FAIL(bindPipeline(state, &rootObject));
        return rootObject;
    }

    // Sets the current pipeline state along with a pre-created mutable root shader object.
    virtual SLANG_NO_THROW Result SLANG_MCALL
        bindPipelineWithRootObject(IPipelineState* state, IShaderObject* rootObject) = 0;

    virtual SLANG_NO_THROW void
        SLANG_MCALL setViewports(uint32_t count, const Viewport* viewports) = 0;
    virtual SLANG_NO_THROW void
        SLANG_MCALL setScissorRects(uint32_t count, const ScissorRect* scissors) = 0;

    /// Sets the viewport, and sets the scissor rect to match the viewport.
    inline void setViewportAndScissor(Viewport const& viewport)
    {
        setViewports(1, &viewport);
        ScissorRect rect = {};
        rect.maxX = static_cast<gfx::Int>(viewport.extentX);
        rect.maxY = static_cast<gfx::Int>(viewport.extentY);
        setScissorRects(1, &rect);
    }

    virtual SLANG_NO_THROW void SLANG_MCALL setPrimitiveTopology(PrimitiveTopology topology) = 0;
    virtual SLANG_NO_THROW void SLANG_MCALL setVertexBuffers(
        uint32_t startSlot,
        uint32_t slotCount,
        IBufferResource* const* buffers,
        const uint32_t* offsets) = 0;
    inline void setVertexBuffer(
        uint32_t slot, IBufferResource* buffer, uint32_t offset = 0)
    {
        setVertexBuffers(slot, 1, &buffer, &offset);
    }

    virtual SLANG_NO_THROW void SLANG_MCALL
        setIndexBuffer(IBufferResource* buffer, Format indexFormat, uint32_t offset = 0) = 0;
    virtual SLANG_NO_THROW void SLANG_MCALL
        draw(uint32_t vertexCount, uint32_t startVertex = 0) = 0;
    virtual SLANG_NO_THROW void SLANG_MCALL
        drawIndexed(uint32_t indexCount, uint32_t startIndex = 0, uint32_t baseVertex = 0) = 0;
    virtual SLANG_NO_THROW void SLANG_MCALL drawIndirect(
        uint32_t maxDrawCount,
        IBufferResource* argBuffer,
        uint64_t argOffset,
        IBufferResource* countBuffer = nullptr,
        uint64_t countOffset = 0) = 0;
    virtual SLANG_NO_THROW void SLANG_MCALL drawIndexedIndirect(
        uint32_t maxDrawCount,
        IBufferResource* argBuffer,
        uint64_t argOffset,
        IBufferResource* countBuffer = nullptr,
        uint64_t countOffset = 0) = 0;
    virtual SLANG_NO_THROW void SLANG_MCALL setStencilReference(uint32_t referenceValue) = 0;
    virtual SLANG_NO_THROW Result SLANG_MCALL setSamplePositions(
        uint32_t samplesPerPixel, uint32_t pixelCount, const SamplePosition* samplePositions) = 0;
    virtual SLANG_NO_THROW void SLANG_MCALL drawInstanced(
        uint32_t vertexCount,
        uint32_t instanceCount,
        uint32_t startVertex,
        uint32_t startInstanceLocation) = 0;
    virtual SLANG_NO_THROW void SLANG_MCALL drawIndexedInstanced(
        uint32_t indexCount,
        uint32_t instanceCount,
        uint32_t startIndexLocation,
        int32_t baseVertexLocation,
        uint32_t startInstanceLocation) = 0;
};

class IComputeCommandEncoder : public IResourceCommandEncoder
{
public:
    // Sets the current pipeline state. This method returns a transient shader object for
    // writing shader parameters. This shader object will not retain any resources or
    // sub-shader-objects bound to it. The user must be responsible for ensuring that any
    // resources or shader objects that is set into `outRooShaderObject` stays alive during
    // the execution of the command buffer.
    virtual SLANG_NO_THROW Result SLANG_MCALL
        bindPipeline(IPipelineState* state, IShaderObject** outRootShaderObject) = 0;
    inline IShaderObject* bindPipeline(IPipelineState* state)
    {
        IShaderObject* rootObject = nullptr;
        SLANG_RETURN_NULL_ON_FAIL(bindPipeline(state, &rootObject));
        return rootObject;
    }
    // Sets the current pipeline state along with a pre-created mutable root shader object.
    virtual SLANG_NO_THROW Result SLANG_MCALL
        bindPipelineWithRootObject(IPipelineState* state, IShaderObject* rootObject) = 0;
    virtual SLANG_NO_THROW void SLANG_MCALL dispatchCompute(int x, int y, int z) = 0;
    virtual SLANG_NO_THROW void SLANG_MCALL dispatchComputeIndirect(IBufferResource* cmdBuffer, uint64_t offset) = 0;
};

enum class AccelerationStructureCopyMode
{
    Clone, Compact
};

struct AccelerationStructureQueryDesc
{
    QueryType queryType;

    IQueryPool* queryPool;

    int32_t firstQueryIndex;
};

class IRayTracingCommandEncoder : public IResourceCommandEncoder
{
public:
    virtual SLANG_NO_THROW void SLANG_MCALL buildAccelerationStructure(
        const IAccelerationStructure::BuildDesc& desc,
        int propertyQueryCount,
        AccelerationStructureQueryDesc* queryDescs) = 0;
    virtual SLANG_NO_THROW void SLANG_MCALL copyAccelerationStructure(
        IAccelerationStructure* dest,
        IAccelerationStructure* src,
        AccelerationStructureCopyMode mode) = 0;
    virtual SLANG_NO_THROW void SLANG_MCALL queryAccelerationStructureProperties(
        int accelerationStructureCount,
        IAccelerationStructure* const* accelerationStructures,
        int queryCount,
        AccelerationStructureQueryDesc* queryDescs) = 0;
    virtual SLANG_NO_THROW void SLANG_MCALL
        serializeAccelerationStructure(DeviceAddress dest, IAccelerationStructure* source) = 0;
    virtual SLANG_NO_THROW void SLANG_MCALL
        deserializeAccelerationStructure(IAccelerationStructure* dest, DeviceAddress source) = 0;

    virtual SLANG_NO_THROW void SLANG_MCALL
        bindPipeline(IPipelineState* state, IShaderObject** outRootObject) = 0;
    // Sets the current pipeline state along with a pre-created mutable root shader object.
    virtual SLANG_NO_THROW Result SLANG_MCALL
        bindPipelineWithRootObject(IPipelineState* state, IShaderObject* rootObject) = 0;

    /// Issues a dispatch command to start ray tracing workload with a ray tracing pipeline.
    /// `rayGenShaderIndex` specifies the index into the shader table that identifies the ray generation shader.
    virtual SLANG_NO_THROW void SLANG_MCALL dispatchRays(
        uint32_t rayGenShaderIndex,
        IShaderTable* shaderTable,
        int32_t width,
        int32_t height,
        int32_t depth) = 0;
};
#define SLANG_UUID_IRayTracingCommandEncoder                                           \
    {                                                                                  \
        0x9a672b87, 0x5035, 0x45e3, { 0x96, 0x7c, 0x1f, 0x85, 0xcd, 0xb3, 0x63, 0x4f } \
    }

class ICommandBuffer : public ISlangUnknown
{
public:
    // Only one encoder may be open at a time. User must call `ICommandEncoder::endEncoding`
    // before calling other `encode*Commands` methods.
    // Once `endEncoding` is called, the `ICommandEncoder` object becomes obsolete and is
    // invalid for further use. To continue recording, the user must request a new encoder
    // object by calling one of the `encode*Commands` methods again.
    virtual SLANG_NO_THROW void SLANG_MCALL encodeRenderCommands(
        IRenderPassLayout* renderPass,
        IFramebuffer* framebuffer,
        IRenderCommandEncoder** outEncoder) = 0;
    IRenderCommandEncoder*
        encodeRenderCommands(IRenderPassLayout* renderPass, IFramebuffer* framebuffer)
    {
        IRenderCommandEncoder* result;
        encodeRenderCommands(renderPass, framebuffer, &result);
        return result;
    }

    virtual SLANG_NO_THROW void SLANG_MCALL
        encodeComputeCommands(IComputeCommandEncoder** outEncoder) = 0;
    IComputeCommandEncoder* encodeComputeCommands()
    {
        IComputeCommandEncoder* result;
        encodeComputeCommands(&result);
        return result;
    }

    virtual SLANG_NO_THROW void SLANG_MCALL
        encodeResourceCommands(IResourceCommandEncoder** outEncoder) = 0;
    IResourceCommandEncoder* encodeResourceCommands()
    {
        IResourceCommandEncoder* result;
        encodeResourceCommands(&result);
        return result;
    }

    virtual SLANG_NO_THROW void SLANG_MCALL
        encodeRayTracingCommands(IRayTracingCommandEncoder** outEncoder) = 0;
    IRayTracingCommandEncoder* encodeRayTracingCommands()
    {
        IRayTracingCommandEncoder* result;
        encodeRayTracingCommands(&result);
        return result;
    }

    virtual SLANG_NO_THROW void SLANG_MCALL close() = 0;

    virtual SLANG_NO_THROW Result SLANG_MCALL getNativeHandle(InteropHandle* outHandle) = 0;
};
#define SLANG_UUID_ICommandBuffer                                                      \
    {                                                                                  \
        0x5d56063f, 0x91d4, 0x4723, { 0xa7, 0xa7, 0x7a, 0x15, 0xaf, 0x93, 0xeb, 0x48 } \
    }

class ICommandQueue : public ISlangUnknown
{
public:
    enum class QueueType
    {
        Graphics
    };
    struct Desc
    {
        QueueType type;
    };

    // For D3D12, this is the pointer to the queue. For Vulkan, this is the queue itself.
    typedef uint64_t NativeHandle;

    virtual SLANG_NO_THROW const Desc& SLANG_MCALL getDesc() = 0;

    virtual SLANG_NO_THROW void SLANG_MCALL executeCommandBuffers(
        uint32_t count,
        ICommandBuffer* const* commandBuffers,
        IFence* fenceToSignal,
        uint64_t newFenceValue) = 0;
    inline void executeCommandBuffer(
        ICommandBuffer* commandBuffer, IFence* fenceToSignal = nullptr, uint64_t newFenceValue = 0)
    {
        executeCommandBuffers(1, &commandBuffer, fenceToSignal, newFenceValue);
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL getNativeHandle(InteropHandle* outHandle) = 0;

    virtual SLANG_NO_THROW void SLANG_MCALL waitOnHost() = 0;

    /// Queues a device side wait for the given fences.
    virtual SLANG_NO_THROW Result SLANG_MCALL
        waitForFenceValuesOnDevice(uint32_t fenceCount, IFence** fences, uint64_t* waitValues) = 0;
};
#define SLANG_UUID_ICommandQueue                                                    \
    {                                                                               \
        0x14e2bed0, 0xad0, 0x4dc8, { 0xb3, 0x41, 0x6, 0x3f, 0xe7, 0x2d, 0xbf, 0xe } \
    }

class ITransientResourceHeap : public ISlangUnknown
{
public:
    struct Flags
    {
        enum Enum
        {
            None = 0,
            AllowResizing = 0x1,
        };
    };
    struct Desc
    {
        Flags::Enum flags;
        size_t constantBufferSize;
        uint32_t samplerDescriptorCount;
        uint32_t uavDescriptorCount;
        uint32_t srvDescriptorCount;
        uint32_t constantBufferDescriptorCount;
        uint32_t accelerationStructureDescriptorCount;
    };

    // Waits until GPU commands issued before last call to `finish()` has been completed, and resets
    // all transient resources holds by the heap.
    // This method must be called before using the transient heap to issue new GPU commands.
    // In most situations this method should be called at the beginning of each frame.
    virtual SLANG_NO_THROW Result SLANG_MCALL synchronizeAndReset() = 0;

    // Must be called when the application has done using this heap to issue commands. In most situations
    // this method should be called at the end of each frame.
    virtual SLANG_NO_THROW Result SLANG_MCALL finish() = 0;

    // Command buffers are one-time use. Once it is submitted to the queue via
    // `executeCommandBuffers` a command buffer is no longer valid to be used any more. Command
    // buffers must be closed before submission. The current D3D12 implementation has a limitation
    // that only one command buffer maybe recorded at a time. User must finish recording a command
    // buffer before creating another command buffer.
    virtual SLANG_NO_THROW Result SLANG_MCALL
        createCommandBuffer(ICommandBuffer** outCommandBuffer) = 0;
    inline ComPtr<ICommandBuffer> createCommandBuffer()
    {
        ComPtr<ICommandBuffer> result;
        SLANG_RETURN_NULL_ON_FAIL(createCommandBuffer(result.writeRef()));
        return result;
    }
};
#define SLANG_UUID_ITransientResourceHeap                                             \
    {                                                                                 \
        0xcd48bd29, 0xee72, 0x41b8, { 0xbc, 0xff, 0xa, 0x2b, 0x3a, 0xaa, 0x6d, 0xeb } \
    }

class ID3D12TransientResourceHeap : public ISlangUnknown
{
public:
    enum class DescriptorType
    {
        ResourceView, Sampler
    };
    virtual SLANG_NO_THROW Result SLANG_MCALL allocateTransientDescriptorTable(
        DescriptorType type,
        uint32_t count,
        uint64_t& outDescriptorOffset,
        void** outD3DDescriptorHeapHandle) = 0;
};
#define SLANG_UUID_ID3D12TransientResourceHeap                                             \
    {                                                                                  \
        0x9bc6a8bc, 0x5f7a, 0x454a, { 0x93, 0xef, 0x3b, 0x10, 0x5b, 0xb7, 0x63, 0x7e } \
    }

class ISwapchain : public ISlangUnknown
{
public:
    struct Desc
    {
        Format format;
        uint32_t width, height;
        uint32_t imageCount;
        ICommandQueue* queue;
        bool enableVSync;
    };
    virtual SLANG_NO_THROW const Desc& SLANG_MCALL getDesc() = 0;

    /// Returns the back buffer image at `index`.
    virtual SLANG_NO_THROW Result SLANG_MCALL
        getImage(uint32_t index, ITextureResource** outResource) = 0;

    /// Present the next image in the swapchain.
    virtual SLANG_NO_THROW Result SLANG_MCALL present() = 0;

    /// Returns the index of next back buffer image that will be presented in the next
    /// `present` call. If the swapchain is invalid/out-of-date, this method returns -1.
    virtual SLANG_NO_THROW int SLANG_MCALL acquireNextImage() = 0;

    /// Resizes the back buffers of this swapchain. All render target views and framebuffers
    /// referencing the back buffer images must be freed before calling this method.
    virtual SLANG_NO_THROW Result SLANG_MCALL resize(uint32_t width, uint32_t height) = 0;

    // Check if the window is occluded.
    virtual SLANG_NO_THROW bool SLANG_MCALL isOccluded() = 0;

    // Toggle full screen mode.
    virtual SLANG_NO_THROW Result SLANG_MCALL setFullScreenMode(bool mode) = 0;
};
#define SLANG_UUID_ISwapchain                                                        \
    {                                                                                \
        0xbe91ba6c, 0x784, 0x4308, { 0xa1, 0x0, 0x19, 0xc3, 0x66, 0x83, 0x44, 0xb2 } \
    }

struct DeviceInfo
{
    DeviceType deviceType;

    BindingStyle bindingStyle;

    ProjectionStyle projectionStyle;

    /// An projection matrix that ensures x, y mapping to pixels
    /// is the same on all targets
    float identityProjectionMatrix[16];

    /// The name of the graphics API being used by this device.
    const char* apiName = nullptr;

    /// The name of the graphics adapter.
    const char* adapterName = nullptr;

    /// The clock frequency used in timestamp queries.
    uint64_t timestampFrequency = 0;
};

enum class DebugMessageType
{
    Info, Warning, Error
};
enum class DebugMessageSource
{
    Layer, Driver, Slang
};
class IDebugCallback
{
public:
    virtual SLANG_NO_THROW void SLANG_MCALL
        handleMessage(DebugMessageType type, DebugMessageSource source, const char* message) = 0;
};

class IDevice: public ISlangUnknown
{
public:
    struct SlangDesc
    {
        slang::IGlobalSession* slangGlobalSession = nullptr; // (optional) A slang global session object. If null will create automatically.

        SlangMatrixLayoutMode defaultMatrixLayoutMode = SLANG_MATRIX_LAYOUT_ROW_MAJOR;

        char const* const* searchPaths = nullptr;
        SlangInt            searchPathCount = 0;

        slang::PreprocessorMacroDesc const* preprocessorMacros = nullptr;
        SlangInt                        preprocessorMacroCount = 0;

        const char* targetProfile = nullptr; // (optional) Target shader profile. If null this will be set to platform dependent default.
        SlangFloatingPointMode floatingPointMode = SLANG_FLOATING_POINT_MODE_DEFAULT;
        SlangOptimizationLevel optimizationLevel = SLANG_OPTIMIZATION_LEVEL_DEFAULT;
        SlangTargetFlags targetFlags = 0;
        SlangLineDirectiveMode lineDirectiveMode = SLANG_LINE_DIRECTIVE_MODE_DEFAULT;
    };

    struct InteropHandles
    {
        InteropHandle handles[3] = {};
    };

    struct Desc
    {
        // The underlying API/Platform of the device.
        DeviceType deviceType = DeviceType::Default;
        // The device's handles (if they exist) and their associated API. For D3D12, this contains a single InteropHandle
        // for the ID3D12Device. For Vulkan, the first InteropHandle is the VkInstance, the second is the VkPhysicalDevice,
        // and the third is the VkDevice. For CUDA, this only contains a single value for the CUDADevice.
        InteropHandles existingDeviceHandles;
        // Name to identify the adapter to use
        const char* adapter = nullptr;
        // Number of required features.
        int requiredFeatureCount = 0;
        // Array of required feature names, whose size is `requiredFeatureCount`.
        const char** requiredFeatures = nullptr;
        // A command dispatcher object that intercepts and handles actual low-level API call.
        ISlangUnknown* apiCommandDispatcher = nullptr;
        // The slot (typically UAV) used to identify NVAPI intrinsics. If >=0 NVAPI is required.
        int nvapiExtnSlot = -1;
        // The file system for loading cached shader kernels. The layer does not maintain a strong reference to the object,
        // instead the user is responsible for holding the object alive during the lifetime of an `IDevice`.
        ISlangFileSystem* shaderCacheFileSystem = nullptr;
        // Configurations for Slang compiler.
        SlangDesc slang = {};

        uint32_t extendedDescCount = 0;
        void** extendedDescs = nullptr;
    };

    virtual SLANG_NO_THROW Result SLANG_MCALL getNativeDeviceHandles(InteropHandles* outHandles) = 0;

    virtual SLANG_NO_THROW bool SLANG_MCALL hasFeature(const char* feature) = 0;

        /// Returns a list of features supported by the renderer.
    virtual SLANG_NO_THROW Result SLANG_MCALL getFeatures(const char** outFeatures, UInt bufferSize, UInt* outFeatureCount) = 0;

    virtual SLANG_NO_THROW Result SLANG_MCALL getFormatSupportedResourceStates(Format format, ResourceStateSet* outStates) = 0;

    virtual SLANG_NO_THROW Result SLANG_MCALL getSlangSession(slang::ISession** outSlangSession) = 0;

    inline ComPtr<slang::ISession> getSlangSession()
    {
        ComPtr<slang::ISession> result;
        getSlangSession(result.writeRef());
        return result;
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL createTransientResourceHeap(
        const ITransientResourceHeap::Desc& desc,
        ITransientResourceHeap** outHeap) = 0;
    inline ComPtr<ITransientResourceHeap> createTransientResourceHeap(
        const ITransientResourceHeap::Desc& desc)
    {
        ComPtr<ITransientResourceHeap> result;
        createTransientResourceHeap(desc, result.writeRef());
        return result;
    }

        /// Create a texture resource.
        ///
        /// If `initData` is non-null, then it must point to an array of
        /// `ITextureResource::SubresourceData` with one element for each
        /// subresource of the texture being created.
        ///
        /// The number of subresources in a texture is:
        ///
        ///     effectiveElementCount * mipLevelCount
        ///
        /// where the effective element count is computed as:
        ///
        ///     effectiveElementCount = (isArray ? arrayElementCount : 1) * (isCube ? 6 : 1);
        ///
    virtual SLANG_NO_THROW Result SLANG_MCALL createTextureResource(
        const ITextureResource::Desc& desc,
        const ITextureResource::SubresourceData* initData,
        ITextureResource** outResource) = 0;

        /// Create a texture resource. initData holds the initialize data to set the contents of the texture when constructed.
    inline SLANG_NO_THROW ComPtr<ITextureResource> createTextureResource(
        const ITextureResource::Desc& desc,
        const ITextureResource::SubresourceData* initData = nullptr)
    {
        ComPtr<ITextureResource> resource;
        SLANG_RETURN_NULL_ON_FAIL(createTextureResource(desc, initData, resource.writeRef()));
        return resource;
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL createTextureFromNativeHandle(
        InteropHandle handle,
        const ITextureResource::Desc& srcDesc,
        ITextureResource** outResource) = 0;

    virtual SLANG_NO_THROW Result SLANG_MCALL createTextureFromSharedHandle(
        InteropHandle handle,
        const ITextureResource::Desc& srcDesc,
        const size_t size,
        ITextureResource** outResource) = 0;

        /// Create a buffer resource
    virtual SLANG_NO_THROW Result SLANG_MCALL createBufferResource(
        const IBufferResource::Desc& desc,
        const void* initData,
        IBufferResource** outResource) = 0;

    inline SLANG_NO_THROW ComPtr<IBufferResource> createBufferResource(
        const IBufferResource::Desc& desc,
        const void* initData = nullptr)
    {
        ComPtr<IBufferResource> resource;
        SLANG_RETURN_NULL_ON_FAIL(createBufferResource(desc, initData, resource.writeRef()));
        return resource;
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL createBufferFromNativeHandle(
        InteropHandle handle,
        const IBufferResource::Desc& srcDesc,
        IBufferResource** outResource) = 0;

    virtual SLANG_NO_THROW Result SLANG_MCALL createBufferFromSharedHandle(
        InteropHandle handle,
        const IBufferResource::Desc& srcDesc,
        IBufferResource** outResource) = 0;

    virtual SLANG_NO_THROW Result SLANG_MCALL
        createSamplerState(ISamplerState::Desc const& desc, ISamplerState** outSampler) = 0;

    inline ComPtr<ISamplerState> createSamplerState(ISamplerState::Desc const& desc)
    {
        ComPtr<ISamplerState> sampler;
        SLANG_RETURN_NULL_ON_FAIL(createSamplerState(desc, sampler.writeRef()));
        return sampler;
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL createTextureView(
        ITextureResource* texture, IResourceView::Desc const& desc, IResourceView** outView) = 0;

    inline ComPtr<IResourceView> createTextureView(ITextureResource* texture, IResourceView::Desc const& desc)
    {
        ComPtr<IResourceView> view;
        SLANG_RETURN_NULL_ON_FAIL(createTextureView(texture, desc, view.writeRef()));
        return view;
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL createBufferView(
        IBufferResource* buffer,
        IBufferResource* counterBuffer,
        IResourceView::Desc const& desc,
        IResourceView** outView) = 0;

    inline ComPtr<IResourceView> createBufferView(
        IBufferResource* buffer, IBufferResource* counterBuffer, IResourceView::Desc const& desc)
    {
        ComPtr<IResourceView> view;
        SLANG_RETURN_NULL_ON_FAIL(createBufferView(buffer, counterBuffer, desc, view.writeRef()));
        return view;
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL
        createFramebufferLayout(IFramebufferLayout::Desc const& desc, IFramebufferLayout** outFrameBuffer) = 0;
    inline ComPtr<IFramebufferLayout> createFramebufferLayout(IFramebufferLayout::Desc const& desc)
    {
        ComPtr<IFramebufferLayout> fb;
        SLANG_RETURN_NULL_ON_FAIL(createFramebufferLayout(desc, fb.writeRef()));
        return fb;
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL
        createFramebuffer(IFramebuffer::Desc const& desc, IFramebuffer** outFrameBuffer) = 0;
    inline ComPtr<IFramebuffer> createFramebuffer(IFramebuffer::Desc const& desc)
    {
        ComPtr<IFramebuffer> fb;
        SLANG_RETURN_NULL_ON_FAIL(createFramebuffer(desc, fb.writeRef()));
        return fb;
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL createRenderPassLayout(
        const IRenderPassLayout::Desc& desc,
        IRenderPassLayout** outRenderPassLayout) = 0;
    inline ComPtr<IRenderPassLayout> createRenderPassLayout(const IRenderPassLayout::Desc& desc)
    {
        ComPtr<IRenderPassLayout> rs;
        SLANG_RETURN_NULL_ON_FAIL(createRenderPassLayout(desc, rs.writeRef()));
        return rs;
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL createSwapchain(
        ISwapchain::Desc const& desc, WindowHandle window, ISwapchain** outSwapchain) = 0;
    inline ComPtr<ISwapchain> createSwapchain(ISwapchain::Desc const& desc, WindowHandle window)
    {
        ComPtr<ISwapchain> swapchain;
        SLANG_RETURN_NULL_ON_FAIL(createSwapchain(desc, window, swapchain.writeRef()));
        return swapchain;
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL createInputLayout(
        IInputLayout::Desc const& desc, IInputLayout** outLayout) = 0;

    inline ComPtr<IInputLayout> createInputLayout(IInputLayout::Desc const& desc)
    {
        ComPtr<IInputLayout> layout;
        SLANG_RETURN_NULL_ON_FAIL(createInputLayout(desc, layout.writeRef()));
        return layout;
    }

    inline Result createInputLayout(size_t vertexSize, InputElementDesc const* inputElements, Int inputElementCount, IInputLayout** outLayout)
    {
        VertexStreamDesc streamDesc = { (uint32_t)vertexSize, InputSlotClass::PerVertex, 0 };

        IInputLayout::Desc inputLayoutDesc = {};
        inputLayoutDesc.inputElementCount = inputElementCount;
        inputLayoutDesc.inputElements = inputElements;
        inputLayoutDesc.vertexStreamCount = 1;
        inputLayoutDesc.vertexStreams = &streamDesc;
        return createInputLayout(inputLayoutDesc, outLayout);
    }

    inline ComPtr<IInputLayout> createInputLayout(size_t vertexSize, InputElementDesc const* inputElements, Int inputElementCount)
    {
        ComPtr<IInputLayout> layout;
        SLANG_RETURN_NULL_ON_FAIL(createInputLayout(vertexSize, inputElements, inputElementCount, layout.writeRef()));
        return layout;
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL
        createCommandQueue(const ICommandQueue::Desc& desc, ICommandQueue** outQueue) = 0;
    inline ComPtr<ICommandQueue> createCommandQueue(const ICommandQueue::Desc& desc)
    {
        ComPtr<ICommandQueue> queue;
        SLANG_RETURN_NULL_ON_FAIL(createCommandQueue(desc, queue.writeRef()));
        return queue;
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL createShaderObject(
        slang::TypeReflection* type,
        ShaderObjectContainerType container,
        IShaderObject** outObject) = 0;

    inline ComPtr<IShaderObject> createShaderObject(slang::TypeReflection* type)
    {
        ComPtr<IShaderObject> object;
        SLANG_RETURN_NULL_ON_FAIL(createShaderObject(type, ShaderObjectContainerType::None, object.writeRef()));
        return object;
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL createMutableShaderObject(
        slang::TypeReflection* type,
        ShaderObjectContainerType container,
        IShaderObject** outObject) = 0;
    
    virtual SLANG_NO_THROW Result SLANG_MCALL createShaderObjectFromTypeLayout(
        slang::TypeLayoutReflection* typeLayout, IShaderObject** outObject) = 0;

    virtual SLANG_NO_THROW Result SLANG_MCALL createMutableShaderObjectFromTypeLayout(
        slang::TypeLayoutReflection* typeLayout, IShaderObject** outObject) = 0;

    virtual SLANG_NO_THROW Result SLANG_MCALL createMutableRootShaderObject(
        IShaderProgram* program,
        IShaderObject** outObject) = 0;

    virtual SLANG_NO_THROW Result SLANG_MCALL
        createShaderTable(const IShaderTable::Desc& desc, IShaderTable** outTable) = 0;

    virtual SLANG_NO_THROW Result SLANG_MCALL createProgram(
        const IShaderProgram::Desc& desc,
        IShaderProgram** outProgram,
        ISlangBlob** outDiagnosticBlob = nullptr) = 0;

    inline ComPtr<IShaderProgram> createProgram(const IShaderProgram::Desc& desc)
    {
        ComPtr<IShaderProgram> program;
        SLANG_RETURN_NULL_ON_FAIL(createProgram(desc, program.writeRef()));
        return program;
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL createGraphicsPipelineState(
        const GraphicsPipelineStateDesc&    desc,
        IPipelineState**                    outState) = 0;

    inline ComPtr<IPipelineState> createGraphicsPipelineState(
        const GraphicsPipelineStateDesc& desc)
    {
        ComPtr<IPipelineState> state;
        SLANG_RETURN_NULL_ON_FAIL(createGraphicsPipelineState(desc, state.writeRef()));
        return state;
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL createComputePipelineState(
        const ComputePipelineStateDesc&    desc,
        IPipelineState**                     outState) = 0;

    inline ComPtr<IPipelineState> createComputePipelineState(
        const ComputePipelineStateDesc& desc)
    {
        ComPtr<IPipelineState> state;
        SLANG_RETURN_NULL_ON_FAIL(createComputePipelineState(desc, state.writeRef()));
        return state;
    }

    virtual SLANG_NO_THROW Result SLANG_MCALL createRayTracingPipelineState(
        const RayTracingPipelineStateDesc& desc, IPipelineState** outState) = 0;

        /// Read back texture resource and stores the result in `outBlob`.
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL readTextureResource(
        ITextureResource* resource,
        ResourceState state,
        ISlangBlob** outBlob,
        size_t* outRowPitch,
        size_t* outPixelSize) = 0;

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL readBufferResource(
        IBufferResource* buffer,
        size_t offset,
        size_t size,
        ISlangBlob** outBlob) = 0;

        /// Get the type of this renderer
    virtual SLANG_NO_THROW const DeviceInfo& SLANG_MCALL getDeviceInfo() const = 0;

    virtual SLANG_NO_THROW Result SLANG_MCALL createQueryPool(
        const IQueryPool::Desc& desc, IQueryPool** outPool) = 0;

    
    virtual SLANG_NO_THROW Result SLANG_MCALL getAccelerationStructurePrebuildInfo(
        const IAccelerationStructure::BuildInputs& buildInputs,
        IAccelerationStructure::PrebuildInfo* outPrebuildInfo) = 0;

    virtual SLANG_NO_THROW Result SLANG_MCALL createAccelerationStructure(
        const IAccelerationStructure::CreateDesc& desc,
        IAccelerationStructure** outView) = 0;

    virtual SLANG_NO_THROW Result SLANG_MCALL
        createFence(const IFence::Desc& desc, IFence** outFence) = 0;

    /// Wait on the host for the fences to signals.
    /// `timeout` is in nanoseconds, can be set to `kTimeoutInfinite`.
    virtual SLANG_NO_THROW Result SLANG_MCALL waitForFences(
        uint32_t fenceCount,
        IFence** fences,
        uint64_t* values,
        bool waitForAll,
        uint64_t timeout) = 0;

    virtual SLANG_NO_THROW Result SLANG_MCALL getTextureAllocationInfo(
        const ITextureResource::Desc& desc, size_t* outSize, size_t* outAlignment) = 0;

    virtual SLANG_NO_THROW Result SLANG_MCALL getTextureRowAlignment(size_t* outAlignment) = 0;
};

#define SLANG_UUID_IDevice                                                               \
    {                                                                                    \
          0x715bdf26, 0x5135, 0x11eb, { 0xAE, 0x93, 0x02, 0x42, 0xAC, 0x13, 0x00, 0x02 } \
    }


class IPipelineCreationAPIDispatcher : public ISlangUnknown
{
public:
    virtual SLANG_NO_THROW Result SLANG_MCALL createComputePipelineState(
        IDevice* device,
        slang::IComponentType* program,
        void* pipelineDesc,
        void** outPipelineState) = 0;
    virtual SLANG_NO_THROW Result SLANG_MCALL createGraphicsPipelineState(
        IDevice* device,
        slang::IComponentType* program,
        void* pipelineDesc,
        void** outPipelineState) = 0;
    virtual SLANG_NO_THROW Result SLANG_MCALL
        beforeCreateRayTracingState(IDevice* device, slang::IComponentType* program) = 0;
    virtual SLANG_NO_THROW Result SLANG_MCALL
        afterCreateRayTracingState(IDevice* device, slang::IComponentType* program) = 0;
};
#define SLANG_UUID_IPipelineCreationAPIDispatcher                                     \
    {                                                                                 \
        0xc3d5f782, 0xeae1, 0x4da6, { 0xab, 0x40, 0x75, 0x32, 0x31, 0x2, 0xb7, 0xdc } \
    }


// Global public functions

extern "C"
{
    /// Checks if format is compressed
    SLANG_GFX_API bool gfxIsCompressedFormat(Format format);

    /// Checks if format is typeless
    SLANG_GFX_API bool gfxIsTypelessFormat(Format format);

    /// Gets information about the format 
    SLANG_GFX_API SlangResult gfxGetFormatInfo(Format format, FormatInfo* outInfo);

    /// Given a type returns a function that can construct it, or nullptr if there isn't one
    SLANG_GFX_API SlangResult SLANG_MCALL
        gfxCreateDevice(const IDevice::Desc* desc, IDevice** outDevice);

    /// Sets a callback for receiving debug messages.
    /// The layer does not hold a strong reference to the callback object.
    /// The user is responsible for holding the callback object alive.
    SLANG_GFX_API SlangResult SLANG_MCALL
        gfxSetDebugCallback(IDebugCallback* callback);

    /// Enables debug layer. The debug layer will check all `gfx` calls and verify that uses are valid.
    SLANG_GFX_API void SLANG_MCALL gfxEnableDebugLayer();

    SLANG_GFX_API const char* SLANG_MCALL gfxGetDeviceTypeName(DeviceType type);
}

// Extended descs.
struct D3D12DeviceExtendedDesc
{
    StructType structType = StructType::D3D12ExtendedDesc;
    const char* rootParameterShaderAttributeName = nullptr;
    bool debugBreakOnD3D12Error = false;
};

}


using namespace Slang;
using namespace gfx;

#endif


#ifdef SLANG_PRELUDE_NAMESPACE
using namespace SLANG_PRELUDE_NAMESPACE;
#endif


#line 6 "./examples/heterogeneous-hello-world/main.slang"
int32_t main()
{
    printf("%s", ((String("hello world"))).getBuffer());
    return int(0);
}

