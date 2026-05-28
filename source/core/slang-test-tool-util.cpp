
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

} // namespace Slang
