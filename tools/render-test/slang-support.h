// slang-support.h
#pragma once

#include "core/slang-std-writers.h"
#include "options.h"
#include "shader-input-layout.h"
#include "slang.h"

#include <mutex>
#include <slang-rhi.h>

namespace renderer_test
{

/// Bridge from core debug callback to RHI debug callback.
///
/// RHI backends may invoke debug callbacks from backend or driver threads, so
/// binding changes and forwarded messages are serialized.
/// TODO: We should replace rhi::IDebugCallback with Slang::IDebugCallback.
class CoreToRHIDebugBridge : public rhi::IDebugCallback
{
public:
    void setCoreCallback(Slang::IDebugCallback* coreCallback)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_coreCallback = coreCallback;
    }

    virtual SLANG_NO_THROW void SLANG_MCALL handleMessage(
        rhi::DebugMessageType type,
        rhi::DebugMessageSource source,
        const char* message) override
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        auto coreCallback = m_coreCallback;
        if (coreCallback)
        {
            // Convert RHI types to core types
            Slang::DebugMessageType coreType = static_cast<Slang::DebugMessageType>(type);
            Slang::DebugMessageSource coreSource = static_cast<Slang::DebugMessageSource>(source);
            coreCallback->handleMessage(coreType, coreSource, message);
        }
    }

private:
    std::mutex m_mutex;
    Slang::IDebugCallback* m_coreCallback = nullptr;
};

/// Binds an RHI debug bridge to a core callback for one active test invocation.
///
/// The bridge may be retained by RHI device state after this scope exits, but the
/// per-test core callback must not be retained. Messages that arrive while no
/// scoped callback is active are intentionally dropped by the bridge instead of
/// being written to dead callback storage.
class ScopedCoreDebugCallback
{
public:
    ScopedCoreDebugCallback(CoreToRHIDebugBridge& bridge, Slang::IDebugCallback* coreCallback)
        : m_bridge(bridge)
    {
        m_bridge.setCoreCallback(coreCallback);
    }

    ~ScopedCoreDebugCallback() { m_bridge.setCoreCallback(nullptr); }

    ScopedCoreDebugCallback(const ScopedCoreDebugCallback&) = delete;
    ScopedCoreDebugCallback& operator=(const ScopedCoreDebugCallback&) = delete;

private:
    CoreToRHIDebugBridge& m_bridge;
};

/// Core debug callback that captures debug messages in a string buffer.
///
/// Message capture is thread-safe so backend debug callbacks can report while
/// the test harness reads the collected messages.
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
            std::lock_guard<std::mutex> lock(m_mutex);
            m_buf << message;
            if (message[strlen(message) - 1] != '\n')
            {
                m_buf << '\n';
            }
        }
    }

    void clear()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_buf.clear();
    }

    Slang::String getString()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_buf.toString();
    }

private:
    std::mutex m_mutex;
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
