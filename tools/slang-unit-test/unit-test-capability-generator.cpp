// unit-test-capability-generator.cpp

#include "core/slang-io.h"
#include "core/slang-process-util.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

// The generator is a build-time tool emitted into the sibling "generators/<config>/bin" tree, not
// the test binary's own "<config>/bin" directory, so derive its path by walking up from
// executableDirectory (.../build/<config>/bin) and back down into the generators tree.
static String _getGeneratorPath(UnitTestContext* context)
{
    const String binDir = context->executableDirectory;
    const String configDir = Path::getParentDirectory(binDir);
    const String config = Path::getFileName(configDir);
    const String buildDir = Path::getParentDirectory(configDir);

    String path = Path::combine(buildDir, "generators");
    path = Path::combine(path, config);
    path = Path::combine(path, "bin");
    path = Path::combine(
        path,
        String("slang-capability-generator") + String(Process::getExecutableSuffix()));
    return path;
}

static SlangResult _runGenerator(
    const String& generatorPath,
    const String& inputPath,
    const String& targetDir,
    ExecuteResult& outResult)
{
    CommandLine cmdLine;
    cmdLine.setExecutableLocation(
        ExecutableLocation(ExecutableLocation::Type::Path, generatorPath));
    cmdLine.addArg(inputPath);
    cmdLine.addArg("--target-directory");
    cmdLine.addArg(targetDir);
    return ProcessUtil::execute(cmdLine, outResult);
}

static bool _anyOutputWritten(const String& targetDir)
{
    static const char* const kOutputs[] = {
        "slang-generated-capability-defs.h",
        "slang-generated-capability-defs-impl.h",
        "slang-lookup-capability-defs.cpp",
    };
    for (auto name : kOutputs)
    {
        if (File::exists(Path::combine(targetDir, name)))
            return true;
    }
    return false;
}

static bool _contains(const String& text, const char* expected)
{
    return text.getUnownedSlice().indexOf(UnownedStringSlice(expected)) >= 0;
}

struct TempDir
{
    String path;
    ~TempDir()
    {
        if (path.getLength())
            Path::removeNonEmpty(path);
    }
};

SLANG_UNIT_TEST(CapabilityGeneratorFailsOnError)
{
    const String generatorPath = _getGeneratorPath(unitTestContext);
    if (!File::exists(generatorPath))
    {
        // Absent in install-only / cross-compiled layouts where the generator lives at an external
        // SLANG_GENERATORS_PATH.
        SLANG_IGNORE_TEST;
    }

    String tempBase;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(File::generateTemporary(UnownedStringSlice("capgen-test"), tempBase)));
    File::remove(tempBase);

    TempDir tempDir;
    tempDir.path = tempBase + ".d";
    SLANG_CHECK_ABORT(Path::createDirectoryRecursive(tempDir.path));

    // An internal `_foo` atom with no public `foo` pair triggers error 20007
    // (missingExternalInternalAtomPair).
    {
        const String invalidPath = tempBase + "-invalid.capdef";
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            File::writeAllText(invalidPath, "abstract stage;\ndef _foo : stage;\n")));

        ExecuteResult result;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(_runGenerator(generatorPath, invalidPath, tempDir.path, result)));

        if (result.resultCode == 0)
            getTestReporter()->message(TestMessageType::Info, result.standardError.getBuffer());
        SLANG_CHECK_MSG(_contains(result.standardError, "20007"), "expected diagnostic 20007");
        SLANG_CHECK_MSG(result.resultCode != 0, "generator must exit nonzero on a capdef error");
        SLANG_CHECK_MSG(
            !_anyOutputWritten(tempDir.path),
            "generator must not write outputs on error");

        File::remove(invalidPath);
    }

    // Control: a valid capdef still succeeds and writes its outputs, confirming the guard keys on
    // errors only and does not reject clean input.
    {
        const String validPath = tempBase + "-valid.capdef";
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(File::writeAllText(
            validPath,
            "abstract stage;\ndef _bar : stage;\n/// A documented public atom.\nalias bar = "
            "_bar;\n")));

        ExecuteResult result;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(_runGenerator(generatorPath, validPath, tempDir.path, result)));

        if (result.resultCode != 0)
            getTestReporter()->message(TestMessageType::Info, result.standardError.getBuffer());
        SLANG_CHECK_MSG(result.resultCode == 0, "generator must succeed on a valid capdef");
        SLANG_CHECK_MSG(_anyOutputWritten(tempDir.path), "generator must write outputs on success");

        File::remove(validPath);
    }
}
