// glslang.h
#ifndef GLSLANG_H_INCLUDED
#define GLSLANG_H_INCLUDED

typedef void (*glslang_OutputFunc)(void const* data, size_t size, void* userData);

struct glslang_CompileRequest
{
    char const*         sourcePath;
    char const*         sourceText;

    glslang_OutputFunc  diagnosticFunc;
    void*               diagnosticUserData;

    glslang_OutputFunc  outputFunc;
    void*               outputUserData;

    int                 slangStage;

    bool                disassembleResult;
};

typedef int (*glslang_CompileFunc)(glslang_CompileRequest* request);

#endif
