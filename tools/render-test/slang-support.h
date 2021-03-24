// slang-support.h
#pragma once

#include "slang-gfx.h"

#include <slang.h>

#include "shader-input-layout.h"
#include "options.h"

namespace renderer_test {

gfx::StageType translateStage(SlangStage slangStage);

struct ShaderCompileRequest
{
    struct SourceInfo
    {
        char const* path;

        // The data may either be source text (in which
        // case it can be assumed to be nul-terminated with
        // `dataEnd` pointing at the terminator), or
        // raw binary data (in which case `dataEnd` points
        // at the end of the buffer).
        char const* dataBegin;
        char const* dataEnd;
    };

    struct EntryPoint
    {
        char const* name = nullptr;
        SlangStage slangStage;
    };

    SourceInfo source;
    Slang::List<EntryPoint> entryPoints;

    Slang::List<Slang::String> globalSpecializationArgs;
    Slang::List<Slang::String> entryPointSpecializationArgs;

    Slang::List<Slang::CommandLine::Arg> compileArgs;
};


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
        void set(
            PipelineType                        pipelineType,
            slang::IComponentType*              slangProgram);
        void reset();
        ~Output()
        {
            reset();
        }

        ComPtr<slang::IComponentType> slangProgram;
        IShaderProgram::Desc desc = {};

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

    static SlangResult readSource(const Slang::String& inSourcePath, Slang::List<char>& outSourceText);

    static SlangResult _compileProgramImpl(SlangSession* session, const Options& options, const Input& input, const ShaderCompileRequest& request, Output& out);
    static SlangResult compileProgram(SlangSession* session, const Options& options, const Input& input, const ShaderCompileRequest& request, Output& out);
};


} // renderer_test
