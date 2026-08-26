// unit-test-depfile.cpp
// Tests for -depfile output.

#include "core/slang-io.h"
#include "core/slang-process-util.h"
#include "core/slang-string-util.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

/// Returns true if `text` contains the substring `expected`.
static bool _contains(const String& text, const char* expected)
{
    return text.getUnownedSlice().indexOf(UnownedStringSlice(expected)) >= 0;
}

/// Returns true if any space-delimited token on the first line of `depContent` (the single
/// `<output>: <dep> <dep...>` statement these tests produce) has `fileName` as its final path
/// component. Splitting on space (not all whitespace) and taking only the first line pins the
/// dependency to the same line as the target — so a regression that orphans a dependency onto its
/// own line, or drops the terminating newline, is caught rather than passing. It also distinguishes
/// `a.slang` from `a.slang-module` (the former is a substring of the latter).
static bool _listsDependencyFile(const String& depContent, const char* fileName)
{
    UnownedStringSlice firstLine = depContent.getUnownedSlice();
    Index newlinePos = firstLine.indexOf('\n');
    if (newlinePos < 0)
        return false; // A well-formed statement is newline-terminated; its absence is a failure.
    firstLine = firstLine.head(newlinePos);

    List<UnownedStringSlice> tokens;
    StringUtil::split(firstLine, ' ', tokens);
    UnownedStringSlice name(fileName);
    for (auto token : tokens)
    {
        if (Path::getFileName(String(token)).getUnownedSlice() == name)
            return true;
    }
    return false;
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

/// RAII wrapper that recursively deletes a temporary directory on destruction.
struct TempDir
{
    String path;

    ~TempDir()
    {
        if (path.getLength())
            Path::removeNonEmpty(path);
    }
};

/// Creates a temporary file and stores its path in `out`.
static SlangResult _makeTempFile(const char* prefix, TempFile& out)
{
    SLANG_RETURN_ON_FAIL(File::generateTemporary(UnownedStringSlice(prefix), out.path));
    return SLANG_OK;
}

/// Creates a fresh temporary directory and stores its path in `out`. We need a directory (rather
/// than the random per-file names `File::generateTemporary` produces) so that the module file has
/// a predictable name `a.slang-module` that `import a;` can resolve.
static SlangResult _makeTempDir(const char* prefix, TempDir& out)
{
    String base;
    SLANG_RETURN_ON_FAIL(File::generateTemporary(UnownedStringSlice(prefix), base));
    // `generateTemporary` creates a placeholder file; remove it and use a directory of the same
    // unique name so nothing is left behind on the temp path.
    SLANG_RETURN_ON_FAIL(File::remove(base));
    if (!Path::createDirectoryRecursive(base))
        return SLANG_FAIL;
    out.path = base;
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

        SLANG_CHECK_MSG(
            _contains(depContent, Path::getFileName(spvPath).getBuffer()),
            "depfile missing output path target");
        SLANG_CHECK_MSG(
            _contains(depContent, Path::getFileName(slangPath).getBuffer()),
            "depfile missing input file dependency");
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

        // Without -o the target must be "-" (stdout sentinel).
        SLANG_CHECK_MSG(
            depContent.startsWith("-: "),
            "depfile target line must start with '-: ' (stdout sentinel + space)");
        SLANG_CHECK_MSG(
            _contains(depContent, Path::getFileName(slangPath).getBuffer()),
            "depfile missing input file dependency");
    }

    // --- Test 3: depfile lists an imported pre-compiled `.slang-module` (issue #12663) ---
    //
    // A build system consuming the depfile must recompile the consumer when the compiled
    // module it imports changes, so the `.slang-module` must appear as a dependency.
    {
        TempDir dir;
        SLANG_CHECK(SLANG_SUCCEEDED(_makeTempDir("slangc-df-mod", dir)));

        const String modSrcPath = Path::combine(dir.path, "a.slang");
        const String modBinPath = Path::combine(dir.path, "a.slang-module");
        const String consumerPath = Path::combine(dir.path, "b.slang");
        const String spvPath = Path::combine(dir.path, "b.spv");

        SLANG_CHECK(SLANG_SUCCEEDED(
            File::writeAllText(modSrcPath, "module a;\npublic void func(int x) {}\n")));
        SLANG_CHECK(SLANG_SUCCEEDED(File::writeAllText(
            consumerPath,
            "import a;\n[shader(\"compute\")] void main() { func(1); }\n")));

        {
            List<String> args;
            args.add("-o");
            args.add(modBinPath);
            args.add(modSrcPath);

            ExecuteResult result;
            SLANG_CHECK(SLANG_SUCCEEDED(_runSlangc(unitTestContext, args, result)));
            if (result.resultCode != 0)
                getTestReporter()->message(TestMessageType::Info, result.standardError.getBuffer());
            SLANG_CHECK(result.resultCode == 0);
        }

        // Delete the source so only the pre-compiled module remains, matching a distribution
        // that ships `.slang-module` files without their `.slang` sources.
        SLANG_CHECK(SLANG_SUCCEEDED(File::remove(modSrcPath)));

        TempFile depFile;
        SLANG_CHECK(SLANG_SUCCEEDED(_makeTempFile("slangc-df-mod-dep", depFile)));

        List<String> args;
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
        args.add(consumerPath);

        ExecuteResult result;
        SLANG_CHECK(SLANG_SUCCEEDED(_runSlangc(unitTestContext, args, result)));
        if (result.resultCode != 0)
            getTestReporter()->message(TestMessageType::Info, result.standardError.getBuffer());
        SLANG_CHECK(result.resultCode == 0);

        TempFile spvGuard;
        spvGuard.path = spvPath;

        String depContent;
        SLANG_CHECK(SLANG_SUCCEEDED(File::readAllText(depFile.path, depContent)));
        getTestReporter()->message(TestMessageType::Info, depContent.getBuffer());

        SLANG_CHECK_MSG(
            _listsDependencyFile(depContent, Path::getFileName(modBinPath).getBuffer()),
            "depfile missing imported .slang-module dependency");
    }

    // --- Test 4: with both source and `.slang-module` present, the depfile lists both ---
    //
    // An `import` prefers the pre-compiled `.slang-module` over the `.slang` source, so both are
    // genuine inputs: the source is folded into the file dependencies and the module file is
    // appended as a module dependency.
    {
        TempDir dir;
        SLANG_CHECK(SLANG_SUCCEEDED(_makeTempDir("slangc-df-src", dir)));

        const String modSrcPath = Path::combine(dir.path, "a.slang");
        const String modBinPath = Path::combine(dir.path, "a.slang-module");
        const String consumerPath = Path::combine(dir.path, "b.slang");
        const String spvPath = Path::combine(dir.path, "b.spv");

        SLANG_CHECK(SLANG_SUCCEEDED(
            File::writeAllText(modSrcPath, "module a;\npublic void func(int x) {}\n")));
        SLANG_CHECK(SLANG_SUCCEEDED(File::writeAllText(
            consumerPath,
            "import a;\n[shader(\"compute\")] void main() { func(1); }\n")));

        {
            List<String> args;
            args.add("-o");
            args.add(modBinPath);
            args.add(modSrcPath);

            ExecuteResult result;
            SLANG_CHECK(SLANG_SUCCEEDED(_runSlangc(unitTestContext, args, result)));
            if (result.resultCode != 0)
                getTestReporter()->message(TestMessageType::Info, result.standardError.getBuffer());
            SLANG_CHECK(result.resultCode == 0);
        }

        TempFile depFile;
        SLANG_CHECK(SLANG_SUCCEEDED(_makeTempFile("slangc-df-src-dep", depFile)));

        List<String> args;
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
        args.add(consumerPath);

        ExecuteResult result;
        SLANG_CHECK(SLANG_SUCCEEDED(_runSlangc(unitTestContext, args, result)));
        if (result.resultCode != 0)
            getTestReporter()->message(TestMessageType::Info, result.standardError.getBuffer());
        SLANG_CHECK(result.resultCode == 0);

        TempFile spvGuard;
        spvGuard.path = spvPath;

        String depContent;
        SLANG_CHECK(SLANG_SUCCEEDED(File::readAllText(depFile.path, depContent)));
        getTestReporter()->message(TestMessageType::Info, depContent.getBuffer());

        SLANG_CHECK_MSG(
            _listsDependencyFile(depContent, Path::getFileName(modSrcPath).getBuffer()),
            "depfile missing module source dependency when source is present");
        SLANG_CHECK_MSG(
            _listsDependencyFile(depContent, Path::getFileName(modBinPath).getBuffer()),
            "depfile missing imported .slang-module dependency (import loads it even when source "
            "is present)");
    }

    // --- Test 5: primary source gone but a secondary dependency remains — module still tracked ---
    //
    // The module `a` `#include`s `helper.slang`, so its serialized dependency list is
    // `[a.slang, helper.slang]`. Deleting only the primary `a.slang` (keeping the include) still
    // lists the `.slang-module`: the module is a module dependency regardless of which of its
    // recorded sources happen to resolve, so appending its resolved path covers this case too.
    {
        TempDir dir;
        SLANG_CHECK(SLANG_SUCCEEDED(_makeTempDir("slangc-df-partial", dir)));

        const String modSrcPath = Path::combine(dir.path, "a.slang");
        const String includePath = Path::combine(dir.path, "helper.slang");
        const String modBinPath = Path::combine(dir.path, "a.slang-module");
        const String consumerPath = Path::combine(dir.path, "b.slang");
        const String spvPath = Path::combine(dir.path, "b.spv");

        SLANG_CHECK(
            SLANG_SUCCEEDED(File::writeAllText(includePath, "void helperFunc(int x) {}\n")));
        SLANG_CHECK(SLANG_SUCCEEDED(File::writeAllText(
            modSrcPath,
            "#include \"helper.slang\"\nmodule a;\npublic void func(int x) { helperFunc(x); }\n")));
        SLANG_CHECK(SLANG_SUCCEEDED(File::writeAllText(
            consumerPath,
            "import a;\n[shader(\"compute\")] void main() { func(1); }\n")));

        {
            List<String> args;
            args.add("-o");
            args.add(modBinPath);
            args.add(modSrcPath);

            ExecuteResult result;
            SLANG_CHECK(SLANG_SUCCEEDED(_runSlangc(unitTestContext, args, result)));
            if (result.resultCode != 0)
                getTestReporter()->message(TestMessageType::Info, result.standardError.getBuffer());
            SLANG_CHECK(result.resultCode == 0);
        }

        SLANG_CHECK(SLANG_SUCCEEDED(File::remove(modSrcPath)));

        TempFile depFile;
        SLANG_CHECK(SLANG_SUCCEEDED(_makeTempFile("slangc-df-partial-dep", depFile)));

        List<String> args;
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
        args.add(consumerPath);

        ExecuteResult result;
        SLANG_CHECK(SLANG_SUCCEEDED(_runSlangc(unitTestContext, args, result)));
        if (result.resultCode != 0)
            getTestReporter()->message(TestMessageType::Info, result.standardError.getBuffer());
        SLANG_CHECK(result.resultCode == 0);

        TempFile spvGuard;
        spvGuard.path = spvPath;

        String depContent;
        SLANG_CHECK(SLANG_SUCCEEDED(File::readAllText(depFile.path, depContent)));
        getTestReporter()->message(TestMessageType::Info, depContent.getBuffer());

        SLANG_CHECK_MSG(
            _listsDependencyFile(depContent, Path::getFileName(modBinPath).getBuffer()),
            "depfile must list the .slang-module when the module's own source is gone, even if a "
            "secondary dependency remains");
        // The surviving `#include`d source is what distinguishes this from the source-absent case:
        // it resolves and is listed as a file dependency alongside the module file.
        SLANG_CHECK_MSG(
            _listsDependencyFile(depContent, Path::getFileName(includePath).getBuffer()),
            "depfile missing the surviving secondary #include dependency");
    }

    // --- Test 6: a module imported from source (no `.slang-module` on disk) lists no module file
    // ---
    //
    // The negative counterpart to Tests 3-5: when only `a.slang` exists, `import a;` compiles it
    // from source, so there is no `.slang-module` to append and the depfile lists only sources.
    {
        TempDir dir;
        SLANG_CHECK(SLANG_SUCCEEDED(_makeTempDir("slangc-df-srconly", dir)));

        const String modSrcPath = Path::combine(dir.path, "a.slang");
        const String modBinName = "a.slang-module";
        const String consumerPath = Path::combine(dir.path, "b.slang");
        const String spvPath = Path::combine(dir.path, "b.spv");

        SLANG_CHECK(SLANG_SUCCEEDED(
            File::writeAllText(modSrcPath, "module a;\npublic void func(int x) {}\n")));
        SLANG_CHECK(SLANG_SUCCEEDED(File::writeAllText(
            consumerPath,
            "import a;\n[shader(\"compute\")] void main() { func(1); }\n")));

        TempFile depFile;
        SLANG_CHECK(SLANG_SUCCEEDED(_makeTempFile("slangc-df-srconly-dep", depFile)));

        List<String> args;
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
        args.add(consumerPath);

        ExecuteResult result;
        SLANG_CHECK(SLANG_SUCCEEDED(_runSlangc(unitTestContext, args, result)));
        if (result.resultCode != 0)
            getTestReporter()->message(TestMessageType::Info, result.standardError.getBuffer());
        SLANG_CHECK(result.resultCode == 0);

        TempFile spvGuard;
        spvGuard.path = spvPath;

        String depContent;
        SLANG_CHECK(SLANG_SUCCEEDED(File::readAllText(depFile.path, depContent)));
        getTestReporter()->message(TestMessageType::Info, depContent.getBuffer());

        SLANG_CHECK_MSG(
            _listsDependencyFile(depContent, Path::getFileName(modSrcPath).getBuffer()),
            "depfile missing the module source dependency");
        SLANG_CHECK_MSG(
            !_listsDependencyFile(depContent, modBinName.getBuffer()),
            "depfile must not list a .slang-module that was never produced");
    }
}
