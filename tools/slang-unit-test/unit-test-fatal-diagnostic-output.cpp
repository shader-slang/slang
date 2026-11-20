#include "compiler-core/slang-diagnostic-sink.h"
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
    BufferedFatalDiagnosticWriter()
        : AppendBufferWriter(0)
    {
    }

    SLANG_NO_THROW SlangResult SLANG_MCALL write(const char* chars, size_t numChars) SLANG_OVERRIDE
    {
        m_buffer.append(chars, Index(numChars));
        return SLANG_OK;
    }

    SLANG_NO_THROW void SLANG_MCALL flush() SLANG_OVERRIDE
    {
        fwrite(m_buffer.getBuffer(), 1, size_t(m_buffer.getLength()), stderr);
        m_buffer.clear();
    }

private:
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
    static char stderrBuffer[8192];
    setvbuf(stderr, stderrBuffer, _IOFBF, sizeof(stderrBuffer));

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
        BufferedFatalDiagnosticWriter bufferedWriter;
        DiagnosticSink parent;
        DiagnosticSink sink;
        if (strcmp(mode, "parent") == 0)
        {
            sink.setParentSink(&parent);
        }
        else if (strcmp(mode, "writer") == 0)
        {
            sink.writer = &bufferedWriter;
        }
        sink.diagnoseRaw(Severity::Error, "earlier diagnostic marker\n");
        if (strcmp(mode, "raw") == 0 || strcmp(mode, "parent") == 0 || strcmp(mode, "writer") == 0)
        {
            sink.diagnoseRaw(Severity::Fatal, "fatal diagnostic marker\n");
        }
        else
        {
            DiagnosticInfo info =
                {90000, Severity::Fatal, "fatalOutputTest", "fatal diagnostic marker"};
            if (strcmp(mode, "rich") == 0)
                sink.setFlag(DiagnosticSink::Flag::AlwaysGenerateRichDiagnostics);
            sink.diagnose(SourceLoc(), info);
        }
    }
    // This is a failure sentinel: a fatal path must terminate rather than return to its caller.
    fputs("fatal path returned\n", stdout);
    fflush(stdout);
}

#endif

SLANG_UNIT_TEST(fatalDiagnosticOutputBeforeCrash)
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

    const char* modes[] =
        {"raw", "legacy", "rich", "parent", "writer", "signal", "import", "import-name", "codegen"};
    for (const auto childMode : modes)
    {
        // Unit tests run serially within each test-server process. The child inherits this mode
        // and performs the fatal action instead of recursively launching another test runner.
        SlangUnitTest::ScopedEnvVar childEnvironment(kChildMode, childMode);
        CommandLine command;
        command.setExecutableLocation(
            ExecutableLocation(unitTestContext->executableDirectory, "slang-test"));
        command.addArg(kTestName);
        command.addArg("-skip-api-detection");
        ExecuteResult result;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(ProcessUtil::execute(command, result)));
        SLANG_CHECK(result.resultCode != 0);
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
            auto first = result.standardError.indexOf("earlier diagnostic marker");
            SLANG_CHECK(
                result.standardError.getUnownedSlice().tail(first + 1).indexOf(
                    UnownedStringSlice("earlier diagnostic marker")) == -1);
        }
    }
#endif
}
