// unit-test-load-module-name-collision.cpp

// Regression test for #10957: `ISession::loadModuleFromSource` used to silently
// return a previously cached module when called a second time with the same
// module name but different source contents.  That caused downstream code to
// operate on the wrong module and sometimes crash.
//
// Expected behaviour after the fix:
//   - Same name, same source        -> returns the cached module (no-op).
//   - Same name, different source   -> returns nullptr and produces a
//                                      diagnostic complaining about the
//                                      collision.

#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <cstring>

using namespace Slang;

SLANG_UNIT_TEST(loadModuleFromSourceNameCollision)
{
    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    slang::TargetDesc targetDesc{};
    targetDesc.format = SLANG_SPIRV;
    targetDesc.profile = globalSession->findProfile("spirv_1_5");

    slang::SessionDesc sessionDesc{};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;

    ComPtr<slang::ISession> session;
    SLANG_CHECK_ABORT(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    const char* sourceA = R"(
        [shader("compute")][numthreads(1,1,1)]
        void a() {}
    )";
    const char* sourceB = R"(
        [shader("compute")][numthreads(1,1,1)]
        void b() {}
    )";

    // First load of "mod" with source A -- should succeed.
    ComPtr<slang::IBlob> diagA1;
    auto modA1 =
        session->loadModuleFromSourceString("mod", "mod.slang", sourceA, diagA1.writeRef());
    SLANG_CHECK(modA1 != nullptr);

    // Reloading "mod" with identical source is still allowed (no-op):
    // should return the cached module and produce no diagnostic.
    ComPtr<slang::IBlob> diagA2;
    auto modA2 =
        session->loadModuleFromSourceString("mod", "mod.slang", sourceA, diagA2.writeRef());
    SLANG_CHECK(modA2 != nullptr);
    SLANG_CHECK(modA2 == modA1);

    // Loading "mod" with a *different* source must now fail and emit a
    // diagnostic pointing at the collision, rather than silently returning
    // the previously cached module.
    ComPtr<slang::IBlob> diagB;
    auto modB = session->loadModuleFromSourceString("mod", "mod.slang", sourceB, diagB.writeRef());
    SLANG_CHECK(modB == nullptr);
    SLANG_CHECK(diagB != nullptr);

    // The diagnostic should be the new E38204 collision error.
    const char* diagText = diagB ? (const char*)diagB->getBufferPointer() : "";
    SLANG_CHECK(strstr(diagText, "38204") != nullptr);
    SLANG_CHECK(strstr(diagText, "mod") != nullptr);
}
