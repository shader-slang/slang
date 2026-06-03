// unit-test-depfile.cpp
// Tests for -depfile output, covering both file-output and stdout-output (no -o) cases,
// plus a dedup guard test exercising the writtenStdoutSentinel branch.

#include "core/slang-io.h"
#include "core/slang-process-util.h"
#include "slang/slang-compiler-api.h"
#include "slang/slang-internal.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

/// Returns true if `text` contains the substring `expected`.
static bool _contains(const String& text, const char* expected)
{
    return text.getUnownedSlice().indexOf(UnownedStringSlice(expected)) >= 0;
}

/// RAII wrapper that deletes a temporary file on destruction.
struct TempFile
{
    String path;

    ~TempFile()
    {
        if (path.getLength())
            File::remove(path);
    }
};

/// Creates a temporary file and stores its path in `out`.
static SlangResult _makeTempFile(const char* prefix, TempFile& out)
{
    SLANG_RETURN_ON_FAIL(File::generateTemporary(UnownedStringSlice(prefix), out.path));
    return SLANG_OK;
}

/// Runs slangc with the given arguments and captures stdout/stderr into `outResult`.
static SlangResult _runSlangc(
    UnitTestContext* context,
    const List<String>& args,
    ExecuteResult& outResult)
{
    CommandLine cmdLine;
    cmdLine.setExecutableLocation(ExecutableLocation(context->executableDirectory, "slangc"));
    for (const auto& arg : args)
        cmdLine.addArg(arg);
    return ProcessUtil::execute(cmdLine, outResult);
}

SLANG_UNIT_TEST(DepfileOutput)
{
    // --- Test 1: depfile with -o ---
    {
        TempFile inputBase;
        SLANG_CHECK(SLANG_SUCCEEDED(_makeTempFile("slangc-df-in", inputBase)));
        const String slangPath = inputBase.path + ".slang";
        SLANG_CHECK(SLANG_SUCCEEDED(
            File::writeAllText(slangPath, "[shader(\"compute\")] void main() {}\n")));
        TempFile slangGuard;
        slangGuard.path = slangPath;

        TempFile outputBase;
        SLANG_CHECK(SLANG_SUCCEEDED(_makeTempFile("slangc-df-out", outputBase)));
        const String spvPath = outputBase.path + ".spv";
        TempFile spvGuard;
        spvGuard.path = spvPath;

        TempFile depFile;
        SLANG_CHECK(SLANG_SUCCEEDED(_makeTempFile("slangc-df-dep", depFile)));

        List<String> args;
        args.add("-lang");
        args.add("slang");
        args.add("-target");
        args.add("spirv");
        args.add("-entry");
        args.add("main");
        args.add("-stage");
        args.add("compute");
        args.add("-o");
        args.add(spvPath);
        args.add("-depfile");
        args.add(depFile.path);
        args.add(slangPath);

        ExecuteResult result;
        SLANG_CHECK(SLANG_SUCCEEDED(_runSlangc(unitTestContext, args, result)));
        if (result.resultCode != 0)
            getTestReporter()->message(TestMessageType::Info, result.standardError.getBuffer());
        SLANG_CHECK(result.resultCode == 0);

        String depContent;
        SLANG_CHECK(SLANG_SUCCEEDED(File::readAllText(depFile.path, depContent)));
        getTestReporter()->message(TestMessageType::Info, depContent.getBuffer());

        // Depfile escapes ':' and '\', but bare filenames are unescaped.
        // Extract just the filename component to search for in the depfile content.
        SLANG_CHECK_MSG(
            _contains(depContent, Path::getFileName(spvPath).getBuffer()),
            "depfile missing output path target");
        SLANG_CHECK_MSG(
            _contains(depContent, Path::getFileName(slangPath).getBuffer()),
            "depfile missing input file dependency");
        // Negative: a non-stdout compile must not emit the "-:" sentinel.
        SLANG_CHECK_MSG(
            !depContent.startsWith("-:") && !_contains(depContent, "\n-:"),
            "depfile must not contain '-:' sentinel when -o is specified");
    }

    // --- Test 2: depfile without -o (output to stdout) ---
    {
        TempFile inputBase;
        SLANG_CHECK(SLANG_SUCCEEDED(_makeTempFile("slangc-df-stdout-in", inputBase)));
        const String slangPath = inputBase.path + ".slang";
        SLANG_CHECK(SLANG_SUCCEEDED(
            File::writeAllText(slangPath, "[shader(\"compute\")] void main() {}\n")));
        TempFile slangGuard;
        slangGuard.path = slangPath;

        TempFile depFile;
        SLANG_CHECK(SLANG_SUCCEEDED(_makeTempFile("slangc-df-stdout-dep", depFile)));

        List<String> args;
        args.add("-lang");
        args.add("slang");
        args.add("-target");
        args.add("spirv");
        args.add("-entry");
        args.add("main");
        args.add("-stage");
        args.add("compute");
        // Deliberately no -o — output goes to stdout.
        args.add("-depfile");
        args.add(depFile.path);
        args.add(slangPath);

        ExecuteResult result;
        SLANG_CHECK(SLANG_SUCCEEDED(_runSlangc(unitTestContext, args, result)));
        if (result.resultCode != 0)
            getTestReporter()->message(TestMessageType::Info, result.standardError.getBuffer());
        SLANG_CHECK(result.resultCode == 0);

        String depContent;
        SLANG_CHECK(SLANG_SUCCEEDED(File::readAllText(depFile.path, depContent)));
        getTestReporter()->message(TestMessageType::Info, depContent.getBuffer());

        // Without -o the target must be "-" (stdout sentinel), anchored at line start,
        // followed by a colon, a space, and the dependency — all on the same line.
        SLANG_CHECK_MSG(
            depContent.startsWith("-: "),
            "depfile target line must start with '-: ' (stdout sentinel + space)");
        SLANG_CHECK_MSG(
            !_contains(depContent, "\n-:"),
            "depfile must contain only one '-:' sentinel line");
        SLANG_CHECK_MSG(
            _contains(depContent, Path::getFileName(slangPath).getBuffer()),
            "depfile missing input file dependency");
    }

    // --- Test 3: in-process dedup — writtenStdoutSentinel prevents a second "-:" line ---
    //
    // The writtenStdoutSentinel guard in _writeDependencyStatement is designed to
    // de-duplicate "-:" lines when writeDependencyFile is called with multiple
    // output paths that are all empty (stdout). This scenario arises when a SPIRV
    // entry-point output (empty path, from auto-rawOutput) co-exists with a
    // SlangModule container format whose output path is also empty. The guard must
    // ensure the depfile contains exactly one "-:" sentinel line, not two.
    //
    // This combination cannot be triggered through the slangc CLI (the auto-rawOutput
    // path only fires for single-target invocations, and the SlangModule container
    // output path is always set when specified on the command line), so we exercise
    // it in-process by setting the internal fields directly.
    {
        TempFile inputBase;
        SLANG_CHECK(SLANG_SUCCEEDED(_makeTempFile("slangc-df-dedup-in", inputBase)));
        const String slangPath = inputBase.path + ".slang";
        SLANG_CHECK(SLANG_SUCCEEDED(
            File::writeAllText(slangPath, "[shader(\"compute\")] void main() {}\n")));
        TempFile slangGuard;
        slangGuard.path = slangPath;

        TempFile depFile;
        SLANG_CHECK(SLANG_SUCCEEDED(_makeTempFile("slangc-df-dedup-dep", depFile)));

        {
            SlangCompileRequest* request =
                spCreateCompileRequest((SlangSession*)unitTestContext->slangGlobalSession);
            SLANG_CHECK(request != nullptr);

            spSetCommandLineCompilerMode(request);

            // Directly access the internal request to pre-configure fields that
            // cannot be set via CLI:
            //   m_containerFormat = SlangModule  → triggers a second call to
            //     _writeDependencyStatement with an empty container output path,
            //     exercising the writtenStdoutSentinel dedup guard.
            //   m_dependencyOutputPath            → path for the depfile to write.
            auto* endToEnd = asInternal(request);
            endToEnd->m_containerFormat = ContainerFormat::SlangModule;
            endToEnd->m_dependencyOutputPath = depFile.path;

            const char* slangPathCStr = slangPath.begin();
            const char* parseArgs[] = {
                "-lang",
                "slang",
                "-target",
                "spirv",
                "-entry",
                "main",
                "-stage",
                "compute",
                slangPathCStr,
            };
            // Parse arguments (sets up targets and entry points, but does not compile).
            const SlangResult parseRes =
                spProcessCommandLineArguments(request, parseArgs, SLANG_COUNT_OF(parseArgs));
            if (SLANG_FAILED(parseRes))
                getTestReporter()->message(
                    TestMessageType::Info,
                    spGetDiagnosticOutput(request));
            SLANG_CHECK(SLANG_SUCCEEDED(parseRes));

            // Compile: generates SPIRV code, then writes the depfile via writeDependencyFile.
            // The SPIRV entry-point output path (empty, from auto-rawOutput) triggers the
            // first "-:" sentinel. The SlangModule container path (also empty) triggers the
            // second call, which the writtenStdoutSentinel guard suppresses.
            const SlangResult compileRes = spCompile(request);
            if (SLANG_FAILED(compileRes))
                getTestReporter()->message(
                    TestMessageType::Info,
                    spGetDiagnosticOutput(request));
            SLANG_CHECK(SLANG_SUCCEEDED(compileRes));

            spDestroyCompileRequest(request);
        }

        String depContent;
        SLANG_CHECK(SLANG_SUCCEEDED(File::readAllText(depFile.path, depContent)));
        getTestReporter()->message(TestMessageType::Info, depContent.getBuffer());

        // The dedup guard must have fired: the depfile must start with "-:" (one sentinel
        // was written) but must NOT contain a second "\n-:" line (the guard suppressed it).
        SLANG_CHECK_MSG(
            depContent.startsWith("-:"),
            "depfile must contain '-:' stdout sentinel");
        SLANG_CHECK_MSG(
            !_contains(depContent, "\n-:"),
            "dedup guard failed: depfile must not contain a second '-:' sentinel line");
        SLANG_CHECK_MSG(
            _contains(depContent, Path::getFileName(slangPath).getBuffer()),
            "depfile missing input file dependency");
    }

    // --- Test 4: whole-program output path branch (-emit-spirv-directly) ---
    //
    // When -emit-spirv-directly is active, SPIRV compilation sets isWholeProgram=true
    // and routes through targetInfo->wholeTargetOutputPath in writeDependencyFile
    // (the GenerateWholeProgram branch, lines 105-116), which is not exercised by
    // Tests 1-3 (those all take the per-entry-point path).
    {
        TempFile inputBase;
        SLANG_CHECK(SLANG_SUCCEEDED(_makeTempFile("slangc-df-wp-in", inputBase)));
        const String slangPath = inputBase.path + ".slang";
        SLANG_CHECK(SLANG_SUCCEEDED(
            File::writeAllText(slangPath, "[shader(\"compute\")] void main() {}\n")));
        TempFile slangGuard;
        slangGuard.path = slangPath;

        TempFile outputBase;
        SLANG_CHECK(SLANG_SUCCEEDED(_makeTempFile("slangc-df-wp-out", outputBase)));
        const String spvPath = outputBase.path + ".spv";
        TempFile spvGuard;
        spvGuard.path = spvPath;

        TempFile depFile;
        SLANG_CHECK(SLANG_SUCCEEDED(_makeTempFile("slangc-df-wp-dep", depFile)));

        List<String> args;
        args.add("-lang");
        args.add("slang");
        args.add("-target");
        args.add("spirv");
        args.add("-emit-spirv-directly");
        args.add("-entry");
        args.add("main");
        args.add("-stage");
        args.add("compute");
        args.add("-o");
        args.add(spvPath);
        args.add("-depfile");
        args.add(depFile.path);
        args.add(slangPath);

        ExecuteResult result;
        SLANG_CHECK(SLANG_SUCCEEDED(_runSlangc(unitTestContext, args, result)));
        if (result.resultCode != 0)
            getTestReporter()->message(TestMessageType::Info, result.standardError.getBuffer());
        SLANG_CHECK(result.resultCode == 0);

        String depContent;
        SLANG_CHECK(SLANG_SUCCEEDED(File::readAllText(depFile.path, depContent)));
        getTestReporter()->message(TestMessageType::Info, depContent.getBuffer());

        // Whole-program output: the depfile target is the .spv file, not "-:".
        SLANG_CHECK_MSG(
            _contains(depContent, Path::getFileName(spvPath).getBuffer()),
            "depfile missing whole-program output path target");
        SLANG_CHECK_MSG(
            _contains(depContent, Path::getFileName(slangPath).getBuffer()),
            "depfile missing input file dependency");
        SLANG_CHECK_MSG(
            !depContent.startsWith("-:") && !_contains(depContent, "\n-:"),
            "depfile must not contain '-:' sentinel for whole-program output");
    }

    // --- Test 5: per-entry-point output path branch (-emit-spirv-via-glsl) ---
    //
    // The per-entry-point branch in writeDependencyFile (the else arm that iterates
    // entryPointOutputPaths) is only reached for SPIRV when -emit-spirv-via-glsl is
    // specified; the default SPIRV path (shouldEmitSPIRVDirectly() == true) sets
    // SLANG_TARGET_FLAG_GENERATE_WHOLE_PROGRAM and routes through wholeTargetOutputPath
    // instead.  Tests 1-4 therefore all exercise the whole-program branch; this test
    // covers the per-entry-point branch by forcing via-GLSL lowering.
    {
        TempFile inputBase;
        SLANG_CHECK(SLANG_SUCCEEDED(_makeTempFile("slangc-df-viaglsl-in", inputBase)));
        const String slangPath = inputBase.path + ".slang";
        SLANG_CHECK(SLANG_SUCCEEDED(
            File::writeAllText(slangPath, "[shader(\"compute\")] void main() {}\n")));
        TempFile slangGuard;
        slangGuard.path = slangPath;

        TempFile outputBase;
        SLANG_CHECK(SLANG_SUCCEEDED(_makeTempFile("slangc-df-viaglsl-out", outputBase)));
        const String spvPath = outputBase.path + ".spv";
        TempFile spvGuard;
        spvGuard.path = spvPath;

        TempFile depFile;
        SLANG_CHECK(SLANG_SUCCEEDED(_makeTempFile("slangc-df-viaglsl-dep", depFile)));

        List<String> args;
        args.add("-lang");
        args.add("slang");
        args.add("-target");
        args.add("spirv");
        args.add("-emit-spirv-via-glsl");
        args.add("-entry");
        args.add("main");
        args.add("-stage");
        args.add("compute");
        args.add("-o");
        args.add(spvPath);
        args.add("-depfile");
        args.add(depFile.path);
        args.add(slangPath);

        ExecuteResult result;
        SLANG_CHECK(SLANG_SUCCEEDED(_runSlangc(unitTestContext, args, result)));
        if (result.resultCode != 0)
            getTestReporter()->message(TestMessageType::Info, result.standardError.getBuffer());
        SLANG_CHECK(result.resultCode == 0);

        String depContent;
        SLANG_CHECK(SLANG_SUCCEEDED(File::readAllText(depFile.path, depContent)));
        getTestReporter()->message(TestMessageType::Info, depContent.getBuffer());

        // Per-entry-point output: the depfile target is the .spv file, not "-:".
        SLANG_CHECK_MSG(
            _contains(depContent, Path::getFileName(spvPath).getBuffer()),
            "depfile missing per-entry-point output path target");
        SLANG_CHECK_MSG(
            _contains(depContent, Path::getFileName(slangPath).getBuffer()),
            "depfile missing input file dependency");
        SLANG_CHECK_MSG(
            !depContent.startsWith("-:") && !_contains(depContent, "\n-:"),
            "depfile must not contain '-:' sentinel for named per-entry-point output");
    }

    // --- Test 6: multi-entry-point per-entry-point path, no -o — exactly one "-:" line ---
    //
    // With two entry points, no -o, and -emit-spirv-via-glsl (which routes through the
    // per-entry-point branch of writeDependencyFile), only the last entry point gets an empty
    // path in entryPointOutputPaths (auto-rawOutput targets the last -entry). The loop
    // therefore calls _writeDependencyStatement once with an empty path, producing exactly
    // one "-: <deps>" line. A regression that, for example, reset writtenStdoutSentinel
    // between entry points would still produce only one line here (since there is only one
    // call), but this test pins down the correct single-sentinel shape for the most common
    // multi-entry-point / no-output CLI invocation.
    {
        TempFile inputBase;
        SLANG_CHECK(SLANG_SUCCEEDED(_makeTempFile("slangc-df-multi-ep-in", inputBase)));
        const String slangPath = inputBase.path + ".slang";
        SLANG_CHECK(SLANG_SUCCEEDED(File::writeAllText(
            slangPath,
            "[shader(\"compute\")] void main1() {}\n"
            "[shader(\"compute\")] void main2() {}\n")));
        TempFile slangGuard;
        slangGuard.path = slangPath;

        TempFile depFile;
        SLANG_CHECK(SLANG_SUCCEEDED(_makeTempFile("slangc-df-multi-ep-dep", depFile)));

        List<String> args;
        args.add("-lang");
        args.add("slang");
        args.add("-target");
        args.add("spirv");
        args.add("-emit-spirv-via-glsl");
        args.add("-entry");
        args.add("main1");
        args.add("-stage");
        args.add("compute");
        args.add("-entry");
        args.add("main2");
        args.add("-stage");
        args.add("compute");
        // Deliberately no -o — output goes to stdout.
        args.add("-depfile");
        args.add(depFile.path);
        args.add(slangPath);

        ExecuteResult result;
        SLANG_CHECK(SLANG_SUCCEEDED(_runSlangc(unitTestContext, args, result)));
        if (result.resultCode != 0)
            getTestReporter()->message(TestMessageType::Info, result.standardError.getBuffer());
        SLANG_CHECK(result.resultCode == 0);

        String depContent;
        SLANG_CHECK(SLANG_SUCCEEDED(File::readAllText(depFile.path, depContent)));
        getTestReporter()->message(TestMessageType::Info, depContent.getBuffer());

        // Exactly one "-: <deps>" line — not zero, not two.
        SLANG_CHECK_MSG(
            depContent.startsWith("-: "),
            "depfile must start with '-: ' sentinel for multi-entry stdout output");
        SLANG_CHECK_MSG(
            !_contains(depContent, "\n-:"),
            "depfile must not contain a second '-:' sentinel line");
        SLANG_CHECK_MSG(
            _contains(depContent, Path::getFileName(slangPath).getBuffer()),
            "depfile missing input file dependency");
    }
}
