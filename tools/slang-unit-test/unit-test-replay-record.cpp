// unit-test-record-replay.cpp
//
// Tests that verify the record-replay system works correctly by:
// 1. Running example programs with recording enabled (via SLANG_RECORD_LAYER=1)
// 2. Verifying that replay files (stream.bin) are created
// 3. Decoding the stream.bin and computing a hash for verification
//
// Future: Load and playback the recordings to verify determinism

#include "../../source/core/slang-io.h"
#include "../../source/core/slang-process-util.h"
#include "../../source/core/slang-stable-hash.h"
#include "../../source/core/slang-string-util.h"
#include "../../source/slang-record-replay/replay-context.h"
#include "../../source/slang-record-replay/replay-stream-decoder.h"
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

static String getRecordPathForTest(const char* testName)
{
    // Use a predictable path: .slang-replays/<testName>
    return Path::combine(".slang-replays", testName);
}

static SlangResult executeReplay(const char* testName, const String& recordPath)
{
    StringBuilder msgBuilder;
    msgBuilder << "Executing replay for '" << testName << "'...\n";
    getTestReporter()->message(TestMessageType::Info, msgBuilder.toString().getBuffer());

    // Get the replay context and reset it to clean state
    auto& ctx = SlangRecord::ReplayContext::get();
    ctx.reset();
    //ctx.setTtyLogging(true);

    // Load the replay from the recorded folder
    SlangResult res = ctx.loadReplay(recordPath.getBuffer());
    if (SLANG_FAILED(res))
    {
        msgBuilder.clear();
        msgBuilder << "Failed to load replay for '" << testName << "' from: " << recordPath << "\n";
        getTestReporter()->message(TestMessageType::TestFailure, msgBuilder.toString().getBuffer());
        ctx.reset();
        return res;
    }

    // Execute all recorded calls
    try
    {
        ctx.executeAll();
        
        msgBuilder.clear();
        msgBuilder << "Replay completed successfully for '" << testName << "'\n";
        getTestReporter()->message(TestMessageType::Info, msgBuilder.toString().getBuffer());
    }
    catch (const Slang::Exception& e)
    {
        msgBuilder.clear();
        msgBuilder << "Replay failed for '" << testName << "': " << e.Message << "\n";
        getTestReporter()->message(TestMessageType::TestFailure, msgBuilder.toString().getBuffer());
        ctx.reset();
        return SLANG_FAIL;
    }
    catch (const std::exception& e)
    {
        msgBuilder.clear();
        msgBuilder << "Replay failed for '" << testName << "': " << e.what() << "\n";
        getTestReporter()->message(TestMessageType::TestFailure, msgBuilder.toString().getBuffer());
        ctx.reset();
        return SLANG_FAIL;
    }

    // Reset context after replay
    ctx.reset();
    return SLANG_OK;
}

static SlangResult runTest(UnitTestContext* context, const char* testName, uint64_t expectedHash = 0)
{
    // If the context is already active, it means we're running the testing framework as a replay,
    // so specific replay based test cases have to be ignored.
    if (SlangRecord::ReplayContext::get().isActive())
    {
        SLANG_IGNORE_TEST;
        return SLANG_OK;
    }

    // Use a predictable path for each test so we can easily find and verify recordings
    String recordPath = getRecordPathForTest(testName);

    // Clean up any leftover files from previous runs
    cleanupRecordFiles(recordPath);

    // Enable recording via environment variable for child process
    writeEnvironmentVariable("SLANG_RECORD_LAYER", "1");
    
    // Set the explicit recording path
    writeEnvironmentVariable("SLANG_RECORD_PATH", recordPath.getBuffer());

    // Disable logging during tests to reduce noise
    writeEnvironmentVariable("SLANG_RECORD_LOG", "0");

    // Run the example with recording enabled
    RefPtr<Process> process;
    ExecuteResult exeRes;
    List<String> optArgs;
    optArgs.add("--test-mode");

    SlangResult res = launchProcessAndReadStdout(context, optArgs, testName, process, exeRes);

    // Disable recording for any future child processes
    writeEnvironmentVariable("SLANG_RECORD_LAYER", "0");
    writeEnvironmentVariable("SLANG_RECORD_PATH", "");

    if (SLANG_FAILED(res))
    {
        cleanupRecordFiles(recordPath);
        return res;
    }

    // Verify stream.bin exists at the expected path
    String streamPath = Path::combine(recordPath, "stream.bin");
    if (!File::exists(streamPath))
    {
        StringBuilder msgBuilder;
        msgBuilder << "No stream.bin found for '" << testName << "'\n";
        msgBuilder << "Expected file at: " << streamPath << "\n";
        getTestReporter()->message(TestMessageType::TestFailure, msgBuilder.toString().getBuffer());
        cleanupRecordFiles(recordPath);
        return SLANG_FAIL;
    }

    // Decode the stream.bin and compute a hash
    try
    {
        String decoded = SlangRecord::ReplayStreamDecoder::decodeFile(streamPath.getBuffer());
        
        // Compute hash of the decoded content
        StableHashCode64 hash = getStableHashCode64(
            decoded.getBuffer(), 
            decoded.getLength());
        
        // Log the decoded content and hash
        StringBuilder msgBuilder;
        msgBuilder << "Decoded stream.bin for '" << testName << "' (" 
                   << decoded.getLength() << " bytes):\n";
        msgBuilder << "Recording path: " << recordPath << "\n";
        StringUtil::appendFormat(msgBuilder, "Hash: 0x%016llx\n", (unsigned long long)hash.hash);
        msgBuilder << "--- Begin decoded content ---\n";
        msgBuilder << decoded;
        msgBuilder << "--- End decoded content ---\n";
        getTestReporter()->message(TestMessageType::Info, msgBuilder.toString().getBuffer());
        
        // Verify the hash matches the expected value (if provided)
        if (expectedHash != 0 && hash.hash != expectedHash)
        {
            StringBuilder errBuilder;
            StringUtil::appendFormat(errBuilder, 
                "Hash mismatch for '%s': expected 0x%016llx, got 0x%016llx\n",
                testName, (unsigned long long)expectedHash, (unsigned long long)hash.hash);
            getTestReporter()->message(TestMessageType::TestFailure, errBuilder.toString().getBuffer());
            cleanupRecordFiles(recordPath);
            return SLANG_FAIL;
        }
    }
    catch (const Exception& e)
    {
        StringBuilder msgBuilder;
        msgBuilder << "Failed to decode stream.bin for '" << testName << "': " 
                   << e.Message << "\n";
        getTestReporter()->message(TestMessageType::TestFailure, msgBuilder.toString().getBuffer());
        cleanupRecordFiles(recordPath);
        return SLANG_FAIL;
    }

    // Execute the replay to verify the recording works
    SlangResult replayResult = executeReplay(testName, recordPath);
    if (SLANG_FAILED(replayResult))
    {
        // Don't cleanup on failure so we can debug
        return replayResult;
    }

    // Cleanup (disable for now for debugging)
    // cleanupRecordFiles(recordPath);
    return SLANG_OK;
}

// =============================================================================
// Test cases
// =============================================================================

// These examples depend on Vulkan, so we only run them on non-Apple platforms.
// In the future, we may be able to modify the examples to remove render API
// dependencies so they can run on Apple platforms.
#if !(SLANG_APPLE_FAMILY)

// Expected hashes for deterministic verification.
// These hashes include machine-specific paths, so they may need to be updated
// when running on a different machine or after significant code changes.

SLANG_UNIT_TEST(replayRecord_cpu_hello_world)
{
    SLANG_CHECK(SLANG_SUCCEEDED(runTest(unitTestContext, "cpu-hello-world", 0xd1479b2fc60f96f4)));
}

SLANG_UNIT_TEST(replayRecord_triangle)
{
    SLANG_CHECK(SLANG_SUCCEEDED(runTest(unitTestContext, "triangle", 0xbcab34e287651166)));
}

SLANG_UNIT_TEST(replayRecord_ray_tracing)
{
    SLANG_CHECK(SLANG_SUCCEEDED(runTest(unitTestContext, "ray-tracing", 0xce3657d0e05184d4)));
}

SLANG_UNIT_TEST(replayRecord_ray_tracing_pipeline)
{
    SLANG_CHECK(SLANG_SUCCEEDED(runTest(unitTestContext, "ray-tracing-pipeline", 0xb4534afbdd416912)));
}

SLANG_UNIT_TEST(replayRecord_autodiff_texture)
{
    SLANG_CHECK(SLANG_SUCCEEDED(runTest(unitTestContext, "autodiff-texture", 0x1af9d984cc7e9b7a)));
}

SLANG_UNIT_TEST(replayRecord_gpu_printing)
{
    SLANG_CHECK(SLANG_SUCCEEDED(runTest(unitTestContext, "gpu-printing", 0x1894fc7b1f9fd7fa)));
}

SLANG_UNIT_TEST(replayRecord_shader_object)
{
    SLANG_CHECK(SLANG_SUCCEEDED(runTest(unitTestContext, "shader-object", 0x9e6a1f8c528c6f0f)));
}

SLANG_UNIT_TEST(replayRecord_model_viewer)
{
    SLANG_CHECK(SLANG_SUCCEEDED(runTest(unitTestContext, "model-viewer", 0xe382cd77cb38818c)));
}

#endif
