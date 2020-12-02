// slang-support.h
#pragma once

#include "render.h"

#include <slang.h>

#include "shader-input-layout.h"
#include "options.h"

namespace renderer_test {

gfx::StageType translateStage(SlangStage slangStage);

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
            if (m_requestForKernels && session)
            {
                spDestroyCompileRequest(m_requestForKernels);
            }
            if (m_extraRequestForReflection && session)
            {
                spDestroyCompileRequest(m_extraRequestForReflection);
            }
            session = nullptr;
            m_requestForKernels = nullptr;
            m_extraRequestForReflection = nullptr;
        }
        ~Output()
        {
            reset();
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

            /// Compile request that owns the lifetime of compiled kernel code.
        SlangCompileRequest* m_requestForKernels = nullptr;

            /// Compile request that owns the lifetime of reflection information.
        SlangCompileRequest* m_extraRequestForReflection = nullptr;

        SlangCompileRequest* getRequestForKernels() const { return m_requestForKernels; }
        SlangCompileRequest* getRequestForReflection() const { return m_extraRequestForReflection ? m_extraRequestForReflection : m_requestForKernels; }

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

    static SlangResult _compileProgramImpl(SlangSession* session, const Options& options, const Input& input, const ShaderCompileRequest& request, Output& out);
    static SlangResult compileProgram(SlangSession* session, const Options& options, const Input& input, const ShaderCompileRequest& request, Output& out);
};


} // renderer_test
