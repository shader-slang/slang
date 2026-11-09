// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include "hgvm-test-tool.h"

#include "compiler-core/slang-diagnostic-sink.h"
#include "core/slang-blob.h"
#include "core/slang-io.h"
#include "hgbc-compiler.h"
#include "hgvm-native-host.h"
#include "hgvm-trace.h"
#include "slang/slang-hgbc-disassembler.h"
#include "slangi-options.h"

namespace TestServer
{
using namespace Slang;
using namespace Slang::HGVM;

namespace
{

struct EnvironmentEntry
{
    RenderApiType api = RenderApiType::Unknown;
    SlangDiagnosticColor diagnosticColor = SLANG_DIAGNOSTIC_COLOR_AUTO;
    SlangOptimizationLevel shaderOptimizationLevel = SLANG_OPTIMIZATION_LEVEL_DEFAULT;
    bool reportCheckpointIntermediates = false;
    RefPtr<SlangITool::NativeHGVMEnvironment> environment;
};

class WriterOutputSink : public IHGVMOutputSink
{
public:
    explicit WriterOutputSink(ISlangWriter* writer)
        : m_writer(writer)
    {
    }

    void writeLine(std::string_view text) override
    {
        WriterHelper(m_writer).print("%.*s\n", int(text.size()), text.data());
    }

private:
    ComPtr<ISlangWriter> m_writer;
};

String getSourceDirectory(UnownedStringSlice fileName)
{
    String directory = Path::getParentDirectory(fileName);
    return directory.getLength() ? directory : ".";
}

} // namespace

struct HGVMTestTool::Impl
{
    RefPtr<SlangITool::HGBCCompiler> compiler;
    List<EnvironmentEntry> environments;

    SlangResult getEnvironment(
        RenderApiType api,
        SlangDiagnosticColor diagnosticColor,
        SlangOptimizationLevel shaderOptimizationLevel,
        bool reportCheckpointIntermediates,
        RefPtr<SlangITool::NativeHGVMEnvironment>& outEnvironment,
        String& outError)
    {
        for (const EnvironmentEntry& entry : environments)
        {
            if (entry.api == api && entry.diagnosticColor == diagnosticColor &&
                entry.shaderOptimizationLevel == shaderOptimizationLevel &&
                entry.reportCheckpointIntermediates == reportCheckpointIntermediates)
            {
                outEnvironment = entry.environment;
                return SLANG_OK;
            }
        }

        EnvironmentEntry entry;
        entry.api = api;
        entry.diagnosticColor = diagnosticColor;
        entry.shaderOptimizationLevel = shaderOptimizationLevel;
        entry.reportCheckpointIntermediates = reportCheckpointIntermediates;
        SLANG_RETURN_ON_FAIL(SlangITool::NativeHGVMEnvironment::create(
            api,
            diagnosticColor,
            shaderOptimizationLevel,
            entry.environment,
            outError,
            reportCheckpointIntermediates));
        outEnvironment = entry.environment;
        environments.add(std::move(entry));
        return SLANG_OK;
    }
};

HGVMTestTool::HGVMTestTool()
    : m_impl(new Impl)
{
}

HGVMTestTool::~HGVMTestTool() = default;

SlangResult HGVMTestTool::execute(
    StdWriters* stdWriters,
    slang::IGlobalSession* globalSession,
    const SlangITool::Options& options)
{
    if (!stdWriters || !globalSession)
        return SLANG_E_INVALID_ARG;

    WriterHelper errorWriter(stdWriters->getWriter(SLANG_WRITER_CHANNEL_STD_ERROR));
    if (options.debug)
    {
        errorWriter.print("Interactive debugging requires standalone slangi.\n");
        return SLANG_E_NOT_AVAILABLE;
    }
    if (options.events.mode == SlangITool::EventOutputOptions::Mode::Stats)
    {
        errorWriter.print("Statistics require standalone slangi.\n");
        return SLANG_E_NOT_AVAILABLE;
    }
    ComPtr<slang::IBlob> code;
    if (Path::getPathExt(options.fileName.getUnownedSlice()) == toSlice("hgbc"))
    {
        List<uint8_t> bytes;
        if (SLANG_FAILED(File::readAllBytes(options.fileName, bytes)))
        {
            errorWriter.print("Unable to read HGBC program '%s'.\n", options.fileName.getBuffer());
            return SLANG_FAIL;
        }
        code = ListBlob::moveCreate(bytes);
    }
    else
    {
        if (!m_impl->compiler)
            m_impl->compiler = new SlangITool::HGBCCompiler(globalSession);
        List<const char*> searchPaths;
        for (const String& searchPath : options.additionalSearchPaths)
            searchPaths.add(searchPath.getBuffer());
        String diagnostics;
        SlangResult compileResult = m_impl->compiler->compile(
            options.fileName.getBuffer(),
            options.entryPointName.getBuffer(),
            searchPaths.getBuffer(),
            searchPaths.getCount(),
            options.events.isEnabled() ? SLANG_DEBUG_INFO_LEVEL_MINIMAL
                                       : SLANG_DEBUG_INFO_LEVEL_NONE,
            options.diagnosticColor,
            code,
            diagnostics,
            options.reportCheckpointIntermediates);
        if (diagnostics.getLength())
            errorWriter.print("%s", diagnostics.getBuffer());
        if (SLANG_FAILED(compileResult))
            return compileResult;
    }

    if (!code || !code->getBufferSize())
        return SLANG_FAIL;
    if (options.disassemble)
    {
        DiagnosticSink sink;
        sink.writer = stdWriters->getWriter(SLANG_WRITER_CHANNEL_STD_ERROR);
        SLANG_RETURN_ON_FAIL(
            disassembleHGBC(code, stdWriters->getWriter(SLANG_WRITER_CHANNEL_STD_OUTPUT), &sink));
        return SLANG_OK;
    }

    RefPtr<SlangITool::NativeHGVMEnvironment> environment;
    String error;
    SlangResult environmentResult = m_impl->getEnvironment(
        options.api,
        options.diagnosticColor,
        options.shaderOptimizationLevel,
        options.reportCheckpointIntermediates,
        environment,
        error);
    if (SLANG_FAILED(environmentResult))
    {
        if (error.getLength())
            errorWriter.print("%s\n", error.getBuffer());
        return environmentResult;
    }
    RefPtr<WriterOutputSink> outputSink =
        new WriterOutputSink(stdWriters->getWriter(SLANG_WRITER_CHANNEL_STD_OUTPUT));
    RefPtr<SlangITool::NativeHGVMRuntime> runtime;
    RefPtr<SlangITool::HGVMTraceSink> traceSink;
    if (options.events.isEnabled())
        traceSink = new SlangITool::HGVMTraceSink(
            stdWriters->getWriter(SLANG_WRITER_CHANNEL_STD_OUTPUT),
            options.events);
    String workingDirectory = getSourceDirectory(options.fileName.getUnownedSlice());
    SlangResult result = SlangITool::NativeHGVMRuntime::create(
        environment,
        code,
        options.fileName.getBuffer(),
        workingDirectory.getBuffer(),
        options.gpuExecution,
        outputSink,
        runtime,
        error,
        traceSink);
    if (SLANG_SUCCEEDED(result))
    {
        result = runtime->getVirtualMachine()->call(
            runtime->getProgram(),
            options.entryPointName.getBuffer(),
            error);
        if (SLANG_FAILED(result) || options.reportCheckpointIntermediates)
            runtime->prependDebugMessages(error);
    }

    String waitError;
    SlangResult waitResult = environment->waitForIdle(waitError);
    runtime = nullptr;
    if (error.getLength())
        errorWriter.print("%s\n", error.getBuffer());
    if (SLANG_FAILED(result))
    {
        return result;
    }
    if (SLANG_FAILED(waitResult))
    {
        errorWriter.print("%s\n", waitError.getBuffer());
        return waitResult;
    }
    return SLANG_OK;
}

} // namespace TestServer
