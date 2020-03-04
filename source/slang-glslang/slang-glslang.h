// slang-glslang.h
#ifndef SLANG_GLSLANG_H_INCLUDED
#define SLANG_GLSLANG_H_INCLUDED

#include <stddef.h>

typedef void (*glslang_OutputFunc)(void const* data, size_t size, void* userData);

enum
{
    GLSLANG_ACTION_COMPILE_GLSL_TO_SPIRV,
    GLSLANG_ACTION_DISSASSEMBLE_SPIRV,
};

struct glsl_SPIRVVersion
{
    unsigned char major, minor, patch;
    unsigned char misc;                     ///< Set to 0 if not used
};

struct glslang_CompileRequest_1_0
{
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


struct glslang_CompileRequest_1_1
{
    size_t              sizeInBytes;            ///< Size in bytes of this structure

    glslang_CompileRequest_1_0 request_1_0;

    const char*         spirvTargetName;            /// A valid TargetName. If null will use universal based on the spirVersion.
    glsl_SPIRVVersion   spirvVersion;               ///< The SPIR-V version. If all are 0 will use the default which is 1.2 currently
};

typedef int (*glslang_CompileFunc_1_0)(glslang_CompileRequest_1_0* request);
typedef int (*glslang_CompileFunc_1_1)(glslang_CompileRequest_1_1* request);

#endif
