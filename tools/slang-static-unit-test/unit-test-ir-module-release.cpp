// unit-test-ir-module-release.cpp

// slang-compiler-api.h transitively includes slang-ir.h, which declares
// getLiveIRModuleCount.
#include "core/slang-platform.h"
#include "slang/slang-compiler-api.h"
#include "slang/slang-serialize-ir.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

// Asserts that tearing down a session releases the IR modules it loaded.
//
// This exists because the obvious safety nets do not cover the failure it guards
// against. A retain cycle through an `IRModule` -- say the module holding a loader
// that holds a context that holds the module -- keeps every module, its serialized
// blob and its decode state alive forever, and:
//
//   - LeakSanitizer does not report it. The cycle stays reachable from the global
//     session's loaded-module map until the process exits, so there is nothing
//     unreachable to find. It only becomes garbage in a process that destroys a
//     session and keeps running, which no sanitizer job exercises.
//   - Timing and peak-RSS measurements do not show it. Those run a compiler that
//     loads once and exits, which is the one shape in which the leak costs nothing.
//
// What does show it is asking, directly, whether the modules went away. That is a
// yes/no question with no threshold to tune and no dependence on allocator behaviour.
SLANG_UNIT_TEST(irModuleReleasedWithSession)
{
    const Index before = getLiveIRModuleCount();
    const Index deferredLoadersBefore = getDeferredBodyLoaderInstallCount();

    // Asks the loader's own predicate rather than reimplementing it, so this cannot
    // drift into testing the wrong mode.
    const bool onDemandExpected = isOnDemandIRLoadEnabled();

    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

        slang::TargetDesc targetDesc = {};
        targetDesc.format = SLANG_HLSL;
        targetDesc.profile = globalSession->findProfile("sm_5_0");
        slang::SessionDesc sessionDesc = {};
        sessionDesc.targetCount = 1;
        sessionDesc.targets = &targetDesc;

        ComPtr<slang::ISession> session;
        SLANG_CHECK_ABORT(
            globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

        // Compile something, so the builtin modules are loaded and -- under on-demand
        // loading -- so that a deferred-body loader is installed and consulted.
        ComPtr<slang::IBlob> diagnostics;
        ComPtr<slang::IModule> module(session->loadModuleFromSourceString(
            "irModuleReleaseTest",
            "irModuleReleaseTest.slang",
            "float f(float x) { return x * x; }\n"
            "[shader(\"compute\")]\n"
            "[numthreads(1,1,1)]\n"
            "void computeMain(uniform RWStructuredBuffer<float> buf) { buf[0] = f(2.0f); }\n",
            diagnostics.writeRef()));
        SLANG_CHECK_ABORT(module != nullptr);

        SLANG_CHECK(getLiveIRModuleCount() > before);

        // The cycle this guards against can only form on the deferred path, so a run
        // that never took that path proves nothing. Deferral is the default, so assert
        // it happened unless the environment explicitly turned it off -- otherwise the
        // test could stay green while checking nothing.
        if (onDemandExpected)
            SLANG_CHECK(getDeferredBodyLoaderInstallCount() > deferredLoadersBefore);
    }

    // Everything above is out of scope, so every module those sessions created should
    // have been destroyed. A non-zero difference means something still holds one.
    SLANG_CHECK(getLiveIRModuleCount() == before);
}
