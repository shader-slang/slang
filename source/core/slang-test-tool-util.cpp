
#include "slang-test-tool-util.h"

#include "slang-com-helper.h"
#include "slang-io.h"
#include "slang-string-util.h"

namespace Slang
{

/* static */ ToolReturnCode TestToolUtil::getReturnCode(SlangResult res)
{
    switch (res)
    {
    case SLANG_OK:
        return ToolReturnCode::Success;
    case SLANG_E_INTERNAL_FAIL:
        return ToolReturnCode::CompilationFailed;
    case SLANG_FAIL:
        return ToolReturnCode::Failed;
    case SLANG_E_NOT_AVAILABLE:
        return ToolReturnCode::Ignored;
    default:
        {
            return (SLANG_SUCCEEDED(res)) ? ToolReturnCode::Success : ToolReturnCode::Failed;
        }
    }
}

/* static */ ToolReturnCode TestToolUtil::getReturnCodeFromInt(int code)
{
    if (code >= int(ToolReturnCodeSpan::First) && code <= int(ToolReturnCodeSpan::Last))
    {
        return ToolReturnCode(code);
    }
    else
    {
        SLANG_ASSERT(!"Invalid integral code");
        return ToolReturnCode::Failed;
    }
}

/* static */ bool TestToolUtil::hasDeferredCoreModule(Index argc, const char* const* argv)
{
    for (Index i = 0; i < argc; ++i)
    {
        UnownedStringSlice option(argv[i]);
        if (option == "-load-core-module" || option == "-compile-core-module")
        {
            return true;
        }
    }
    return false;
}

/* static */ SlangResult TestToolUtil::getIncludePath(
    const String& parentPath,
    const char* path,
    String& outIncludePath)
{
    String includePath;
    SLANG_RETURN_ON_FAIL(Path::getCanonical(Path::combine(parentPath, path), includePath));

    // Use forward slashes, to avoid escaping the path
    includePath = StringUtil::calcCharReplaced(includePath, '\\', '/');

    // It must exist!
    if (!File::exists(includePath))
    {
        return SLANG_FAIL;
    }

    outIncludePath = includePath;
    return SLANG_OK;
}

static SlangResult _addCPPPrelude(const String& rootPath, slang::IGlobalSession* session)
{
    String includePath;
    SlangResult res = SLANG_FAIL;
    if (SLANG_FAILED(res))
        res = TestToolUtil::getIncludePath(
            Path::combine(rootPath, "include"),
            "slang-cpp-prelude.h",
            includePath);
    if (SLANG_FAILED(res))
        res = TestToolUtil::getIncludePath(rootPath, "prelude/slang-cpp-prelude.h", includePath);
    SLANG_RETURN_ON_FAIL(res);
    StringBuilder prelude;
    prelude << "#include \"" << includePath << "\"\n\n";
    session->setLanguagePrelude(SLANG_SOURCE_LANGUAGE_CPP, prelude.getBuffer());
    return SLANG_OK;
}

static SlangResult _addCUDAPrelude(const String& rootPath, slang::IGlobalSession* session)
{
    String includePath;
    SlangResult res = SLANG_FAIL;
    if (SLANG_FAILED(res))
        res = TestToolUtil::getIncludePath(
            Path::combine(rootPath, "include"),
            "slang-cuda-prelude.h",
            includePath);
    if (SLANG_FAILED(res))
        res = TestToolUtil::getIncludePath(rootPath, "prelude/slang-cuda-prelude.h", includePath);
    SLANG_RETURN_ON_FAIL(res);
    StringBuilder prelude;
    prelude << "#include \"" << includePath << "\"\n\n";
    session->setLanguagePrelude(SLANG_SOURCE_LANGUAGE_CUDA, prelude.getBuffer());
    return SLANG_OK;
}

// Gets the canonical path for exePath, falling back to the operating system's executable path
// when exePath is just a bare command name. This happens when invoked via PATH on Linux, and also
// when a Windows .exe is launched directly through WSL interop.
static SlangResult _getCanonicalOrExecutablePath(const char* exePath, String& outPath)
{
    if (exePath && Path::hasPath(UnownedStringSlice(exePath)) &&
        SLANG_SUCCEEDED(Path::getCanonical(exePath, outPath)) && File::exists(outPath))
    {
        // argv[0] already contains enough path information to resolve directly.
        // Examples:
        // - "./build/Debug/bin/slang-test" on Linux.
        // - ".\\build\\Debug\\bin\\slang-test.exe" from cmd.exe.
        // - "D:\\repo\\build\\Debug\\bin\\slang-test.exe".
        return SLANG_OK;
    }

    // argv[0] is missing, only a file name, or could not be canonicalized. Ask the OS for the
    // actual executable path instead.
    // Examples:
    // - "slang-test" when invoked via PATH on Linux.
    // - "slang-test.exe" when WSL interop strips path information from argv[0]
    //   (e.g., the user invoked it from WSL with the bin dir on PATH).
    // - A stale symlink or otherwise non-resolvable path-like argv[0].
    outPath = Path::getExecutablePath();
    if (outPath.getLength() == 0)
    {
        return SLANG_FAIL;
    }
    return SLANG_OK;
}

/* static */ SlangResult TestToolUtil::getExeDirectoryPath(
    const char* exePath,
    String& outExeDirectoryPath)
{
    String canonicalPath;
    SLANG_RETURN_ON_FAIL(_getCanonicalOrExecutablePath(exePath, canonicalPath));
    outExeDirectoryPath = Path::getParentDirectory(canonicalPath);
    return SLANG_OK;
}

/* static */ SlangResult TestToolUtil::getDllDirectoryPath(
    const char* exePath,
    String& outDllDirectoryPath)
{
    String canonicalPath;
    SLANG_RETURN_ON_FAIL(_getCanonicalOrExecutablePath(exePath, canonicalPath));
    String binPath = Path::getParentDirectory(canonicalPath);

    // Windows puts the dlls in the same directory as the exe, while on other platforms they are in
    // a 'lib' directory
#ifdef _WIN32
    outDllDirectoryPath = binPath;
#else
    String binaryRootPath = Path::getParentDirectory(binPath);
    outDllDirectoryPath = Path::combine(binaryRootPath, "lib");
#endif
    return SLANG_OK;
}

/* static */ SlangResult TestToolUtil::getRootPath(const char* inExePath, String& outExePath)
{
    // Get the directory holding the exe
    String parentPath;
    SLANG_RETURN_ON_FAIL(getExeDirectoryPath(inExePath, parentPath));

    // Work out the relative path to the root, we will search upwards until we
    // find a directory containing 'prelude/slang-cpp-prelude.h'
    String rootRelPath;
    SLANG_RETURN_ON_FAIL(Path::getCanonical(parentPath, rootRelPath));
    do
    {
        if (File::exists(Path::combine(rootRelPath, "include/slang-cpp-prelude.h")))
            break;
        if (File::exists(Path::combine(rootRelPath, "prelude/slang-cpp-prelude.h")))
            break;

        rootRelPath = Path::getParentDirectory(rootRelPath);
        if (rootRelPath == "")
            return SLANG_E_NOT_AVAILABLE;
    } while (1);

    outExePath = std::move(rootRelPath);
    return SLANG_OK;
}

/* static */ SlangResult TestToolUtil::setSessionDefaultPreludeFromExePath(
    const char* inExePath,
    slang::IGlobalSession* session)
{
    String rootPath;
    SLANG_RETURN_ON_FAIL(getRootPath(inExePath, rootPath));
    SLANG_RETURN_ON_FAIL(setSessionDefaultPreludeFromRootPath(rootPath, session));
    return SLANG_OK;
}

/* static */ SlangResult TestToolUtil::setSessionDefaultPreludeFromRootPath(
    const String& rootPath,
    slang::IGlobalSession* session)
{
    // Set the prelude to a path

    if (SLANG_FAILED(_addCPPPrelude(rootPath, session)))
    {
        SLANG_ASSERT(!"Couldn't find the C++ prelude relative to the executable");
    }

    if (SLANG_FAILED(_addCUDAPrelude(rootPath, session)))
    {
        SLANG_ASSERT(!"Couldn't find the CUDA prelude relative to the executable");
    }

    return SLANG_OK;
}

/* static */ int TestToolUtil::getSubtestIndex(const String& prefix, const String& filePath)
{
    if (prefix.getLength() <= filePath.getLength() || !prefix.startsWith(filePath))
        return -1;

    auto suffix = prefix.getUnownedSlice().tail(filePath.getLength());
    if (suffix.getLength() < 2 || suffix[0] != '.')
        return -1;

    // Reject implausibly long digit runs (suffix is '.' + digits): >9 digits could
    // overflow the 32-bit accumulator below (signed overflow is UB). No real subtest
    // index needs this many digits; -1 is the existing "not a subtest" sentinel.
    if (suffix.getLength() > 10)
        return -1;

    // Check all remaining chars are digits
    int index = 0;
    for (Index i = 1; i < suffix.getLength(); i++)
    {
        char c = suffix[i];
        if (c < '0' || c > '9')
            return -1;
        index = index * 10 + (c - '0');
    }

    return index;
}

/* static */ String TestToolUtil::insertSubtestIndex(const String& testName, int index)
{
    Index spacePos = testName.indexOf(' ');
    StringBuilder result;
    if (spacePos >= 0)
    {
        result << testName.subString(0, spacePos) << "." << index
               << testName.subString(spacePos, testName.getLength() - spacePos);
    }
    else
    {
        result << testName << "." << index;
    }
    return result.produceString();
}

/* static */ bool TestToolUtil::doesSubtestMatchExcludeEntry(
    const String& entry,
    const String& filePath,
    const String& outputStem,
    const String& testName,
    Index subTestIndex,
    Index subtestCount)
{
    // Full expanded display name -> skip just this api/synthesized variant.
    if (testName == entry)
        return true;
    // The first subtest of a multi-subtest file prints with an explicit ".0" under
    // -dry-run, while its internal name omits it; accept the printed form too.
    if (subTestIndex == 0 && subtestCount > 1 && insertSubtestIndex(testName, 0) == entry)
        return true;
    // Subtest stem -> skip all variants of that subtest index. Exact compare (not
    // startsWith) so ".6" does not also match ".60". ".0" names the first subtest,
    // whose stem carries no ".0" suffix.
    const int entrySubtest = getSubtestIndex(entry, filePath);
    if (entrySubtest == 0 && outputStem == filePath)
        return true;
    if (entrySubtest > 0 && outputStem == entry)
        return true;
    return false;
}

} // namespace Slang
