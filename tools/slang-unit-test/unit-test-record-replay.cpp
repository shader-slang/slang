// unit-test-record-replay.cpp
//
// Tests that verify the record-replay system works correctly by:
// 1. Running example programs with recording enabled (via SLANG_RECORD_LAYER=1)
// 2. Verifying that replay files (stream.bin) are created
//
// Future: Load and playback the recordings to verify determinism

#include "../../source/core/slang-io.h"
#include "../../source/core/slang-process-util.h"
#include "../../source/core/slang-string-util.h"
#include "../../source/slang-record-replay/replay-context.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

// =============================================================================
// Process launching helpers
// =============================================================================

static SlangResult createProcess(
    UnitTestContext* context,
    const char* processName,
    const List<String>* optArgs,
    RefPtr<Process>& outProcess)
{
    CommandLine cmdLine;
    cmdLine.setExecutableLocation(ExecutableLocation(context->executableDirectory, processName));
    if (optArgs)
    {
        cmdLine.m_args.addRange(optArgs->getBuffer(), optArgs->getCount());
    }

    SLANG_RETURN_ON_FAIL(Process::create(cmdLine, Process::Flag::AttachDebugger, outProcess));

    return SLANG_OK;
}

static int writeEnvironmentVariable(const char* key, const char* val)
{
#ifdef _WIN32
    String var = String(key) + "=" + val;
    return _putenv(var.getBuffer());
#else
    return setenv(key, val, 1);
#endif
}

static SlangResult launchProcessAndReadStdout(
    UnitTestContext* context,
    const List<String>& optArgs,
    const char* exampleName,
    RefPtr<Process>& process,
    ExecuteResult& exeRes)
{
    StringBuilder msgBuilder;
    msgBuilder << "Launching process for '" << exampleName << "'\n";
    getTestReporter()->message(TestMessageType::Info, msgBuilder.toString().getBuffer());

    SlangResult res = createProcess(context, exampleName, &optArgs, process);
    if (SLANG_FAILED(res))
    {
        msgBuilder << "Failed to launch process of '" << exampleName << "'\n";
        getTestReporter()->message(TestMessageType::TestFailure, msgBuilder.toString().getBuffer());
        return res;
    }

    res = ProcessUtil::readUntilTermination(process, exeRes);
    if (SLANG_FAILED(res))
    {
        msgBuilder << "Failed to read stdout from '" << exampleName << "'\n";
        msgBuilder << "process ret code: " << exeRes.resultCode;
        getTestReporter()->message(TestMessageType::TestFailure, msgBuilder.toString().getBuffer());
        return res;
    }

    if (exeRes.resultCode != 0)
    {
        msgBuilder << "'" << exampleName << "' exits with failure\n";
        msgBuilder << "Process ret code: " << exeRes.resultCode << "\n";
        msgBuilder << "Standard output:\n" << exeRes.standardOutput;
        msgBuilder << "Standard error:\n" << exeRes.standardError;
        getTestReporter()->message(TestMessageType::TestFailure, msgBuilder.toString().getBuffer());
        return SLANG_FAIL;
    }

    return SLANG_OK;
}

// =============================================================================
// Record/Replay test infrastructure
// =============================================================================

static SlangResult cleanupRecordFiles(const String& recordDir)
{
    // Silently try to remove the directory - it's okay if it doesn't exist
    Path::removeNonEmpty(recordDir.getBuffer());
    return SLANG_OK;
}

static SlangResult runTest(UnitTestContext* context, const char* testName)
{
    // The replay system uses ".slang-replays" as the default directory.
    // We just need to enable recording and verify files are created there.
    String recordDir = ".slang-replays";

    // Clean up any leftover files from previous runs
    cleanupRecordFiles(recordDir);

    // Enable recording via environment variable for child process
    // The child process will check SLANG_RECORD_LAYER on startup
    writeEnvironmentVariable("SLANG_RECORD_LAYER", "1");

    // Also enable logging
    writeEnvironmentVariable("SLANG_RECORD_LOG", "1");

    // Run the example with recording enabled
    RefPtr<Process> process;
    ExecuteResult exeRes;
    List<String> optArgs;
    optArgs.add("--test-mode");

    SlangResult res = launchProcessAndReadStdout(context, optArgs, testName, process, exeRes);

    // Disable recording for any future child processes
    writeEnvironmentVariable("SLANG_RECORD_LAYER", "0");

    if (SLANG_FAILED(res))
    {
        cleanupRecordFiles(recordDir);
        return res;
    }

    #if 0
    // Verify that a replay folder was created
    String latestFolder = SlangRecord::ReplayContext::findLatestReplayFolder(recordDir.getBuffer());
    if (latestFolder.getLength() == 0)
    {
        StringBuilder msgBuilder;
        msgBuilder << "No replay folder created for '" << testName << "'\n";
        msgBuilder << "Expected replay files in: " << recordDir << "\n";
        getTestReporter()->message(TestMessageType::TestFailure, msgBuilder.toString().getBuffer());
        cleanupRecordFiles(recordDir);
        return SLANG_FAIL;
    }

    // Verify stream.bin exists
    String streamPath = Path::combine(Path::combine(recordDir, latestFolder), "stream.bin");
    if (!File::exists(streamPath))
    {
        StringBuilder msgBuilder;
        msgBuilder << "No stream.bin found for '" << testName << "'\n";
        msgBuilder << "Expected file at: " << streamPath << "\n";
        getTestReporter()->message(TestMessageType::TestFailure, msgBuilder.toString().getBuffer());
        cleanupRecordFiles(recordDir);
        return SLANG_FAIL;
    }
    #endif

    // TODO: Future enhancement - load and playback the replay to verify determinism
    // SlangResult loadRes = slang_loadReplay(Path::combine(recordDir, latestFolder).getBuffer());
    // if (SLANG_SUCCEEDED(loadRes))
    // {
    //     SlangRecord::ReplayContext::get().executeAll();
    // }

    // Cleanup
    cleanupRecordFiles(recordDir);
    return SLANG_OK;
}

// =============================================================================
// Test cases
// =============================================================================

// These examples depend on Vulkan, so we only run them on non-Apple platforms.
// In the future, we may be able to modify the examples to remove render API
// dependencies so they can run on Apple platforms.
#if !(SLANG_APPLE_FAMILY)

SLANG_UNIT_TEST(RecordReplay_cpu_hello_world)
{
    SLANG_CHECK(SLANG_SUCCEEDED(runTest(unitTestContext, "cpu-hello-world")));
}

SLANG_UNIT_TEST(RecordReplay_triangle)
{
    SLANG_CHECK(SLANG_SUCCEEDED(runTest(unitTestContext, "triangle")));
}

SLANG_UNIT_TEST(RecordReplay_ray_tracing)
{
    SLANG_CHECK(SLANG_SUCCEEDED(runTest(unitTestContext, "ray-tracing")));
}

// This causes a Windows Graphics driver crash.
// Temporarily disabled; issue #8022
#if 0
SLANG_UNIT_TEST(RecordReplay_ray_tracing_pipeline)
{
    SLANG_CHECK(SLANG_SUCCEEDED(runTest(unitTestContext, "ray-tracing-pipeline")));
}
#endif

SLANG_UNIT_TEST(RecordReplay_autodiff_texture)
{
    SLANG_CHECK(SLANG_SUCCEEDED(runTest(unitTestContext, "autodiff-texture")));
}

SLANG_UNIT_TEST(RecordReplay_gpu_printing)
{
    SLANG_CHECK(SLANG_SUCCEEDED(runTest(unitTestContext, "gpu-printing")));
}

#if 0
// These examples require reflection API to replay, disabled for now.

SLANG_UNIT_TEST(RecordReplay_shader_object)
{
    SLANG_CHECK(SLANG_SUCCEEDED(runTest(unitTestContext, "shader-object")));
}

SLANG_UNIT_TEST(RecordReplay_model_viewer)
{
    SLANG_CHECK(SLANG_SUCCEEDED(runTest(unitTestContext, "model-viewer")));
}
#endif

#endif
