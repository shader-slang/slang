// main.cpp

// This file implements the entry point for `slangi`, an interpreter for the Slang language.

#include "core/slang-basic.h"
#include "core/slang-blob.h"
#include "core/slang-io.h"
#include "core/slang-std-writers.h"
#include "slang-com-ptr.h"
#include "slang.h"
#include "slangi-options.h"

#if SLANGI_ENABLE_HGVM
#include "compiler-core/slang-diagnostic-sink.h"
#include "hgbc-compiler.h"
#include "hgvm-debug-cli.h"
#include "hgvm-debug-dap.h"
#include "hgvm-native-host.h"
#include "hgvm-stats-console.h"
#include "hgvm-trace.h"
#include "slang/slang-hgbc-disassembler.h"
#endif

using namespace Slang;
using namespace slang;

void maybePrintDiagnostic(const ComPtr<slang::IBlob>& diagnosticBlob)
{
    if (diagnosticBlob)
    {
        const char* diagText = (const char*)diagnosticBlob->getBufferPointer();
        fprintf(stderr, "%s\n", diagText);
    }
}

#if SLANGI_ENABLE_HGVM
static SlangResult disassembleHGBCProgram(slang::IBlob* code, StdWriters* stdWriters)
{
    DiagnosticSink sink;
    sink.writer = stdWriters->getWriter(SLANG_WRITER_CHANNEL_STD_ERROR);
    return disassembleHGBC(code, stdWriters->getWriter(SLANG_WRITER_CHANNEL_STD_OUTPUT), &sink);
}
#endif

/// Returns the directory that contains a source file so imports resolve beside it. A bare filename
/// denotes a file in the process's current directory, which `Path::getParentDirectory` represents
/// as an empty string.
static String getSourceDirectory(UnownedStringSlice fileName)
{
    String directory = Path::getParentDirectory(fileName);
    return directory.getLength() ? directory : ".";
}

#if SLANGI_ENABLE_HGVM
/// Keeps event receivers alive through runtime destruction, including final GPU completions.
static SlangResult executeHGBCWithEventOutput(
    slang::IBlob* code,
    const char* programName,
    const char* workingDirectory,
    const char* entryPointName,
    Slang::HGVM::GPUExecutionMode gpuExecution,
    RenderApiType api,
    SlangDiagnosticColor diagnosticColor,
    SlangOptimizationLevel shaderOptimizationLevel,
    bool reportCheckpointIntermediates,
    String& error,
    const SlangITool::EventOutputOptions& events,
    bool debug,
    StdWriters* writers)
{
    using namespace SlangITool;
    RefPtr<Slang::HGVM::IEventSink> sink;
    RefPtr<HGVMStatsConsole> console;
    auto output = writers->getWriter(SLANG_WRITER_CHANNEL_STD_OUTPUT);
    if (events.mode == EventOutputOptions::Mode::Trace)
        sink = new HGVMTraceSink(output, events);
    else if (events.mode == EventOutputOptions::Mode::Stats)
    {
        RefPtr<HGVMStatsSink> stats = new HGVMStatsSink;
        sink = stats;
        console = new HGVMStatsConsole(
            stats,
            output,
            writers->getWriter(SLANG_WRITER_CHANNEL_STD_ERROR),
            events);
    }
    SlangResult result = executeNativeHGBCProgram(
        code,
        programName,
        workingDirectory,
        entryPointName,
        gpuExecution,
        api,
        diagnosticColor,
        shaderOptimizationLevel,
        error,
        sink,
        debug ? runHGVMDebugCLI : nullptr,
        console,
        reportCheckpointIntermediates);
    if (console)
        console->finish();
    return result;
}

/// Loads a precompiled HGBC program and executes it on the native host without invoking the
/// compiler.
SlangResult loadAndRunHGBC(
    UnownedStringSlice fileName,
    const char* entryPointName,
    bool disasm,
    const SlangITool::EventOutputOptions& events,
    bool debug,
    HGVM::GPUExecutionMode gpuExecution,
    RenderApiType api,
    SlangDiagnosticColor diagnosticColor,
    SlangOptimizationLevel shaderOptimizationLevel,
    bool reportCheckpointIntermediates,
    StdWriters* stdWriters)
{
    List<uint8_t> bytes;
    String path = fileName;
    if (SLANG_FAILED(File::readAllBytes(path, bytes)))
    {
        fprintf(stderr, "Unable to read precompiled HGBC program '%s'.\n", path.getBuffer());
        return SLANG_FAIL;
    }

    ComPtr<slang::IBlob> code = ListBlob::moveCreate(bytes);
    if (disasm)
        SLANG_RETURN_ON_FAIL(disassembleHGBCProgram(code, stdWriters));

    String error;
    String sourceDirectory = getSourceDirectory(fileName);
    SlangResult result = executeHGBCWithEventOutput(
        code,
        path.getBuffer(),
        sourceDirectory.getBuffer(),
        entryPointName,
        gpuExecution,
        api,
        diagnosticColor,
        shaderOptimizationLevel,
        reportCheckpointIntermediates,
        error,
        events,
        debug,
        stdWriters);
    if (error.getLength())
        fprintf(stderr, "%s\n", error.getBuffer());
    return result;
}

/// Compiles a graph entry point in an HGVM-only session and executes it on the native host.
SlangResult compileAndRunHGBC(
    UnownedStringSlice fileName,
    const char* entryPointName,
    const List<String>& additionalSearchPaths,
    bool disasm,
    const SlangITool::EventOutputOptions& events,
    bool debug,
    HGVM::GPUExecutionMode gpuExecution,
    RenderApiType api,
    SlangDiagnosticColor diagnosticColor,
    SlangOptimizationLevel shaderOptimizationLevel,
    bool reportCheckpointIntermediates,
    StdWriters* stdWriters)
{
    List<const char*> additionalSearchPathPointers;
    for (const String& searchPath : additionalSearchPaths)
        additionalSearchPathPointers.add(searchPath.getBuffer());

    ComPtr<slang::IBlob> code;
    String diagnostics;
    String sourcePath = fileName;
    SlangResult result = SlangITool::compileHGBCProgram(
        sourcePath.getBuffer(),
        entryPointName,
        additionalSearchPathPointers.getBuffer(),
        additionalSearchPathPointers.getCount(),
        debug                ? SLANG_DEBUG_INFO_LEVEL_STANDARD
        : events.isEnabled() ? SLANG_DEBUG_INFO_LEVEL_MINIMAL
                             : SLANG_DEBUG_INFO_LEVEL_NONE,
        diagnosticColor,
        code,
        diagnostics,
        reportCheckpointIntermediates);
    if (diagnostics.getLength())
        fprintf(stderr, "%s\n", diagnostics.getBuffer());
    if (SLANG_FAILED(result))
        return result;

    if (disasm)
        SLANG_RETURN_ON_FAIL(disassembleHGBCProgram(code, stdWriters));

    String error;
    String sourceDirectory = getSourceDirectory(fileName);
    result = executeHGBCWithEventOutput(
        code,
        sourcePath.getBuffer(),
        sourceDirectory.getBuffer(),
        entryPointName,
        gpuExecution,
        api,
        diagnosticColor,
        shaderOptimizationLevel,
        reportCheckpointIntermediates,
        error,
        events,
        debug,
        stdWriters);
    if (error.getLength())
        fprintf(stderr, "%s\n", error.getBuffer());
    return result;
}
#endif

SlangResult compileAndRunLegacy(
    UnownedStringSlice fileName,
    const char* entryPointName,
    const List<String>& additionalSearchPaths,
    bool disasm,
    int argc,
    const char* const* argv,
    SlangDiagnosticColor diagnosticColor)
{
    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_RETURN_ON_FAIL(slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()));
    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_HOST_VM;
    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;
    slang::CompilerOptionEntry compilerOptions[2] = {};
    compilerOptions[0].name = slang::CompilerOptionName::DiagnosticColor;
    compilerOptions[0].value.kind = slang::CompilerOptionValueKind::Int;
    compilerOptions[0].value.intValue0 = diagnosticColor;
    // Slangi and HGVM are experimental, so programs interpreted by this tool may use Slang's
    // experimental modules without requiring a redundant command-line opt-in.
    compilerOptions[1].name = slang::CompilerOptionName::ExperimentalFeature;
    compilerOptions[1].value.kind = slang::CompilerOptionValueKind::Int;
    compilerOptions[1].value.intValue0 = 1;
    sessionDesc.compilerOptionEntries = compilerOptions;
    sessionDesc.compilerOptionEntryCount = SLANG_COUNT_OF(compilerOptions);
    String pathName = getSourceDirectory(fileName);
    String moduleName = Path::getFileNameWithoutExt(fileName);
    List<const char*> searchPaths;
    searchPaths.add(pathName.getBuffer());
    for (const String& searchPath : additionalSearchPaths)
        searchPaths.add(searchPath.getBuffer());
    sessionDesc.searchPathCount = searchPaths.getCount();
    sessionDesc.searchPaths = searchPaths.getBuffer();
    ComPtr<slang::ISession> session;
    SLANG_RETURN_ON_FAIL(globalSession->createSession(sessionDesc, session.writeRef()));

    ComPtr<slang::IBlob> diagnosticBlob;
    auto module = session->loadModule(moduleName.getBuffer(), diagnosticBlob.writeRef());
    if (!module)
    {
        maybePrintDiagnostic(diagnosticBlob);
        return SLANG_FAIL;
    }
    ComPtr<slang::IEntryPoint> entryPoint;
    if (SLANG_FAILED(module->findAndCheckEntryPoint(
            entryPointName,
            SLANG_STAGE_DISPATCH,
            entryPoint.writeRef(),
            diagnosticBlob.writeRef())))
    {
        maybePrintDiagnostic(diagnosticBlob);
        return SLANG_FAIL;
    }
    if (slang_isHostEntryPoint(entryPoint))
    {
        fprintf(stderr, "[host] entry points require HGVM; omit -legacy.\n");
        return SLANG_FAIL;
    }

    ComPtr<slang::IComponentType> compositeComponent;
    slang::IComponentType* components[] = {module, entryPoint.get()};
    if (SLANG_FAILED(session->createCompositeComponentType(
            components,
            2,
            compositeComponent.writeRef(),
            diagnosticBlob.writeRef())))
    {
        maybePrintDiagnostic(diagnosticBlob);
        return SLANG_FAIL;
    }

    ComPtr<slang::IComponentType> linkedProgram;
    if (SLANG_FAILED(compositeComponent->link(linkedProgram.writeRef(), diagnosticBlob.writeRef())))
    {
        maybePrintDiagnostic(diagnosticBlob);
        return SLANG_FAIL;
    }
    ComPtr<slang::IBlob> code;

    if (SLANG_FAILED(linkedProgram->getTargetCode(0, code.writeRef(), diagnosticBlob.writeRef())))
    {
        maybePrintDiagnostic(diagnosticBlob);
        return SLANG_FAIL;
    }

    if (code->getBufferSize() == 0)
    {
        return SLANG_FAIL;
    }

    if (disasm)
    {
        ComPtr<slang::IBlob> disasmBlob;
        if (SLANG_FAILED(slang_disassembleByteCode(code, disasmBlob.writeRef())))
        {
            maybePrintDiagnostic(diagnosticBlob);
            return SLANG_FAIL;
        }
        const char* disasmText = (const char*)disasmBlob->getBufferPointer();
        printf("%s\n", disasmText);
    }

    // Create a byte code runner and interpret the code.
    ComPtr<slang::IByteCodeRunner> runner;
    slang::ByteCodeRunnerDesc runnerDesc = {};
    SLANG_RETURN_ON_FAIL(slang_createByteCodeRunner(&runnerDesc, runner.writeRef()));
    if (SLANG_FAILED(runner->loadModule(code)))
    {
        runner->getErrorString(diagnosticBlob.writeRef());
        maybePrintDiagnostic(diagnosticBlob);
    }
    auto funcIndex = runner->findFunctionByName(entryPointName);
    if (funcIndex < 0)
    {
        printf("Function '%s' not found in byte code.\n", entryPointName);
        return SLANG_FAIL;
    }

    if (SLANG_FAILED(runner->selectFunctionByIndex((uint32_t)funcIndex)))
    {
        runner->getErrorString(diagnosticBlob.writeRef());
        maybePrintDiagnostic(diagnosticBlob);
        return SLANG_FAIL;
    }

    struct Arguments
    {
        uint32_t argc;
        const char* const* argv;
    };
    Arguments args;
    args.argc = argc;
    args.argv = argv;
    void* arguments = nullptr;
    size_t argSize = 0;
    slang::ByteCodeFuncInfo funcInfo;
    if (SLANG_FAILED(runner->getFunctionInfo((uint32_t)funcIndex, &funcInfo)))
    {
        runner->getErrorString(diagnosticBlob.writeRef());
        maybePrintDiagnostic(diagnosticBlob);
        return SLANG_FAIL;
    }
    if (funcInfo.parameterCount == 2)
    {
        arguments = &args;
        argSize = sizeof(Arguments);
    }
    if (SLANG_FAILED(runner->execute(arguments, argSize)))
    {
        runner->getErrorString(diagnosticBlob.writeRef());
        maybePrintDiagnostic(diagnosticBlob);
        return SLANG_FAIL;
    }
    size_t returnValueSize = 0;
    void* returnVal = runner->getReturnValue(&returnValueSize);
    SlangResult result = SLANG_OK;
    memcpy(&result, returnVal, returnValueSize);
    return result;
}

int main(int argc, const char* const* argv)
{
    // DAP gets its launch arguments over the protocol and must never print CLI usage to stdout.
    if (argc == 2 && UnownedStringSlice(argv[1]) == "-dap")
    {
#if SLANGI_ENABLE_HGVM
        SlangResult result = SlangITool::runHGVMDebugDAP();
        slang::shutdown();
        return SLANG_FAILED(result) ? 1 : 0;
#else
        fprintf(stderr, "slangi was built without HGVM debugger support.\n");
        return 1;
#endif
    }
    RefPtr<StdWriters> stdWriters = StdWriters::createDefault();
    SlangITool::Options options;
    bool shouldExit = false;
    if (SLANG_FAILED(SlangITool::parseOptions(stdWriters, argc, argv, options, shouldExit)))
        return -1;
    if (shouldExit)
        return 0;

    SlangResult result;
    if (options.useLegacyInterpreter)
    {
        result = compileAndRunLegacy(
            options.fileName.getUnownedSlice(),
            options.entryPointName.getBuffer(),
            options.additionalSearchPaths,
            options.disassemble,
            argc - options.innerArgIndex,
            argv + options.innerArgIndex,
            options.diagnosticColor);
    }
    else
    {
#if SLANGI_ENABLE_HGVM
        if (Path::getPathExt(options.fileName.getUnownedSlice()) == toSlice("hgbc"))
        {
            result = loadAndRunHGBC(
                options.fileName.getUnownedSlice(),
                options.entryPointName.getBuffer(),
                options.disassemble,
                options.events,
                options.debug,
                options.gpuExecution,
                options.api,
                options.diagnosticColor,
                options.shaderOptimizationLevel,
                options.reportCheckpointIntermediates,
                stdWriters);
        }
        else
        {
            result = compileAndRunHGBC(
                options.fileName.getUnownedSlice(),
                options.entryPointName.getBuffer(),
                options.additionalSearchPaths,
                options.disassemble,
                options.events,
                options.debug,
                options.gpuExecution,
                options.api,
                options.diagnosticColor,
                options.shaderOptimizationLevel,
                options.reportCheckpointIntermediates,
                stdWriters);
        }
#else
        fprintf(stderr, "slangi was built without slang-rhi and cannot execute HGVM programs.\n");
        result = SLANG_FAIL;
#endif
    }
    slang::shutdown();
    return result;
}
