// unit-test-load-module-dedup.cpp

#include "../../source/core/slang-io.h"
#include "../../source/core/slang-process.h"
#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

// Regression test for shader-slang/slang#11307. Repeated `loadModule` calls
// that resolve to the same file (e.g. `"foo"` vs `"foo.slang"`) used to
// return distinct `IModule` instances because the path-keyed cache hit was
// not also registered under the requested name. The loader now stores the
// cached module under every name form it has been seen with, so subsequent
// calls return the same `IModule*`.
SLANG_UNIT_TEST(loadModuleDedupAcrossNameForms)
{
    const char* fileSource = R"(
        public int unit_test_load_module_dedup_f() { return 1; }
        )";

    auto moduleNameBase = "modDedup" + String(Process::getId());
    String fileName = moduleNameBase + ".slang";
    SLANG_CHECK(SLANG_SUCCEEDED(File::writeAllText(fileName, fileSource)));

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_HLSL;
    targetDesc.profile = globalSession->findProfile("sm_5_0");

    const char* searchPaths[] = {"."};
    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;
    sessionDesc.searchPaths = searchPaths;
    sessionDesc.searchPathCount = 1;

    ComPtr<slang::ISession> session;
    SLANG_CHECK(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    ComPtr<slang::IBlob> diag;
    auto modA = session->loadModule(moduleNameBase.getBuffer(), diag.writeRef());
    SLANG_CHECK_ABORT(modA != nullptr);

    auto modB = session->loadModule(fileName.getBuffer(), diag.writeRef());
    SLANG_CHECK_ABORT(modB != nullptr);

    // Same physical file -> same IModule instance.
    SLANG_CHECK(modA == modB);

    // And a third spelling with a forward-slash prefix should also dedup.
    String prefixedName = String("./") + moduleNameBase;
    auto modC = session->loadModule(prefixedName.getBuffer(), diag.writeRef());
    SLANG_CHECK_ABORT(modC != nullptr);
    SLANG_CHECK(modA == modC);

    File::remove(fileName);
}
