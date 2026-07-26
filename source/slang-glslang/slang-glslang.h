// slang-glslang.h
#ifndef SLANG_GLSLANG_H_INCLUDED
#define SLANG_GLSLANG_H_INCLUDED

#include <cstdint>
#include <cstring>
#include <memory>
#include <stddef.h>

// Decorate the entry points that make up the `slang-glslang` ABI.
//
// `slang-glslang` can be built in two ways. By default it is a runtime-loaded module, and
// its entry points have to be exported from the shared library so that
// `ISlangSharedLibrary::findFuncByName` can find them by name at runtime. When
// `SLANG_EMBED_SLANG_GLSLANG` is enabled it is instead built as a static library that is
// linked directly into `slang-compiler`; in that build there is no shared library to export
// from, the entry points are called through the ordinary prototypes declared at the bottom
// of this header, and any export decoration would be wrong (on MSVC, `__declspec(dllexport)`
// on a declaration in a translation unit that does not define the function is an error).
#ifdef SLANG_GLSLANG_STATIC
#define SLANG_GLSLANG_EXPORT
#elif defined(_MSC_VER)
#define SLANG_GLSLANG_EXPORT __declspec(dllexport)
#else
#define SLANG_GLSLANG_EXPORT __attribute__((__visibility__("default")))
#endif

typedef void (*glslang_OutputFunc)(void const* data, size_t size, void* userData);

enum
{
    GLSLANG_ACTION_COMPILE_GLSL_TO_SPIRV,
    GLSLANG_ACTION_DISSASSEMBLE_SPIRV,
    GLSLANG_ACTION_OPTIMIZE_SPIRV,
};

struct glsl_SPIRVVersion
{
    int major, minor, patch;
};

// clang-format off

#define SLANG_GLSLANG_COMPILE_REQUEST_1_0(x)  \
    x(sourcePath) \
    x(inputBegin) \
    x(inputEnd) \
    x(diagnosticFunc) \
    x(diagnosticUserData) \
    x(outputFunc) \
    x(outputUserData) \
    x(slangStage) \
    x(action) \
    x(optimizationLevel) \
    x(debugInfoType)

#define SLANG_GLSLANG_FIELD_COPY(name) name = in.name;

// clang-format on

// Pre-declare
struct glslang_CompileRequest_1_1;

// 1.0 version
struct glslang_CompileRequest_1_0
{
    void set(const glslang_CompileRequest_1_1& in);

    char const* sourcePath;

    void const* inputBegin;
    void const* inputEnd;

    glslang_OutputFunc diagnosticFunc;
    void* diagnosticUserData;

    glslang_OutputFunc outputFunc;
    void* outputUserData;

    int slangStage;

    unsigned action;

    unsigned optimizationLevel;
    unsigned debugInfoType;
};

// 1.1 version
struct glslang_CompileRequest_1_1
{
    /// Set from 1.0
    void set(const glslang_CompileRequest_1_0& in);

    size_t sizeInBytes; ///< Size in bytes of this structure

    // START! Embed the glslang_CompileRequest_1_0 fields
    char const* sourcePath;

    void const* inputBegin;
    void const* inputEnd;

    glslang_OutputFunc diagnosticFunc;
    void* diagnosticUserData;

    glslang_OutputFunc outputFunc;
    void* outputUserData;

    int slangStage;

    unsigned action;

    unsigned optimizationLevel;
    unsigned debugInfoType;
    // END! Embed the glslang_CompileRequest_1_0 fields

    const char* spirvTargetName;    /// A valid TargetName. If null will use universal based on the
                                    /// spirVersion.
    glsl_SPIRVVersion spirvVersion; ///< The SPIR-V version. If all are 0 will use the default which
                                    ///< is 1.2 currently
};

// 1.2 version
struct glslang_CompileRequest_1_2
{
    /// Set from 1.1
    void set(const glslang_CompileRequest_1_1& in);

    size_t sizeInBytes; ///< Size in bytes of this structure

    // START! Embed the glslang_CompileRequest_1_0 fields
    char const* sourcePath;

    void const* inputBegin;
    void const* inputEnd;

    glslang_OutputFunc diagnosticFunc;
    void* diagnosticUserData;

    glslang_OutputFunc outputFunc;
    void* outputUserData;

    int slangStage;

    unsigned action;

    unsigned optimizationLevel;
    unsigned debugInfoType;
    // END! Embed the glslang_CompileRequest_1_0 fields

    const char* spirvTargetName;    /// A valid TargetName. If null will use universal based on the
                                    /// spirVersion.
    glsl_SPIRVVersion spirvVersion; ///< The SPIR-V version. If all are 0 will use the default which
                                    ///< is 1.2 currently

    // glslang_CompileRequest_1_2 fields
    const char* entryPointName; // The name of the entrypoint that will appear in output spirv.
};

inline void glslang_CompileRequest_1_0::set(const glslang_CompileRequest_1_1& in)
{
    SLANG_GLSLANG_COMPILE_REQUEST_1_0(SLANG_GLSLANG_FIELD_COPY)
}

inline void glslang_CompileRequest_1_1::set(const glslang_CompileRequest_1_0& in)
{
    SLANG_GLSLANG_COMPILE_REQUEST_1_0(SLANG_GLSLANG_FIELD_COPY)
}

inline void glslang_CompileRequest_1_2::set(const glslang_CompileRequest_1_1& in)
{
    memcpy(this, &in, sizeof(in));
}

typedef struct glslang_LinkRequest_t
{
    const uint32_t** modules;    // Input: array of pointers to SPIR-V modules
    const uint32_t* moduleSizes; // Input: array of sizes of SPIR-V modules in 32-bit words
    int moduleCount;             // Input: number of modules in the array
    const uint32_t* linkResult;  // Output: pointer to linked SPIR-V module
    size_t linkResultSize;       // Output: size of the linked SPIR-V module in 32-bit words
} glslang_LinkRequest;

typedef int (*glslang_CompileFunc_1_0)(glslang_CompileRequest_1_0* request);
typedef int (*glslang_CompileFunc_1_1)(glslang_CompileRequest_1_1* request);
typedef int (*glslang_CompileFunc_1_2)(glslang_CompileRequest_1_2* request);
typedef bool (*glslang_ValidateSPIRVFunc)(const uint32_t* contents, int contentsSize);
typedef bool (*glslang_DisassembleSPIRVFunc)(const uint32_t* contents, int contentsSize);
typedef bool (*glslang_DisassembleSPIRVWithResultFunc)(
    const uint32_t* contents,
    int contentsSize,
    char** outString);
typedef void (*glslang_FreeDisassemblyFunc)(char* disassembly);
typedef int (*glslang_LinkSPIRVFunc)(glslang_LinkRequest* request);

#ifdef SLANG_GLSLANG_STATIC
// Declare the `slang-glslang` entry points for the static build.
//
// In the default (runtime-loaded module) build these symbols are only ever reached through
// `ISlangSharedLibrary::findFuncByName`, so no consumer needs a prototype. When
// `slang-glslang` is linked statically into `slang-compiler`, `slang-glslang-compiler.cpp`
// binds its function-pointer table to these symbols directly, so it needs declarations whose
// signatures match the definitions in `slang-glslang.cpp` exactly. Declaring them here rather
// than in the consumer keeps a single source of truth for the ABI: the typedefs above and
// these prototypes are checked against the definitions by the compiler when `slang-glslang.cpp`
// includes this header.
extern "C"
{
    SLANG_GLSLANG_EXPORT int glslang_compile(glslang_CompileRequest_1_0* request);
    SLANG_GLSLANG_EXPORT int glslang_compile_1_1(glslang_CompileRequest_1_1* request);
    SLANG_GLSLANG_EXPORT int glslang_compile_1_2(glslang_CompileRequest_1_2* request);
    SLANG_GLSLANG_EXPORT bool glslang_validateSPIRV(const uint32_t* contents, int contentsSize);
    SLANG_GLSLANG_EXPORT bool glslang_disassembleSPIRV(const uint32_t* contents, int contentsSize);
    SLANG_GLSLANG_EXPORT bool glslang_disassembleSPIRVWithResult(
        const uint32_t* contents,
        int contentsSize,
        char** outString);
    SLANG_GLSLANG_EXPORT void glslang_freeDisassembly(char* disassembly);
    SLANG_GLSLANG_EXPORT int glslang_linkSPIRV(glslang_LinkRequest* request);
}
#endif

#endif
