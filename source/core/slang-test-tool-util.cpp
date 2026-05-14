
#include "slang-test-tool-util.h"

#include "slang-com-helper.h"
#include "slang-io.h"
#include "slang-string-util.h"

#if SLANG_IGNORE_ABORT_MSG && defined(_MSC_VER)
#include <crtdbg.h>
#include <stdlib.h>
#include <windows.h>
#endif

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

/* static */ void TestToolUtil::disableAssertMessageBoxes()
{
#if SLANG_IGNORE_ABORT_MSG && defined(_MSC_VER)
    SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX | SEM_NOOPENFILEERRORBOX);

    // Prevent abort() and invalid-parameter handling from raising Windows Error Reporting UI.
    _set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);

    // Debug CRT asserts normally open an Abort/Retry/Ignore dialog even when abort() itself is
    // silenced. Route CRT warnings/errors/asserts to stderr so third-party debug builds, such as
    // glslang, fail the command without blocking unattended test runs.
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);
    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDERR);
    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);
#endif
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

// Gets the canonical path for exePath, falling back to /proc/self/exe on Linux
// when exePath is just a bare command name (e.g., when invoked via PATH).
static SlangResult _getCanonicalOrExecutablePath(const char* exePath, String& outPath)
{
    if (SLANG_SUCCEEDED(Path::getCanonical(exePath, outPath)))
    {
        return SLANG_OK;
    }
    // On Linux, when invoked via PATH, argv[0] is just the bare command name
    // and realpath() fails. Fall back to /proc/self/exe.
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
