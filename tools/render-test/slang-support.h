// slang-support.h
#pragma once

#include "render.h"

#include <slang.h>

#include "shader-input-layout.h"

namespace renderer_test {

struct ShaderCompilerUtil
{
    struct Input
    {
        SlangCompileTarget      target;
        SlangSourceLanguage     sourceLanguage;
        SlangPassThrough        passThrough;
        char const*             profile;
        const char**            args;
        int                     argCount;
    };

    struct Output
    {
        void set(PipelineType pipelineType, const ShaderProgram::KernelDesc* inKernelDescs, int kernelDescCount)
        {
            kernelDescs.clear();
            kernelDescs.addRange(inKernelDescs, kernelDescCount);
            desc.pipelineType = pipelineType;
            desc.kernels = kernelDescs.getBuffer();
            desc.kernelCount = kernelDescCount;
        }
        void reset()
        {
            kernelDescs.clear();
            if (request && session)
            {
                spDestroyCompileRequest(request);
            }
            session = nullptr;
            request = nullptr;
        }
        ~Output()
        {
            if (request && session)
            {
                 spDestroyCompileRequest(request);
            }
        }
        List<ShaderProgram::KernelDesc> kernelDescs;
        ShaderProgram::Desc desc;
        SlangCompileRequest* request = nullptr;
        SlangSession* session = nullptr;
    };

    static SlangResult compileProgram(SlangSession* session, const Input& input, const ShaderCompileRequest& request, Output& out);
};


} // renderer_test
