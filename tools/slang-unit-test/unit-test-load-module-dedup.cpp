// unit-test-load-module-dedup.cpp

#include "core/slang-memory-file-system.h"
#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

// Regression test for shader-slang/slang#11307. Repeated `loadModule` calls
// that resolve to the same file used to return distinct `IModule` instances
// because the path-keyed cache hit was not also registered under the
// requested name. The loader now stores the cached module under every name
// form it has been seen with, so subsequent calls return the same `IModule*`.
//
// Uses `MemoryFileSystem` so no on-disk artifacts are created — the test is
// exception-safe (no cleanup leak if a `SLANG_CHECK_ABORT` fires) and works
// uniformly across platforms.
SLANG_UNIT_TEST(loadModuleDedupAcrossNameForms)
{
    const char* utilSource = R"(
        module util;
        public int unit_test_load_module_dedup_f() { return 1; }
        )";
    const char* otherSource = R"(
        module other;
        public int unit_test_load_module_dedup_g() { return 2; }
        )";

    ComPtr<ISlangFileSystemExt> fs = ComPtr<ISlangFileSystemExt>(new MemoryFileSystem());
    auto& memoryFS = *static_cast<MemoryFileSystem*>(fs.get());
    memoryFS.createDirectory("root");
    memoryFS.createDirectory("root/nested");
    memoryFS.saveFile("root/nested/util.slang", utilSource, strlen(utilSource));
    memoryFS.saveFile("root/other.slang", otherSource, strlen(otherSource));

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_HLSL;
    targetDesc.profile = globalSession->findProfile("sm_5_0");

    const char* searchPaths[] = {"root"};
    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;
    sessionDesc.searchPaths = searchPaths;
    sessionDesc.searchPathCount = 1;
    sessionDesc.fileSystem = fs;

    ComPtr<slang::ISession> session;
    SLANG_CHECK_ABORT(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    ComPtr<slang::IBlob> diag;

    // Several spellings from the #11307 reproducer that all resolve to
    // `root/nested/util.slang`. Each should produce the same `IModule*`
    // once the loader registers cache hits under the requested name.
    auto modSlash = session->loadModule("nested/util", diag.writeRef());
    SLANG_CHECK_ABORT(modSlash != nullptr);

    auto modWithExt = session->loadModule("nested/util.slang", diag.writeRef());
    SLANG_CHECK_ABORT(modWithExt != nullptr);
    SLANG_CHECK(modSlash == modWithExt);

    auto modWithDotPrefix = session->loadModule("./nested/util", diag.writeRef());
    SLANG_CHECK_ABORT(modWithDotPrefix != nullptr);
    SLANG_CHECK(modSlash == modWithDotPrefix);

    auto modWithDotPrefixExt = session->loadModule("./nested/util.slang", diag.writeRef());
    SLANG_CHECK_ABORT(modWithDotPrefixExt != nullptr);
    SLANG_CHECK(modSlash == modWithDotPrefixExt);

    // Negative assertion: a *different* source file must still produce a
    // distinct `IModule*`. Guards against a regression that incorrectly
    // returns the first cached module regardless of the requested name.
    auto modOther = session->loadModule("other", diag.writeRef());
    SLANG_CHECK_ABORT(modOther != nullptr);
    SLANG_CHECK(modOther != modSlash);
}
