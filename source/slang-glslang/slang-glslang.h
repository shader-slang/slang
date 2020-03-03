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

enum glslang_SPIRVStyle 
{
    GLSLANG_SPIRV_STYLE_UNKNOWN,
    GLSLANG_SPIRV_STYLE_UNIVERSAL,
};

struct glslang_SPIRVVersion
{
    unsigned char style;                    /// One of glslang_SPIRVStyle
    unsigned char majorVersion;
    unsigned char minorVersion;
};

struct glslang_CompileRequest
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

    glslang_SPIRVVersion spirvVersion;       
};

typedef int (*glslang_CompileFunc)(glslang_CompileRequest* request);

#endif
