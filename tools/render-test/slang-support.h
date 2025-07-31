// slang-support.h
#pragma once

#include "core/slang-std-writers.h"
#include "options.h"
#include "shader-input-layout.h"
#include "slang.h"

#include <slang-rhi.h>

namespace renderer_test
{

/// Bridge from core debug callback to RHI debug callback
/// This allows core callbacks to receive messages from RHI systems
/// TODO: We should replace rhi::IDebugCallback with Slang::IDebugCallback.
class CoreToRHIDebugBridge : public rhi::IDebugCallback
{
public:
    void setCoreCallback(Slang::IDebugCallback* coreCallback) { m_coreCallback = coreCallback; }

    virtual SLANG_NO_THROW void SLANG_MCALL handleMessage(
        rhi::DebugMessageType type,
        rhi::DebugMessageSource source,
        const char* message) override
    {
        if (m_coreCallback)
        {
            // Convert RHI types to core types
            Slang::DebugMessageType coreType = static_cast<Slang::DebugMessageType>(type);
            Slang::DebugMessageSource coreSource = static_cast<Slang::DebugMessageSource>(source);
            m_coreCallback->handleMessage(coreType, coreSource, message);
        }
    }

private:
    Slang::IDebugCallback* m_coreCallback = nullptr;
};

/// Core debug callback that captures debug messages in a string buffer
class CoreDebugCallback : public Slang::IDebugCallback
{
public:
    virtual SLANG_NO_THROW void SLANG_MCALL handleMessage(
        Slang::DebugMessageType type,
        Slang::DebugMessageSource source,
        const char* message) override
    {
        SLANG_UNUSED(source);

        // Only capture error messages
        if (type == Slang::DebugMessageType::Error)
        {
            m_buf << message;
            if (message[strlen(message) - 1] != '\n')
            {
                m_buf << '\n';
            }
        }
    }

    void clear() { m_buf.clear(); }
    Slang::String getString() { return m_buf.toString(); }

private:
    Slang::StringBuilder m_buf;
};

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

    struct TypeConformance
    {
    public:
        Slang::String derivedTypeName;
        Slang::String baseTypeName;
        Slang::Int idOverride;
    };

    SourceInfo source;
    Slang::List<EntryPoint> entryPoints;

    Slang::List<Slang::String> globalSpecializationArgs;
    Slang::List<Slang::String> entryPointSpecializationArgs;
    Slang::List<TypeConformance> typeConformances;
};


struct ShaderCompilerUtil
{
    struct Input
    {
        SlangCompileTarget target;
        SlangSourceLanguage sourceLanguage;
        SlangPassThrough passThrough;
        Slang::String profile;
    };

    struct Output
    {
        void set(slang::IComponentType* slangProgram);
        void reset();
        ~Output() { reset(); }

        ComPtr<slang::IComponentType> slangProgram;
        ShaderProgramDesc desc = {};

        ComPtr<slang::ISession> m_session = nullptr;

        slang::IGlobalSession* globalSession = nullptr;
    };

    struct OutputAndLayout
    {
        Output output;
        ShaderInputLayout layout;
        Slang::String sourcePath;
    };

    // Wrapper for compileProgram
    static SlangResult compileWithLayout(
        slang::IGlobalSession* globalSession,
        const Options& options,
        const ShaderCompilerUtil::Input& input,
        OutputAndLayout& output);
};


} // namespace renderer_test
