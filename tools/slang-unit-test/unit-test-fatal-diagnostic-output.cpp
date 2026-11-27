// Verify fatal diagnostics are written before exception-disabled builds terminate.
// The parent launches each scenario in a child and checks its captured output. The child
// triggers the fatal path so termination cannot kill the test runner. stderr buffering is
// configured at runner startup, before initialization can write to that stream.

#include "compiler-core/slang-diagnostic-sink.h"
#include "compiler-core/slang-source-loc.h"
#include "core/slang-memory-file-system.h"
#include "core/slang-platform.h"
#include "core/slang-process-util.h"
#include "core/slang-signal.h"
#include "core/slang-writer.h"
#include "scoped-env-var.h"
#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if SLANG_WINDOWS_FAMILY
#include <windows.h>
#else
#include <sys/resource.h>
#endif

using namespace Slang;

#if !SLANG_HAS_EXCEPTIONS
static const char kChildMode[] = "SLANG_FATAL_DIAGNOSTIC_OUTPUT_TEST";
static const char kTestName[] = "slang-unit-test-tool/fatalDiagnosticOutputBeforeCrash";

// Keep text outside stdio until flush() so the writer case verifies the sink flushes the writer,
// rather than passing only because handleSignal flushes stderr itself.
class BufferedFatalDiagnosticWriter : public AppendBufferWriter
{
public:
    BufferedFatalDiagnosticWriter(FILE* stream = stderr)
        : AppendBufferWriter(0), m_stream(stream)
    {
    }

    SLANG_NO_THROW SlangResult SLANG_MCALL write(const char* chars, size_t numChars) SLANG_OVERRIDE
    {
        m_buffer.append(chars, Index(numChars));
        return SLANG_OK;
    }

    SLANG_NO_THROW void SLANG_MCALL flush() SLANG_OVERRIDE
    {
        fwrite(m_buffer.getBuffer(), 1, size_t(m_buffer.getLength()), m_stream);
        fflush(m_stream);
        m_buffer.clear();
    }

private:
    FILE* m_stream;
    StringBuilder m_buffer;
};

// Exercise termination in a child so a fatal signal cannot kill the test runner. stderr is
// deliberately buffered: observing the messages after a crash proves they were flushed first.
static void _runFatalDiagnosticChild(const char* mode)
{
#if SLANG_WINDOWS_FAMILY
    SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);
#else
    const rlimit coreLimit = {0, 0};
    setrlimit(RLIMIT_CORE, &coreLimit);
#endif

    if (strcmp(mode, "import") == 0 || strcmp(mode, "import-name") == 0)
    {
        ComPtr<ISlangMutableFileSystem> fileSystem(new MemoryFileSystem());
        const char* badSource = "public int foo() { return 1 }";
        SLANG_CHECK_ABORT(
            fileSystem->saveFile("bad.slang", badSource, strlen(badSource)) == SLANG_OK);
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        slang::SessionDesc desc = {};
        desc.fileSystem = fileSystem;
        ComPtr<slang::ISession> session;
        SLANG_CHECK_ABORT(globalSession->createSession(desc, session.writeRef()) == SLANG_OK);
        ComPtr<slang::IBlob> diagnostics;
        const char* mainSource = "import bad; void bar() { let a = foo(); let b = foo(); }";
        if (strcmp(mode, "import-name") == 0)
        {
            SLANG_CHECK_ABORT(
                fileSystem->saveFile("main.slang", mainSource, strlen(mainSource)) == SLANG_OK);
            session->loadModule("main", diagnostics.writeRef());
        }
        else
        {
            session->loadModuleFromSourceString(
                "main",
                "main.slang",
                mainSource,
                diagnostics.writeRef());
        }
    }
    else if (strcmp(mode, "codegen") == 0)
    {
        // Undefined opaque handles are diagnosed during target code generation, after module
        // loading and linking succeed. This exercises the code/artifact API's fatal path too.
        const char* source = R"(
            Texture2D<float4> gTexture;
            SamplerState gSampler;
            RWStructuredBuffer<float4> outputBuffer;
            cbuffer Settings { uint index; };
            [shader("compute")][numthreads(1, 1, 1)]
            void main()
            {
                Texture2D<float4> t;
                Texture2D<float4> textures[2] = { gTexture, t };
                outputBuffer[0] = textures[index].SampleLevel(gSampler, float2(0), 0);
            }
        )";
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        slang::TargetDesc target = {};
        target.format = SLANG_SPIRV;
        target.profile = globalSession->findProfile("spirv_1_5");
        slang::SessionDesc desc = {};
        desc.targets = &target;
        desc.targetCount = 1;
        ComPtr<slang::ISession> session;
        SLANG_CHECK_ABORT(globalSession->createSession(desc, session.writeRef()) == SLANG_OK);
        ComPtr<slang::IBlob> diagnostics;
        auto module = session->loadModuleFromSourceString(
            "codegen",
            "codegen.slang",
            source,
            diagnostics.writeRef());
        SLANG_CHECK_ABORT(module != nullptr);
        ComPtr<slang::IEntryPoint> entryPoint;
        SLANG_CHECK_ABORT(module->findEntryPointByName("main", entryPoint.writeRef()) == SLANG_OK);
        ComPtr<slang::IComponentType> program;
        SLANG_CHECK_ABORT(entryPoint->link(program.writeRef(), diagnostics.writeRef()) == SLANG_OK);
        ComPtr<slang::IBlob> code;
        fputs("starting target code generation\n", stdout);
        fflush(stdout);
        program->getEntryPointCode(0, 0, code.writeRef(), diagnostics.writeRef());
    }
    else if (strcmp(mode, "signal") == 0)
    {
        SLANG_ABORT_COMPILATION("direct abort marker");
    }
    else
    {
        // A separate stream proves the child's writer was flushed even when its parent
        // independently reports the same diagnostic to stderr.
        const bool childWriter = strstr(mode, "child-writer") != nullptr;
        BufferedFatalDiagnosticWriter bufferedWriter(childWriter ? stdout : stderr);
        BufferedFatalDiagnosticWriter parentWriter;
        SourceManager sourceManager;
        DiagnosticSink parent(&sourceManager, nullptr);
        DiagnosticSink sink(&sourceManager, nullptr);
        if (strcmp(mode, "parent") == 0 || childWriter)
        {
            sink.setParentSink(&parent);
        }
        else if (strcmp(mode, "writer") == 0)
        {
            sink.writer = &bufferedWriter;
        }
        if (childWriter)
            sink.writer = &bufferedWriter;
        if (strcmp(mode, "child-writer-parent-writer") == 0)
            parent.writer = &parentWriter;
        sink.diagnoseRaw(Severity::Error, "earlier diagnostic marker\n");
        if (strcmp(mode, "raw") == 0 || strcmp(mode, "parent") == 0 ||
            strcmp(mode, "writer") == 0 || strcmp(mode, "startup-warning") == 0 ||
            strcmp(mode, "child-writer-raw") == 0 ||
            strcmp(mode, "child-writer-parent-writer") == 0)
        {
            sink.diagnoseRaw(Severity::Fatal, "fatal diagnostic marker\n");
        }
        else
        {
            DiagnosticInfo info =
                {90000, Severity::Fatal, "fatalOutputTest", "fatal diagnostic marker"};
            if (strcmp(mode, "rich") == 0 || strcmp(mode, "machine") == 0 ||
                strcmp(mode, "child-writer-rich") == 0)
                sink.setFlag(DiagnosticSink::Flag::AlwaysGenerateRichDiagnostics);
            if (strcmp(mode, "machine") == 0)
                sink.setFlag(DiagnosticSink::Flag::MachineReadableDiagnostics);
            sink.diagnose(SourceLoc(), info);
        }
    }
    // This is a failure sentinel: a fatal path must terminate rather than return to its caller.
    fputs("fatal path returned\n", stdout);
    fflush(stdout);
}

#endif

SLANG_UNIT_TEST(fatalDiagnosticOutputBeforeCrash, NoRenderApi)
{
#if SLANG_HAS_EXCEPTIONS
    // Exception-enabled builds keep their existing diagnostic-return and catch behavior.
    SLANG_IGNORE_TEST;
    return;
#else
    StringBuilder mode;
    if (SLANG_SUCCEEDED(
            PlatformUtil::getEnvironmentVariable(UnownedStringSlice(kChildMode), mode)) &&
        mode.getLength() != 0)
    {
        _runFatalDiagnosticChild(mode.getBuffer());
        return;
    }

    const char* modes[] = {
        "raw",
        "legacy",
        "rich",
        "machine",
        "parent",
        "writer",
        "child-writer-raw",
        "child-writer-legacy",
        "child-writer-rich",
        "child-writer-parent-writer",
        "startup-warning",
        "signal",
        "import",
        "import-name",
        "codegen"};
    for (const auto childMode : modes)
    {
        // Unit tests run serially within each test-server process. The child inherits this mode
        // and performs the fatal action instead of recursively launching another test runner.
        SlangUnitTest::ScopedEnvVar childEnvironment(kChildMode, childMode);
        // Force an early stderr warning in one child; all other scenarios use a valid value
        // so machine-readable output can be checked without unrelated startup warnings.
        SlangUnitTest::ScopedEnvVar timeoutEnvironment(
            "SLANG_TEST_RPC_TIMEOUT_MS",
            strcmp(childMode, "startup-warning") == 0 ? "invalid" : "60000");
        CommandLine command;
        command.setExecutableLocation(
            ExecutableLocation(unitTestContext->executableDirectory, "slang-test"));
        command.addArg(kTestName);
        command.addArg("-skip-api-detection");
        ExecuteResult result;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(ProcessUtil::execute(command, result)));
        SLANG_CHECK(result.resultCode != 0);
        SLANG_CHECK(result.standardOutput.indexOf("fatal diagnostic buffering setup failed") == -1);
        SLANG_CHECK(result.standardOutput.indexOf("fatal path returned") == -1);
        if (strcmp(childMode, "import") == 0 || strcmp(childMode, "import-name") == 0)
        {
            SLANG_CHECK(result.standardError.indexOf("bad.slang") != -1);
            SLANG_CHECK(result.standardError.indexOf("compilation ceased") != -1);
            SLANG_CHECK(result.standardError.indexOf("undefined identifier 'foo'") == -1);
        }
        else if (strcmp(childMode, "codegen") == 0)
        {
            SLANG_CHECK(result.standardOutput.indexOf("starting target code generation") != -1);
            SLANG_CHECK(result.standardError.indexOf("use of uninitialized opaque handle") != -1);
        }
        else if (strcmp(childMode, "signal") == 0)
        {
            SLANG_CHECK(result.standardError.indexOf("direct abort marker") != -1);
        }
        else
        {
            SLANG_CHECK(result.standardError.indexOf("earlier diagnostic marker") != -1);
            SLANG_CHECK(result.standardError.indexOf("fatal diagnostic marker") != -1);
            const char* markers[] = {"earlier diagnostic marker", "fatal diagnostic marker"};
            for (const auto marker : markers)
            {
                auto first = result.standardError.indexOf(marker);
                SLANG_CHECK(
                    result.standardError.getUnownedSlice().tail(first + 1).indexOf(
                        UnownedStringSlice(marker)) == -1);
                if (strstr(childMode, "child-writer"))
                {
                    auto writerFirst = result.standardOutput.indexOf(marker);
                    SLANG_CHECK(writerFirst != -1);
                    SLANG_CHECK(
                        result.standardOutput.getUnownedSlice()
                            .tail(writerFirst + 1)
                            .indexOf(UnownedStringSlice(marker)) == -1);
                }
            }
            SLANG_CHECK(result.standardError.indexOf("abort compilation:") == -1);
            if (strcmp(childMode, "machine") == 0)
            {
                SLANG_CHECK(
                    result.standardError ==
                    "earlier diagnostic marker\n"
                    "E90000\tfatal error\t\t0\t0\t0\t0\tfatal diagnostic marker\n");
            }
            if (strcmp(childMode, "startup-warning") == 0)
                SLANG_CHECK(
                    result.standardError.indexOf("ignoring invalid SLANG_TEST_RPC_TIMEOUT_MS") !=
                    -1);
        }
    }
#endif
}
