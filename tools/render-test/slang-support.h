// slang-support.h
#pragma once

#include "render.h"

#include <slang.h>

#include "shader-input-layout.h"
#include "options.h"

namespace renderer_test {

struct ShaderCompilerUtil
{
    struct Input
    {
        SlangCompileTarget      target;
        SlangSourceLanguage     sourceLanguage;
        SlangPassThrough        passThrough;
        PipelineType            pipelineType = PipelineType::Unknown;
        char const*             profile;
        const char**            args;
        int                     argCount;
    };

    struct Output
    {
        void set(PipelineType pipelineType, const ShaderProgram::KernelDesc* inKernelDescs, Slang::Index kernelDescCount)
        {
            kernelDescs.clear();
            kernelDescs.addRange(inKernelDescs, kernelDescCount);
            desc.pipelineType = pipelineType;
            desc.kernels = kernelDescs.getBuffer();
            desc.kernelCount = kernelDescCount;
        }
        void reset()
        {
            {
                desc.pipelineType = PipelineType::Unknown;
                desc.kernels = nullptr;
                desc.kernelCount = 0;
            }

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

        Slang::Index findKernelDescIndex(gfx::StageType stage) const
        {
            for (Slang::Index i = 0; i < kernelDescs.getCount(); ++i)
            {
                if (kernelDescs[i].stage == stage)
                {
                    return i;
                }
            }
            return -1;
        }

        List<ShaderProgram::KernelDesc> kernelDescs;
        ShaderProgram::Desc desc;
        SlangCompileRequest* request = nullptr;
        SlangSession* session = nullptr;

    };

    struct OutputAndLayout
    {
        Output output;
        ShaderInputLayout layout;
        Slang::String sourcePath;
    };

    static SlangResult compileWithLayout(SlangSession* session, const Options& options, const ShaderCompilerUtil::Input& input, OutputAndLayout& output);

    static SlangResult readSource(const Slang::String& inSourcePath, List<char>& outSourceText);

    static SlangResult compileProgram(SlangSession* session, const Input& input, const ShaderCompileRequest& request, Output& out);
};


} // renderer_test
