// slang-glslang.h
#ifndef SLANG_GLSLANG_H_INCLUDED
#define SLANG_GLSLANG_H_INCLUDED

#include <stddef.h>
#include <memory>
#include <cstring>

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

// Pre-declare
struct glslang_CompileRequest_1_1;

// 1.0 version
struct glslang_CompileRequest_1_0
{
    void set(const glslang_CompileRequest_1_1& in);

    char const*         sourcePath;

    void const*         inputBegin;
    void const*         inputEnd;

    glslang_OutputFunc  diagnosticFunc;
    void*               diagnosticUserData;

    glslang_OutputFunc  outputFunc;
    void*               outputUserData;

    int                 slangStage;

    unsigned            action;

    unsigned            optimizationLevel;
    unsigned            debugInfoType;   
};

// 1.1 version
struct glslang_CompileRequest_1_1
{
        /// Set from 1.0 
    void set(const glslang_CompileRequest_1_0& in);

    size_t              sizeInBytes;            ///< Size in bytes of this structure

    // START! Embed the glslang_CompileRequest_1_0 fields
    char const*         sourcePath;

    void const*         inputBegin;
    void const*         inputEnd;

    glslang_OutputFunc  diagnosticFunc;
    void*               diagnosticUserData;

    glslang_OutputFunc  outputFunc;
    void*               outputUserData;

    int                 slangStage;

    unsigned            action;

    unsigned            optimizationLevel;
    unsigned            debugInfoType;
    // END! Embed the glslang_CompileRequest_1_0 fields

    const char*         spirvTargetName;            /// A valid TargetName. If null will use universal based on the spirVersion.
    glsl_SPIRVVersion   spirvVersion;               ///< The SPIR-V version. If all are 0 will use the default which is 1.2 currently
};

// 1.2 version
struct glslang_CompileRequest_1_2
{
    /// Set from 1.1 
    void set(const glslang_CompileRequest_1_1& in);

    size_t              sizeInBytes;            ///< Size in bytes of this structure

    // START! Embed the glslang_CompileRequest_1_0 fields
    char const* sourcePath;

    void const* inputBegin;
    void const* inputEnd;

    glslang_OutputFunc  diagnosticFunc;
    void* diagnosticUserData;

    glslang_OutputFunc  outputFunc;
    void* outputUserData;

    int                 slangStage;

    unsigned            action;

    unsigned            optimizationLevel;
    unsigned            debugInfoType;
    // END! Embed the glslang_CompileRequest_1_0 fields

    const char* spirvTargetName;            /// A valid TargetName. If null will use universal based on the spirVersion.
    glsl_SPIRVVersion   spirvVersion;               ///< The SPIR-V version. If all are 0 will use the default which is 1.2 currently

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

typedef int (*glslang_CompileFunc_1_0)(glslang_CompileRequest_1_0* request);
typedef int (*glslang_CompileFunc_1_1)(glslang_CompileRequest_1_1* request);
typedef int (*glslang_CompileFunc_1_2)(glslang_CompileRequest_1_2* request);

#endif
