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

struct glslang_CompileRequest
{
    size_t              sizeInBytes;            ///< Size in bytes of this structure

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

        /// Can be nullptr if not defined, else a string that identifies the spirvVersion 
        /// Values can be of the form major.minor as in "1.3" for SPIR-V
        /// Passed this way to allow for future availability of SPIR-V target types without
        /// breaking binary compatibility. 
    const char*const*   spirvVersions;
        /// Count of amount of spirv versions specified
    int spirvVersionsCount; 

};

typedef int (*glslang_CompileFunc)(glslang_CompileRequest* request);

#endif
