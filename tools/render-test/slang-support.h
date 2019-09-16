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

    static SlangResult compileWithLayout(SlangSession* session, const Slang::String& sourcePath, const Slang::List<Slang::CommandLine::Arg>& compileArgs, Options::ShaderProgramType shaderType, const ShaderCompilerUtil::Input& input, OutputAndLayout& output);

    static SlangResult readSource(const Slang::String& inSourcePath, List<char>& outSourceText);

    static SlangResult compileProgram(SlangSession* session, const Input& input, const ShaderCompileRequest& request, Output& out);
};


} // renderer_test
