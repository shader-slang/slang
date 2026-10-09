// unit-test-search-path-list.cpp

#include "core/slang-io.h"
#include "core/slang-memory-file-system.h"
#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

namespace
{

struct TemporaryFile
{
    String path;

    ~TemporaryFile()
    {
        if (path.getLength())
            File::remove(path);
    }
};

} // namespace

SLANG_UNIT_TEST(SearchPathListABI)
{
    TemporaryFile file;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        File::generateTemporary(UnownedStringSlice("slang-search-path-list"), file.path)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        File::writeAllText(file.path, "/project/src\r\n\r\n/sdk with spaces/math/src\n")));

    const char* const* searchPaths = nullptr;
    SlangInt searchPathCount = 0;
    ComPtr<ISlangUnknown> allocation;
    ComPtr<ISlangBlob> diagnostics;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(slang_readSearchPathsFile(
        file.path.getBuffer(),
        nullptr,
        &searchPaths,
        &searchPathCount,
        allocation.writeRef(),
        diagnostics.writeRef())));
    SLANG_CHECK(!diagnostics);
    SLANG_CHECK(searchPathCount == 2);
    SLANG_CHECK(String(searchPaths[0]) == "/project/src");
    SLANG_CHECK(String(searchPaths[1]) == "/sdk with spaces/math/src");

    const char fileSystemContents[] = "relative/to/current-directory\n";
    ComPtr<ISlangFileSystemExt> fileSystem(new MemoryFileSystem);
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        static_cast<MemoryFileSystem*>(fileSystem.get())
            ->saveFile("search-paths.txt", fileSystemContents, sizeof(fileSystemContents) - 1)));
    const char* const* fileSystemSearchPaths = nullptr;
    SlangInt fileSystemSearchPathCount = 0;
    ComPtr<ISlangUnknown> fileSystemAllocation;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(slang_readSearchPathsFile(
        "search-paths.txt",
        fileSystem,
        &fileSystemSearchPaths,
        &fileSystemSearchPathCount,
        fileSystemAllocation.writeRef())));
    SLANG_CHECK(fileSystemSearchPathCount == 1);
    SLANG_CHECK(String(fileSystemSearchPaths[0]) == "relative/to/current-directory");

    slang::SessionDesc sessionDesc;
    sessionDesc.searchPaths = searchPaths;
    sessionDesc.searchPathCount = searchPathCount;

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef())));
    ComPtr<slang::ISession> session;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(globalSession->createSession(sessionDesc, session.writeRef())));

    const char* const* searchPathsWithoutDiagnostics = nullptr;
    SlangInt searchPathCountWithoutDiagnostics = 0;
    ComPtr<ISlangUnknown> allocationWithoutDiagnostics;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(slang_readSearchPathsFile(
        file.path.getBuffer(),
        nullptr,
        &searchPathsWithoutDiagnostics,
        &searchPathCountWithoutDiagnostics,
        allocationWithoutDiagnostics.writeRef())));

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(File::writeAllText(file.path, "")));
    const char* const* emptySearchPaths = nullptr;
    SlangInt emptySearchPathCount = -1;
    ComPtr<ISlangUnknown> emptyAllocation;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(slang_readSearchPathsFile(
        file.path.getBuffer(),
        nullptr,
        &emptySearchPaths,
        &emptySearchPathCount,
        emptyAllocation.writeRef())));
    SLANG_CHECK(emptySearchPathCount == 0);

    sessionDesc.searchPaths = emptySearchPaths;
    sessionDesc.searchPathCount = emptySearchPathCount;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(globalSession->createSession(sessionDesc, session.writeRef())));

    SLANG_CHECK(
        slang_readSearchPathsFile(
            nullptr,
            nullptr,
            &searchPaths,
            &searchPathCount,
            allocation.writeRef(),
            diagnostics.writeRef()) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(
        slang_readSearchPathsFile(
            "",
            nullptr,
            &searchPaths,
            &searchPathCount,
            allocation.writeRef(),
            diagnostics.writeRef()) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(
        slang_readSearchPathsFile(
            file.path.getBuffer(),
            nullptr,
            nullptr,
            &searchPathCount,
            allocation.writeRef(),
            diagnostics.writeRef()) == SLANG_E_INVALID_ARG);

    const char embeddedNull[] = "/first\n/sec\0ond\n";
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(File::writeAllBytes(file.path, embeddedNull, sizeof(embeddedNull) - 1)));
    SLANG_CHECK(SLANG_FAILED(slang_readSearchPathsFile(
        file.path.getBuffer(),
        nullptr,
        &searchPaths,
        &searchPathCount,
        allocation.writeRef(),
        diagnostics.writeRef())));
    SLANG_CHECK(
        String(UnownedStringSlice(
                   static_cast<const char*>(diagnostics->getBufferPointer()),
                   diagnostics->getBufferSize()))
            .indexOf("line 2") >= 0);

    SLANG_CHECK(SLANG_FAILED(slang_readSearchPathsFile(
        "/path/that/does/not/exist/search-paths.txt",
        nullptr,
        &searchPaths,
        &searchPathCount,
        allocation.writeRef(),
        diagnostics.writeRef())));
    SLANG_CHECK(!searchPaths);
    SLANG_CHECK(searchPathCount == 0);
    SLANG_CHECK(!allocation);
    SLANG_CHECK(diagnostics);
}
