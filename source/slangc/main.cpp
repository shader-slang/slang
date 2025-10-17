// main.cpp

#include "slang.h"

SLANG_API void spSetCommandLineCompilerMode(SlangCompileRequest* request);

#include "../core/slang-io.h"
#include "../core/slang-test-tool-util.h"
#include "../slang/slang-internal.h"

using namespace Slang;

#include <assert.h>

#ifdef _WIN32
#define MAIN slangc_main
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
#define MAIN main
#include <dlfcn.h>
#endif

static void _diagnosticCallback(char const* message, void* /*userData*/)
{
    auto stdError = StdWriters::getError();
    stdError.put(message);
    stdError.flush();
}

// Helper function to extract -bindir argument from command line and remove it from args
static bool _extractAndRemoveBinDir(
    int& argc,
    const char**& argv,
    String& outBinDir,
    List<const char*>& filteredArgs)
{
    bool foundBinDir = false;
    filteredArgs.clear();

    // Always add the first argument (program name)
    if (argc > 0)
    {
        filteredArgs.add(argv[0]);
    }

    for (int i = 1; i < argc; ++i)
    {
        if (UnownedStringSlice(argv[i]) == "-bindir")
        {
            if (i + 1 < argc)
            {
                outBinDir = argv[i + 1];
                foundBinDir = true;
                ++i; // Skip the next argument (the bindir value)
            }
            else
            {
                // -bindir without a value, will be handled as error later
                filteredArgs.add(argv[i]);
            }
        }
        else
        {
            filteredArgs.add(argv[i]);
        }
    }

    argc = (int)filteredArgs.getCount();
    argv = filteredArgs.getBuffer();

    return foundBinDir;
}

// Helper function to get the actual path of the loaded slang library
static SlangResult _getLoadedSlangLibraryPath(String& outPath)
{
#ifdef _WIN32
    // On Windows, get the path of slang.dll
    HMODULE hModule = GetModuleHandleW(L"slang.dll");
    if (!hModule)
    {
        // Try with full name in case it's statically linked or different name
        hModule = GetModuleHandleW(L"slang");
    }
    if (!hModule)
    {
        return SLANG_FAIL;
    }

    wchar_t path[MAX_PATH];
    DWORD len = GetModuleFileNameW(hModule, path, MAX_PATH);
    if (len == 0 || len == MAX_PATH)
    {
        return SLANG_FAIL;
    }

    outPath = String::fromWString(path);
    return SLANG_OK;
#else
    // On Unix-like systems, use dladdr to get the path of libslang.so
    Dl_info info;

    // Use an exported symbol from slang to find the library
    // slang_createGlobalSession is a good candidate
    if (dladdr((void*)slang_createGlobalSession, &info) && info.dli_fname)
    {
        outPath = String(info.dli_fname);
        return SLANG_OK;
    }

    return SLANG_FAIL;
#endif
}

// Helper function to verify that slang library is loaded from the expected directory
static SlangResult _verifySlangLibraryPath(const String& expectedBinDir)
{
    String loadedLibPath;
    SlangResult res = _getLoadedSlangLibraryPath(loadedLibPath);
    if (SLANG_FAILED(res))
    {
        StdWriters::getError().print(
            "error: Failed to determine the path of the loaded slang library\n");
        return SLANG_FAIL;
    }

    // Get the directory containing the loaded library
    String loadedLibDir = Path::getParentDirectory(loadedLibPath);

    // Normalize both paths for comparison
    String expectedCanonicalPath;
    String loadedCanonicalPath;

    if (SLANG_FAILED(Path::getCanonical(expectedBinDir, expectedCanonicalPath)))
    {
        StdWriters::getError().print(
            "error: Invalid -bindir path: '%s'\n",
            expectedBinDir.getBuffer());
        return SLANG_FAIL;
    }

    if (SLANG_FAILED(Path::getCanonical(loadedLibDir, loadedCanonicalPath)))
    {
        StdWriters::getError().print(
            "error: Failed to canonicalize loaded library path: '%s'\n",
            loadedLibDir.getBuffer());
        return SLANG_FAIL;
    }

    // Compare the canonical paths
    if (expectedCanonicalPath != loadedCanonicalPath)
    {
        StdWriters::getError().print(
            "error: slang library loaded from unexpected location\n"
            "  Expected directory: %s\n"
            "  Actual library path: %s\n",
            expectedCanonicalPath.getBuffer(),
            loadedLibPath.getBuffer());
        return SLANG_FAIL;
    }

    return SLANG_OK;
}

static SlangResult _compile(SlangCompileRequest* compileRequest, int argc, const char* const* argv)
{
    spSetDiagnosticCallback(compileRequest, &_diagnosticCallback, nullptr);
    spSetCommandLineCompilerMode(compileRequest);

    SlangResult res = spProcessCommandLineArguments(compileRequest, &argv[1], argc - 1);
    if (SLANG_FAILED(res))
    {
        // TODO: print usage message
        return res;
    }

    res = SLANG_OK;

#ifndef _DEBUG
    try
#endif
    {
        // Run the compiler (this will produce any diagnostics through
        // SLANG_WRITER_TARGET_TYPE_DIAGNOSTIC).
        res = spCompile(compileRequest);
        // If the compilation failed, then get out of here...
        // Turn into an internal Result -> such that return code can be used to vary result to match
        // previous behavior
        res = SLANG_FAILED(res) ? SLANG_E_INTERNAL_FAIL : res;
    }
#ifndef _DEBUG
    catch (const Exception& e)
    {
        StdWriters::getOut().print("internal compiler error: %S\n", e.Message.toWString().begin());
        res = SLANG_FAIL;
    }
#endif

    return res;
}

bool shouldEmbedPrelude(const char* const* argv, int argc)
{
    for (int i = 0; i < argc; i++)
    {
        if (UnownedStringSlice(argv[i]) == "-embed-prelude")
            return true;
    }
    return false;
}

SLANG_TEST_TOOL_API SlangResult innerMain(
    StdWriters* stdWriters,
    slang::IGlobalSession* sharedSession,
    int argc,
    const char* const* argv,
    const char* const* originalArgv = nullptr,
    int originalArgc = 0)
{
    StdWriters::setSingleton(stdWriters);

    // Assume we will used the shared session
    ComPtr<slang::IGlobalSession> session(sharedSession);

    // The sharedSession always has a pre-loaded core module, is sharedSession is not nullptr.
    // This differed test checks if the command line has an option to setup the core module.
    // If so we *don't* use the sharedSession, and create a new session without the core module just
    // for this compilation.
    if (TestToolUtil::hasDeferredCoreModule(Index(argc - 1), argv + 1))
    {
        SLANG_RETURN_ON_FAIL(
            slang_createGlobalSessionWithoutCoreModule(SLANG_API_VERSION, session.writeRef()));
    }
    else if (!session)
    {
        // Just create the global session in the regular way if there isn't one set
        SlangGlobalSessionDesc desc = {};
        desc.enableGLSL = true;
        Slang::GlobalSessionInternalDesc internalDesc = {};
#ifdef SLANG_BOOTSTRAP
        internalDesc.isBootstrap = true;
#endif
        SLANG_RETURN_ON_FAIL(
            slang_createGlobalSessionImpl(&desc, &internalDesc, session.writeRef()));
    }

    // Use originalArgv[0] if available (contains the executable path before filtering)
    const char* exePath = (originalArgc > 0 && originalArgv) ? originalArgv[0] : argv[0];
    if (!shouldEmbedPrelude(argv, argc))
        TestToolUtil::setSessionDefaultPreludeFromExePath(exePath, session);

    SlangCompileRequest* compileRequest = spCreateCompileRequest(session);
    compileRequest->addSearchPath(Path::getParentDirectory(Path::getExecutablePath()).getBuffer());
    SlangResult res = _compile(compileRequest, argc, argv);
    // Now that we are done, clean up after ourselves
    spDestroyCompileRequest(compileRequest);

    return res;
}

int MAIN(int argc, char** argv)
{
    // Initialize StdWriters early so we can print errors
    auto stdWriters = StdWriters::initDefaultSingleton();

    // Check for -bindir argument and verify slang library location if specified
    String binDir;
    List<const char*> filteredArgs;
    int filteredArgc = argc;
    const char** filteredArgv = (const char**)argv;

    if (_extractAndRemoveBinDir(filteredArgc, filteredArgv, binDir, filteredArgs))
    {
        SlangResult verifyRes = _verifySlangLibraryPath(binDir);
        if (SLANG_FAILED(verifyRes))
        {
            slang::shutdown();
            return (int)TestToolUtil::getReturnCode(verifyRes);
        }
    }

    SlangResult res =
        innerMain(stdWriters, nullptr, filteredArgc, filteredArgv, (const char**)argv, argc);
    slang::shutdown();
    return (int)TestToolUtil::getReturnCode(res);
}

#ifdef _WIN32
int wmain(int argc, wchar_t** argv)
{
    int result = 0;

    {
        // Convert the wide-character Unicode arguments to UTF-8,
        // since that is what Slang expects on the API side.

        List<String> args;
        for (int ii = 0; ii < argc; ++ii)
        {
            args.add(String::fromWString(argv[ii]));
        }
        List<char const*> argBuffers;
        for (int ii = 0; ii < argc; ++ii)
        {
            argBuffers.add(args[ii].getBuffer());
        }

        result = MAIN(argc, (char**)&argBuffers[0]);
    }

#ifdef _MSC_VER
    // _CrtXXX functions are functional only for debug build. The spec says,
    // "When _DEBUG isn't defined, calls to _CrtSetReportMode are removed
    // during preprocessing."
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG);
    _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);
    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG);
    _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDERR);
    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG);
    _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);

    int memleakDetected = _CrtDumpMemoryLeaks();
    SLANG_UNUSED(memleakDetected);
    assert(!memleakDetected);
#endif

    return result;
}
#endif
