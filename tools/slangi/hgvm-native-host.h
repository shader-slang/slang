// hgvm-native-host.h
#pragma once

#include "core/slang-basic.h"
#include "core/slang-render-api-util.h"
#include "slang-hgvm/slang-hgvm-debug-session.h"
#include "slang-hgvm/slang-hgvm.h"

#include <memory>
#include <slang.h>

namespace SlangITool
{

/// Owns the native platform, RHI device, and host services shared by sequential HGVM programs.
class NativeHGVMEnvironment : public Slang::RefObject
{
public:
    static SlangResult create(
        Slang::RenderApiType api,
        SlangDiagnosticColor diagnosticColor,
        SlangOptimizationLevel shaderOptimizationLevel,
        Slang::RefPtr<NativeHGVMEnvironment>& outEnvironment,
        Slang::String& outError,
        bool reportCheckpointIntermediates = false);

    ~NativeHGVMEnvironment();

    /// Waits for work submitted by the current test before its program-owned state is released.
    SlangResult waitForIdle(Slang::String& outError);

private:
    friend class NativeHGVMRuntime;

    struct Impl;
    std::unique_ptr<Impl> m_impl;

    NativeHGVMEnvironment();
};

/// Owns one HGBC program and interpreter while borrowing a native HGVM environment.
class NativeHGVMRuntime : public Slang::RefObject
{
public:
    static SlangResult create(
        slang::IBlob* code,
        const char* programName,
        const char* workingDirectory,
        Slang::HGVM::GPUExecutionMode gpuExecution,
        Slang::HGVM::IHGVMOutputSink* outputSink,
        SlangDiagnosticColor diagnosticColor,
        Slang::RefPtr<NativeHGVMRuntime>& outRuntime,
        Slang::String& outError,
        Slang::HGVM::IEventSink* eventSink = nullptr,
        bool enableDebugging = false);

    static SlangResult create(
        slang::IBlob* code,
        const char* programName,
        const char* workingDirectory,
        Slang::HGVM::GPUExecutionMode gpuExecution,
        Slang::HGVM::IHGVMOutputSink* outputSink,
        Slang::RenderApiType api,
        SlangDiagnosticColor diagnosticColor,
        SlangOptimizationLevel shaderOptimizationLevel,
        Slang::RefPtr<NativeHGVMRuntime>& outRuntime,
        Slang::String& outError,
        Slang::HGVM::IEventSink* eventSink = nullptr,
        bool enableDebugging = false,
        bool reportCheckpointIntermediates = false);

    static SlangResult create(
        NativeHGVMEnvironment* environment,
        slang::IBlob* code,
        const char* programName,
        const char* workingDirectory,
        Slang::HGVM::GPUExecutionMode gpuExecution,
        Slang::HGVM::IHGVMOutputSink* outputSink,
        Slang::RefPtr<NativeHGVMRuntime>& outRuntime,
        Slang::String& outError,
        Slang::HGVM::IEventSink* eventSink = nullptr,
        bool enableDebugging = false);

    ~NativeHGVMRuntime();

    Slang::HGVM::Program* getProgram() const;
    Slang::HGVirtualMachine* getVirtualMachine() const;
    Slang::HGVM::DebugSession* getDebugSession() const;
    void prependDebugMessages(Slang::String& ioError) const;

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;

    NativeHGVMRuntime();
};

using DebugSessionRunner =
    SlangResult (*)(Slang::HGVM::DebugSession*, Slang::HGVM::Program*, const char*, Slang::String&);

/// Executes a compiled HGBC program using the native platform window and slang-rhi services.
SlangResult executeNativeHGBCProgram(
    slang::IBlob* code,
    const char* programName,
    const char* workingDirectory,
    const char* entryPointName,
    Slang::HGVM::GPUExecutionMode gpuExecution,
    Slang::RenderApiType api,
    SlangDiagnosticColor diagnosticColor,
    SlangOptimizationLevel shaderOptimizationLevel,
    Slang::String& outError,
    Slang::HGVM::IEventSink* eventSink = nullptr,
    DebugSessionRunner runDebugger = nullptr,
    Slang::HGVM::IHGVMOutputSink* outputSink = nullptr,
    bool reportCheckpointIntermediates = false);

} // namespace SlangITool
