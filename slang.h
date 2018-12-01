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

#if defined(SLANG_DYNAMIC)
    #if defined(_MSC_VER)
        #ifdef SLANG_DYNAMIC_EXPORT
            #define SLANG_API __declspec(dllexport)
        #else
            #define SLANG_API __declspec(dllimport)
        #endif
    #else
        // TODO: need to consider compiler capabilities
//        #ifdef SLANG_DYNAMIC_EXPORT
            #define SLANG_API __attribute__((__visibility__("default")))
//        #endif
    #endif
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
    typedef intptr_t    SlangInt;
    typedef uintptr_t   SlangUInt;
    
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

    typedef int SlangPassThrough;
    enum
    {
        SLANG_PASS_THROUGH_NONE,
        SLANG_PASS_THROUGH_FXC,
        SLANG_PASS_THROUGH_DXC,
        SLANG_PASS_THROUGH_GLSLANG,
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

    typedef int SlangSourceLanguage;
    enum
    {
        SLANG_SOURCE_LANGUAGE_UNKNOWN,
        SLANG_SOURCE_LANGUAGE_SLANG,
        SLANG_SOURCE_LANGUAGE_HLSL,
        SLANG_SOURCE_LANGUAGE_GLSL,
    };

    typedef unsigned int SlangProfileID;
    enum
    {
        SLANG_PROFILE_UNKNOWN,
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

        // alias:
        SLANG_STAGE_PIXEL = SLANG_STAGE_FRAGMENT,
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
#define SLANG_FAIL                          SLANG_MAKE_ERROR(SLANG_FACILITY_WIN_INTERFACE, 5)

#define SLANG_MAKE_WIN_INTERFACE_ERROR(code)    SLANG_MAKE_ERROR(SLANG_FACILITY_WIN_INTERFACE, code)
#define SLANG_MAKE_WIN_API_ERROR(code)          SLANG_MAKE_ERROR(SLANG_FACILITY_WIN_API, code)

    //! Functionality is not implemented
#define SLANG_E_NOT_IMPLEMENTED             SLANG_MAKE_WIN_INTERFACE_ERROR(1)
    //! Interface not be found
#define SLANG_E_NO_INTERFACE                SLANG_MAKE_WIN_INTERFACE_ERROR(2)
    //! Operation was aborted (did not correctly complete)
#define SLANG_E_ABORT                       SLANG_MAKE_WIN_INTERFACE_ERROR(4)

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

    /** Base interface for components exchanged through the API.

    This interface definition is compatible with the COM `IUnknown`,
    and uses the same UUID, but Slang does not require applications
    to use or initialize COM.
    */
    struct ISlangUnknown
    {
    public:
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
    #define SLANG_UUID_ISlangUnknown { 0x00000000, 0x0000, 0x0000, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46 }

    /** A "blob" of binary data.

    This interface definition is compatible with the `ID3DBlob` and `ID3D10Blob` interfaces.
    */
    struct ISlangBlob : public ISlangUnknown
    {
    public:
        virtual SLANG_NO_THROW void const* SLANG_MCALL getBufferPointer() = 0;
        virtual SLANG_NO_THROW size_t SLANG_MCALL getBufferSize() = 0;
    };
    #define SLANG_UUID_ISlangBlob { 0x8BA5FB08, 0x5195, 0x40e2, 0xAC, 0x58, 0x0D, 0x98, 0x9C, 0x3A, 0x01, 0x02 }

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
    public:
        /** Load a file from `path` and return a blob of its contents
        @param path The path to load from, as a null-terminated UTF-8 string.
        @param outBlob A destination pointer to receive the blob of the file contents.
        @returns A `SlangResult` to indicate success or failure in loading the file.

        If load is successful, the implementation should create a blob to hold
        the file's content, store it to `outBlob`, and return 0.
        If the load fails, the implementation should return a failure status
        (any negative value will do).
        */
        virtual SLANG_NO_THROW SlangResult SLANG_MCALL loadFile(
            char const*     path,
            ISlangBlob** outBlob) = 0;
    };
    #define SLANG_UUID_ISlangFileSystem { 0x003A09FC, 0x3A4D, 0x4BA0, 0xAD, 0x60, 0x1F, 0xD8, 0x63, 0xA9, 0x15, 0xAB }


    typedef void(*SlangFuncPtr)(void);

    /** An interface that can be used to encapsulate access to a shared library. An implementaion 
    does not have to implement the library as a shared library. 
    */
    struct ISlangSharedLibrary: public ISlangUnknown
    {
        public: 
            /** Get a function by name. If the library is unloaded will only return nullptr. 
            @param name The name of the function 
            @return The function pointer related to the name or nullptr if not found 
            */
        virtual SLANG_NO_THROW SlangFuncPtr SLANG_MCALL findFuncByName(char const* name) = 0;
    };
    #define SLANG_UUID_ISlangSharedLibrary { 0x9c9d5bc5, 0xeb61, 0x496f,{ 0x80, 0xd7, 0xd1, 0x47, 0xc4, 0xa2, 0x37, 0x30 } };

    struct ISlangSharedLibraryLoader: public ISlangUnknown
    {
        public:
        virtual SLANG_NO_THROW SlangResult SLANG_MCALL loadSharedLibrary(
            const char*     path,
            ISlangSharedLibrary** sharedLibraryOut) = 0;
    };
    #define SLANG_UUID_ISlangSharedLibraryLoader { 0x6264ab2b, 0xa3e8, 0x4a06,{ 0x97, 0xf1, 0x49, 0xbc, 0x2d, 0x2a, 0xb1, 0x4d } };
    
    /* Type that identifies how a path should be interpreted */
    typedef unsigned int SlangPathType;
    enum
    {
        SLANG_PATH_TYPE_DIRECTORY,      /**< Path specified specifies a directory. */
        SLANG_PATH_TYPE_FILE,           /**< Path specified is to a file. */
    };

    /** An extended file system abstraction.
    
    Implementing and using this interface over ISlangFileSystem gives much more control over how paths
    are managed, as well as how it is determined if two files 'are the same'.

    All paths as input char*, or output as ISlangBlobs are always encoded as UTF-8 strings.
    Blobs that contain strings are always zero terminated.
    */
    struct ISlangFileSystemExt : public ISlangFileSystem
    {
    public:
        /** Get a canonical path which uniquely identifies an object of the file system.
           
        Given a path, returns a 'canonical' path which will return the same path for the same file/directory. 
        The canonical path is used to compare if two includes are the same file. 
        The string for the canonical path is held zero terminated in the ISlangBlob of canonicalPathOut. 
   
        Note that a canonical path doesn't *have* to be a 'canonical' path, or a path at all
        - it can just be a string that uniquely identifies a file. For example another possible mechanism
        could be to store the filename combined with the file date time to uniquely identify it.
     
        The client must ensure the blob be released when no longer used, otherwise memory will leak.

        @param path
        @param canonicalPathOut
        @returns A `SlangResult` to indicate success or failure getting the canonical path.
        */
        virtual SLANG_NO_THROW SlangResult SLANG_MCALL getCanoncialPath(
            const char* path,
            ISlangBlob** canonicalPathOut) = 0;

        /** Get a path relative to a 'from' path.

        The client must ensure the blob be released when no longer used, otherwise memory will leak.

        @param fromPathType How to interpret the from path - as a file or a directory.
        @param fromPath The from path. 
        @param path Path to be determined relative to the fromPath
        @param pathOut Holds the string which is the relative path. The string is held in the blob zero terminated.  
        @returns A `SlangResult` to indicate success or failure in loading the file.
        */
        virtual SLANG_NO_THROW SlangResult SLANG_MCALL calcRelativePath(
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
    };

    #define SLANG_UUID_ISlangFileSystemExt { 0x5fb632d2, 0x979d, 0x4481, { 0x9f, 0xee, 0x66, 0x3c, 0x3f, 0x14, 0x49, 0xe1 } }

    /*!
    @brief An instance of the Slang library.
    */
    typedef struct SlangSession SlangSession;

    /*!
    @brief A request for one or more compilation actions to be performed.
    */
    typedef struct SlangCompileRequest SlangCompileRequest;

    /*!
    @brief Initialize an instance of the Slang library.
    */
    SLANG_API SlangSession* spCreateSession(const char* deprecated = 0);

    /*!
    @brief Clean up after an instance of the Slang library.
    */
    SLANG_API void spDestroySession(
        SlangSession*   session);

    /*!
    @brief Set the session shared library loader. If this changes the loader, it may cause shared libraries to be unloaded
    @param session Session to set the loader on
    @param loader The loader to set. Setting nullptr sets the default loader. 
    */
    SLANG_API void spSessionSetSharedLibraryLoader(
        SlangSession*               session,
        ISlangSharedLibraryLoader*  loader);

    /*!
    @brief Gets the currently set shared library loader
    @param session Session to get the loader from
    @return Gets the currently set loader. If returns nullptr, it's the default loader
    */
    SLANG_API ISlangSharedLibraryLoader* spSessionGetSharedLibraryLoader(
        SlangSession*   session);

    /*!
    @brief Returns SLANG_OK if a the compilation target is supported for this session
    @param session Session
    @param target The compilation target to test
    @return SLANG_OK if the target is available
            SLANG_E_NOT_IMPLEMENTED if not implemented in this build
            SLANG_E_NOT_FOUND if other resources (such as shared libraries) required to make target work could not be found
            SLANG_FAIL other kinds of failures */
    SLANG_API SlangResult spSessionCheckCompileTargetSupport(
        SlangSession*       session,
        SlangCompileTarget  target);

    /*!
    @brief Add new builtin declarations to be used in subsequent compiles.
    */
    SLANG_API void spAddBuiltins(
        SlangSession*   session,
        char const*     sourcePath,
        char const*     sourceString);

    /*!
    @brief Create a compile request.
    */
    SLANG_API SlangCompileRequest* spCreateCompileRequest(
        SlangSession* session);

    /*!
    @brief Destroy a compile request.
    */
    SLANG_API void spDestroyCompileRequest(
        SlangCompileRequest*    request);

    /** Set the filesystem hook to use for a compile request

    The provided `fileSystem` will be used to load any files that
    need to be loaded during processing of the compile `request`.
    This includes:

      - Source files loaded via `spAddTranslationUnitSourceFile`
      - Files referenced via `#include`
      - Files loaded to resolve `#import` operations

    */
    SLANG_API void spSetFileSystem(
        SlangCompileRequest*    request,
        ISlangFileSystem*       fileSystem);


    /*!
    @brief Set flags to be used for compilation.
    */
    SLANG_API void spSetCompileFlags(
        SlangCompileRequest*    request,
        SlangCompileFlags       flags);

    /*!
    @brief Set whether to dump intermediate results (for debugging) or not.
    */
    SLANG_API void spSetDumpIntermediates(
        SlangCompileRequest*    request,
        int                     enable);

    /*!
    @brief Set whether (and how) `#line` directives should be output.
    */
    SLANG_API void spSetLineDirectiveMode(
        SlangCompileRequest*    request,
        SlangLineDirectiveMode  mode);

    /*!
    @brief Sets the target for code generation.
    @param request The compilation context.
    @param target The code generation target. Possible values are:
    - SLANG_GLSL. Generates GLSL code.
    - SLANG_HLSL. Generates HLSL code.
    - SLANG_SPIRV. Generates SPIR-V code.
    */
    SLANG_API void spSetCodeGenTarget(
        SlangCompileRequest*    request,
        SlangCompileTarget target);

    /*!
    @brief Add a code-generation target to be used.
    */
    SLANG_API int spAddCodeGenTarget(
        SlangCompileRequest*    request,
        SlangCompileTarget      target);

    SLANG_API void spSetTargetProfile(
        SlangCompileRequest*    request,
        int                     targetIndex,
        SlangProfileID          profile);

    SLANG_API void spSetTargetFlags(
        SlangCompileRequest*    request,
        int                     targetIndex,
        SlangTargetFlags        flags);

    /*!
    @brief Set the floating point mode (e.g., precise or fast) to use a target.
    */
    SLANG_API void spSetTargetFloatingPointMode(
        SlangCompileRequest*    request,
        int                     targetIndex,
        SlangFloatingPointMode  mode);

    /* DEPRECATED: use `spSetMatrixLayoutMode` instead. */
    SLANG_API void spSetTargetMatrixLayoutMode(
        SlangCompileRequest*    request,
        int                     targetIndex,
        SlangMatrixLayoutMode   mode);

    SLANG_API void spSetMatrixLayoutMode(
        SlangCompileRequest*    request,
        SlangMatrixLayoutMode   mode);

    /*!
    @brief Set the container format to be used for binary output.
    */
    SLANG_API void spSetOutputContainerFormat(
        SlangCompileRequest*    request,
        SlangContainerFormat    format);

    SLANG_API void spSetPassThrough(
        SlangCompileRequest*    request,
        SlangPassThrough        passThrough);

    typedef void(*SlangDiagnosticCallback)(
        char const* message,
        void*       userData);

    SLANG_API void spSetDiagnosticCallback(
        SlangCompileRequest*    request,
        SlangDiagnosticCallback callback,
        void const*             userData);

    /*!
    @brief Add a path to use when searching for referenced files.
    This will be used for both `#include` directives and also for explicit `__import` declarations.
    @param ctx The compilation context.
    @param searchDir The additional search directory.
    */
    SLANG_API void spAddSearchPath(
        SlangCompileRequest*    request,
        const char*             searchDir);

    /*!
    @brief Add a macro definition to be used during preprocessing.
    @param key The name of the macro to define.
    @param value The value of the macro to define.
    */
    SLANG_API void spAddPreprocessorDefine(
        SlangCompileRequest*    request,
        const char*             key,
        const char*             value);

    /*!
    @brief Set options using arguments as if specified via command line.
    @return Returns SlangResult. On success SLANG_SUCCEEDED(result) is true.
    */
    SLANG_API SlangResult spProcessCommandLineArguments(
        SlangCompileRequest*    request,
        char const* const*      args,
        int                     argCount);

    /** Add a distinct translation unit to the compilation request

    `name` is optional.
    Returns the zero-based index of the translation unit created.
    */
    SLANG_API int spAddTranslationUnit(
        SlangCompileRequest*    request,
        SlangSourceLanguage     language,
        char const*             name);

    /** Add a preprocessor definition that is scoped to a single translation unit.

    @param translationUnitIndex The index of the translation unit to get the definition.
    @param key The name of the macro to define.
    @param value The value of the macro to define.
    */
    SLANG_API void spTranslationUnit_addPreprocessorDefine(
        SlangCompileRequest*    request,
        int                     translationUnitIndex,
        const char*             key,
        const char*             value);


    /** Add a source file to the given translation unit.

    If a user-defined file system has been specified via
    `spSetFileSystem`, then it will be used to load the
    file at `path`. Otherwise, Slang will use the OS
    file system.

    This function does *not* search for a file using
    the registered search paths (`spAddSearchPath`),
    and instead using the given `path` as-is.
    */
    SLANG_API void spAddTranslationUnitSourceFile(
        SlangCompileRequest*    request,
        int                     translationUnitIndex,
        char const*             path);

    /** Add a source string to the given translation unit.

    @param request The compile request that owns the translation unit.
    @param translationUnitIndex The index of the translation unit to add source to.
    @param path The file-system path that should be assumed for the source code.
    @param source A null-terminated UTF-8 encoded string of source code.

    The implementation will make a copy of the source code data.
    An application may free the buffer immediately after this call returns.

    The `path` will be used in any diagnostic output, as well
    as to determine the base path when resolving relative
    `#include`s.
    */
    SLANG_API void spAddTranslationUnitSourceString(
        SlangCompileRequest*    request,
        int                     translationUnitIndex,
        char const*             path,
        char const*             source);


    /** Add a source string to the given translation unit.

    @param request The compile request that owns the translation unit.
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
    SLANG_API void spAddTranslationUnitSourceStringSpan(
        SlangCompileRequest*    request,
        int                     translationUnitIndex,
        char const*             path,
        char const*             sourceBegin,
        char const*             sourceEnd);

    /** Add a blob of source code to the given translation unit.

    @param request The compile request that owns the translation unit.
    @param translationUnitIndex The index of the translation unit to add source to.
    @param path The file-system path that should be assumed for the source code.
    @param sourceBlob A blob containing UTF-8 encoded source code.
    @param sourceEnd A pointer to to the end of the buffer specified in `sourceBegin`

    The compile request will retain a reference to the blob.

    The `path` will be used in any diagnostic output, as well
    as to determine the base path when resolving relative
    `#include`s.
    */
    SLANG_API void spAddTranslationUnitSourceBlob(
        SlangCompileRequest*    request,
        int                     translationUnitIndex,
        char const*             path,
        ISlangBlob*             sourceBlob);

    /** Look up a compilation profile by name.

    For example, one could look up the string `"ps_5_0"` to find the corresponding target ID.
    */
    SLANG_API SlangProfileID spFindProfile(
        SlangSession*   session,
        char const*     name);

    /** Add an entry point in a particular translation unit
    */
    SLANG_API int spAddEntryPoint(
        SlangCompileRequest*    request,
        int                     translationUnitIndex,
        char const*             name,
        SlangStage              stage);

    /** Add an entry point in a particular translation unit,
        with additional arguments that specify the concrete
        type names for global generic type parameters.
    */
    SLANG_API int spAddEntryPointEx(
        SlangCompileRequest*    request,
        int                     translationUnitIndex,
        char const*             name,
        SlangStage              stage,
        int                     genericTypeNameCount,
        char const**            genericTypeNames);

    /** Execute the compilation request.

    @returns  SlangResult, SLANG_OK on success. Use SLANG_SUCCEEDED() and SLANG_FAILED() to test SlangResult.
    */
    SLANG_API SlangResult spCompile(
        SlangCompileRequest*    request);


    /** Get any diagnostic messages reported by the compiler.

    @returns A null-terminated UTF-8 encoded string of diagnostic messages.

    The returned pointer is only guaranteed to be valid
    until `request` is destroyed. Applications that wish to
    hold on to the diagnostic output for longer should use
    `spGetDiagnosticOutputBlob`.
    */
    SLANG_API char const* spGetDiagnosticOutput(
        SlangCompileRequest*    request);

    /** Get diagnostic messages reported by the compiler.

    @param request The compile request to get output from.
    @param outBlob A pointer to receive a blob holding a nul-terminated UTF-8 encoded string of diagnostic messages.
    @returns A `SlangResult` indicating success or failure.
    */
    SLANG_API SlangResult spGetDiagnosticOutputBlob(
        SlangCompileRequest*    request,
        ISlangBlob**            outBlob);


    /** Get the number of files that this compilation depended on.

    This includes both the explicit source files, as well as any
    additional files that were transitively referenced (e.g., via
    a `#include` directive).
    */
    SLANG_API int
    spGetDependencyFileCount(
        SlangCompileRequest*    request);

    /** Get the path to a file this compilation depended on.
    */
    SLANG_API char const*
    spGetDependencyFilePath(
        SlangCompileRequest*    request,
        int                     index);

    /** Get the number of translation units associated with the compilation request
    */
    SLANG_API int
    spGetTranslationUnitCount(
        SlangCompileRequest*    request);

    /** Get the output source code associated with a specific entry point.

    The lifetime of the output pointer is the same as `request`.
    */
    SLANG_API char const* spGetEntryPointSource(
        SlangCompileRequest*    request,
        int                     entryPointIndex);

    /** Get the output bytecode associated with a specific entry point.

    The lifetime of the output pointer is the same as `request`.
    */
    SLANG_API void const* spGetEntryPointCode(
        SlangCompileRequest*    request,
        int                     entryPointIndex,
        size_t*                 outSize);

    /** Get the output code associated with a specific entry point.

    @param entryPointIndex The index of the entry point to get code for.
    @param targetIndex The index of the target to get code for (default: zero).
    @param outBlob A pointer that will receive the blob of code
    @returns A `SlangResult` to indicate success or failure.

    The lifetime of the output pointer is the same as `request`.
    */
    SLANG_API SlangResult spGetEntryPointCodeBlob(
        SlangCompileRequest*    request,
        int                     entryPointIndex,
        int                     targetIndex,
        ISlangBlob**            outBlob);

    /** Get the output bytecode associated with an entire compile request.

    The lifetime of the output pointer is the same as `request`.
    */
    SLANG_API void const* spGetCompileRequestCode(
        SlangCompileRequest*    request,
        size_t*                 outSize);



    typedef struct SlangVM          SlangVM;
    typedef struct SlangVMModule    SlangVMModule;
    typedef struct SlangVMFunc      SlangVMFunc;
    typedef struct SlangVMThread    SlangVMThread;

    SLANG_API SlangVM* SlangVM_create();

    SLANG_API SlangVMModule* SlangVMModule_load(
        SlangVM*    vm,
        void const* bytecode,
        size_t      bytecodeSize);

    SLANG_API void* SlangVMModule_findGlobalSymbolPtr(
        SlangVMModule*  module,
        char const*     name);

    SLANG_API SlangVMThread* SlangVMThread_create(
        SlangVM*    vm);

    SLANG_API void SlangVMThread_beginCall(
        SlangVMThread*  thread,
        SlangVMFunc*    func);

    SLANG_API void SlangVMThread_setArg(
        SlangVMThread*  thread,
        SlangUInt       argIndex,
        void const*     data,
        size_t          size);

    SLANG_API void SlangVMThread_resume(
        SlangVMThread*  thread);

    /* Note(tfoley): working on new reflection interface...
    */

    typedef struct SlangReflection                  SlangReflection;
    typedef struct SlangReflectionEntryPoint        SlangReflectionEntryPoint;
    typedef struct SlangReflectionModifier          SlangReflectionModifier;
    typedef struct SlangReflectionType              SlangReflectionType;
    typedef struct SlangReflectionTypeLayout        SlangReflectionTypeLayout;
    typedef struct SlangReflectionVariable          SlangReflectionVariable;
    typedef struct SlangReflectionVariableLayout    SlangReflectionVariableLayout;
    typedef struct SlangReflectionTypeParameter     SlangReflectionTypeParameter;

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

        SLANG_RESOURCE_EXT_SHAPE_MASK       = 0xF0,
        SLANG_TEXTURE_ARRAY_FLAG            = 0x40,
        SLANG_TEXTURE_MULTISAMPLE_FLAG      = 0x80,

        SLANG_TEXTURE_1D_ARRAY              = SLANG_TEXTURE_1D   | SLANG_TEXTURE_ARRAY_FLAG,
        SLANG_TEXTURE_2D_ARRAY              = SLANG_TEXTURE_2D   | SLANG_TEXTURE_ARRAY_FLAG,
        SLANG_TEXTURE_CUBE_ARRAY            = SLANG_TEXTURE_CUBE | SLANG_TEXTURE_ARRAY_FLAG,

        SLANG_TEXTURE_2D_MULTISAMPLE        = SLANG_TEXTURE_2D | SLANG_TEXTURE_MULTISAMPLE_FLAG,
        SLANG_TEXTURE_2D_MULTISAMPLE_ARRAY  = SLANG_TEXTURE_2D | SLANG_TEXTURE_MULTISAMPLE_FLAG | SLANG_TEXTURE_ARRAY_FLAG,
    };

    typedef unsigned int SlangResourceAccess;
    enum
    {
        SLANG_RESOURCE_ACCESS_NONE,
        SLANG_RESOURCE_ACCESS_READ,
        SLANG_RESOURCE_ACCESS_READ_WRITE,
        SLANG_RESOURCE_ACCESS_RASTER_ORDERED,
        SLANG_RESOURCE_ACCESS_APPEND,
        SLANG_RESOURCE_ACCESS_CONSUME,
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

        //
        SLANG_PARAMETER_CATEGORY_COUNT,


        // DEPRECATED:
        SLANG_PARAMETER_CATEGORY_VERTEX_INPUT = SLANG_PARAMETER_CATEGORY_VARYING_INPUT,
        SLANG_PARAMETER_CATEGORY_FRAGMENT_OUTPUT = SLANG_PARAMETER_CATEGORY_VARYING_OUTPUT,
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

    // Type Reflection

    SLANG_API SlangTypeKind spReflectionType_GetKind(SlangReflectionType* type);
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
    SLANG_API size_t spReflectionTypeLayout_GetSize(SlangReflectionTypeLayout* type, SlangParameterCategory category);

    SLANG_API SlangReflectionVariableLayout* spReflectionTypeLayout_GetFieldByIndex(SlangReflectionTypeLayout* type, unsigned index);

    SLANG_API size_t spReflectionTypeLayout_GetElementStride(SlangReflectionTypeLayout* type, SlangParameterCategory category);
    SLANG_API SlangReflectionTypeLayout* spReflectionTypeLayout_GetElementTypeLayout(SlangReflectionTypeLayout* type);
    SLANG_API SlangReflectionVariableLayout* spReflectionTypeLayout_GetElementVarLayout(SlangReflectionTypeLayout* type);

    SLANG_API SlangParameterCategory spReflectionTypeLayout_GetParameterCategory(SlangReflectionTypeLayout* type);

    SLANG_API unsigned spReflectionTypeLayout_GetCategoryCount(SlangReflectionTypeLayout* type);
    SLANG_API SlangParameterCategory spReflectionTypeLayout_GetCategoryByIndex(SlangReflectionTypeLayout* type, unsigned index);

    SLANG_API SlangMatrixLayoutMode spReflectionTypeLayout_GetMatrixLayoutMode(SlangReflectionTypeLayout* type);

    SLANG_API int spReflectionTypeLayout_getGenericParamIndex(SlangReflectionTypeLayout* type);

    // Variable Reflection

    SLANG_API char const* spReflectionVariable_GetName(SlangReflectionVariable* var);
    SLANG_API SlangReflectionType* spReflectionVariable_GetType(SlangReflectionVariable* var);

    SLANG_API SlangReflectionModifier* spReflectionVariable_FindModifier(SlangReflectionVariable* var, SlangModifierID modifierID);

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

    // Shader Parameter Reflection

    typedef SlangReflectionVariableLayout SlangReflectionParameter;

    SLANG_API unsigned spReflectionParameter_GetBindingIndex(SlangReflectionParameter* parameter);
    SLANG_API unsigned spReflectionParameter_GetBindingSpace(SlangReflectionParameter* parameter);

    // Entry Point Reflection

    SLANG_API char const* spReflectionEntryPoint_getName(
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

#ifdef __cplusplus
}

/* Helper interfaces for C++ users */
namespace slang
{
#define SLANG_SAFE_BOOL(expr) \
    operator bool() const { return expr; }

    struct BufferReflection;
    struct TypeLayoutReflection;
    struct TypeReflection;
    struct VariableLayoutReflection;
    struct VariableReflection;

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
            Interface = SLANG_TYPE_KIND_INTERFACE
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

        // DEPRECATED:
        VertexInput = SLANG_PARAMETER_CATEGORY_VERTEX_INPUT,
        FragmentOutput = SLANG_PARAMETER_CATEGORY_FRAGMENT_OUTPUT,
    };

    struct TypeLayoutReflection
    {
        TypeReflection* getType()
        {
            return (TypeReflection*) spReflectionTypeLayout_GetType((SlangReflectionTypeLayout*) this);
        }

        TypeReflection::Kind getKind() { return getType()->getKind(); }

        size_t getSize(SlangParameterCategory category = SLANG_PARAMETER_CATEGORY_UNIFORM)
        {
            return spReflectionTypeLayout_GetSize((SlangReflectionTypeLayout*) this, category);
        }

        unsigned int getFieldCount()
        {
            return getType()->getFieldCount();
        }

        VariableLayoutReflection* getFieldByIndex(unsigned int index)
        {
            return (VariableLayoutReflection*) spReflectionTypeLayout_GetFieldByIndex((SlangReflectionTypeLayout*) this, index);
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
    };

    struct EntryPointReflection
    {
        char const* getName()
        {
            return spReflectionEntryPoint_getName((SlangReflectionEntryPoint*) this);
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
    };

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

        static ShaderReflection* get(SlangCompileRequest* request)
        {
            return (ShaderReflection*) spGetReflection(request);
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
    };
}

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


#define SLANG_ERROR_INSUFFICIENT_BUFFER -1
#define SLANG_ERROR_INVALID_PARAMETER -2

SLANG_API char const* spGetTranslationUnitSource(
    SlangCompileRequest*    request,
    int                     translationUnitIndex);

#ifdef __cplusplus
}
#endif

#endif
