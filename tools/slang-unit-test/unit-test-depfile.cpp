// unit-test-depfile.cpp
// Tests for -depfile output, covering both file-output and stdout-output (no -o) cases.

#include "../../source/core/slang-io.h"
#include "../../source/core/slang-process-util.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

static bool _contains(const String& text, const char* expected)
{
    return text.getUnownedSlice().indexOf(UnownedStringSlice(expected)) >= 0;
}

struct TempFile
{
    String path;

    ~TempFile()
    {
        if (path.getLength())
            File::remove(path);
    }
};

static SlangResult _makeTempFile(const char* prefix, TempFile& out)
{
    SLANG_RETURN_ON_FAIL(File::generateTemporary(UnownedStringSlice(prefix), out.path));
    return SLANG_OK;
}

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

        // Without -o the target must be "-" (stdout sentinel).
        SLANG_CHECK_MSG(_contains(depContent, "-:"), "depfile missing '-:' stdout target");
        SLANG_CHECK_MSG(
            _contains(depContent, Path::getFileName(slangPath).getBuffer()),
            "depfile missing input file dependency");
    }
}
