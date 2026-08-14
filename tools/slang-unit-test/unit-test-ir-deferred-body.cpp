#include "core/slang-platform.h"
#include "slang-com-ptr.h"
#include "slang/slang-compiler-api.h"
#include "slang/slang-serialize-ir.h"
#include "unit-test/slang-unit-test.h"

#include <atomic>
#include <thread>

using namespace Slang;


// Checks that a decoration's own children survive on-demand loading.
//
// This guards the `inDecorationSubtree` rule in the load-time scan. Decorations are kept
// eager because the symbol index reads them without materializing anything, and a
// decoration that is itself a parent means keeping the decoration is not enough: its
// children are reachable only through it, so nothing on that path would ever trigger the
// materialization that would supply them. Keeping only the decoration inst gives back a
// decoration that silently has no children.
//
// The module under test is built directly rather than compiled from source, and the
// building happens inside slang because that is where the IR builders live. The shape does
// not occur in any module the compiler produces -- a scan over every serialized decoration
// in the builtin modules finds zero with children, and a precompiled module built from
// autodiff source has none either -- so a test driven by a shader would pass whether this
// rule were implemented or not. The IR permits the shape (decorations can be declared
// `parent = true`), which is what makes the rule worth having and worth testing, and
// building the module directly is the only way to test it that cannot go vacuous.
SLANG_UNIT_TEST(irDeferredBodyKeepsDecorationChildren)
{
    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    Index expectedChildren = 0;
    Index actualChildren = 0;
    bool bodyWasDeferred = false;
    _testRoundTripDecorationWithChildren(
        globalSession,
        expectedChildren,
        actualChildren,
        bodyWasDeferred);

    // Guards the premise: if the decoration ever stops being built with children, the
    // comparison below would hold trivially and this test would check nothing.
    SLANG_CHECK_ABORT(expectedChildren == 2);

    // Likewise: an eager load keeps everything, so it says nothing about the rule.
    if (isOnDemandIRLoadEnabled())
        SLANG_CHECK(bodyWasDeferred);

    // The assertion the rule is about. Under the bug this replaces, the decoration comes
    // back with no children at all: they sit at the same depth as body instructions and
    // were skipped along with them, and materializing the body later does not supply them,
    // because the body's encoding starts after the decorations.
    SLANG_CHECK(actualChildren == expectedChildren);
}

// Checks that concurrent first-touch materialization of a deferred body is safe.
//
// A global session is shared across threads and holds the modules whose bodies are
// deferred, so two compiles can reach the same body at once. That is what the loader's
// mutex and the acquire/release publication of a body exist for: a body is built as a
// detached chain and attached with a single release store, and every list traversal loads
// those links with acquire, so a walker sees either no body or a complete one.
//
// The other tests here are single-threaded, which leaves that protocol unexercised.
//
// Deliberately scoped to materialization rather than to whole compiles. Compiling
// concurrently against one shared global session is documented as unsupported --
// `include/slang.h` states a global session is not thread-safe and that front-end work
// requires external synchronization -- and measurably crashes, with on-demand loading
// either on or off. A test shaped that way would be exercising unsupported usage rather
// than this mechanism, and would fail no matter what this PR did.
//
// The concurrency Slang does support is the serial-frontend/parallel-backend workflow in
// docs/user-guide/08-compiling.md, and that is clean here at 16 threads in both modes.
SLANG_UNIT_TEST(irDeferredBodyConcurrentMaterialization)
{
    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    Index deferredCount = 0;
    Index mismatches = 0;
    _testConcurrentBodyMaterialization(globalSession, deferredCount, mismatches);

    // An eager load races nothing, so it would make the assertion below meaningless.
    if (isOnDemandIRLoadEnabled())
        SLANG_CHECK(deferredCount > 0);

    // Every thread must have seen a complete body every time. A body published before its
    // instructions were fully linked shows up here as a short child list.
    SLANG_CHECK(mismatches == 0);
}

// Checks that the concurrency Slang actually supports is where deferred bodies get
// materialized, and that racing on them yields identical output.
//
// This is the counterpart to the test above: that one drives `ensureBodyMaterialized`
// directly on a synthetic module, which proves the protocol works but not that anything
// real depends on it. This one runs the documented serial-frontend/parallel-backend
// workflow from docs/user-guide/08-compiling.md -- load, specialize and `link()` on one
// thread, then call `getEntryPointCode()` from many -- and asserts that the parallel phase
// is where first touches happen.
//
// That assertion is the one that keeps the loader's mutex honest. If linking ever starts
// materializing everything eagerly, the concurrent first touch stops occurring, and the
// justification for the lock quietly becomes false without any test noticing. Measured
// when written: zero materializations during the front end, and 38 (1 thread) rising to 57
// (16 threads) during the backend, the excess being threads that all observed the deferred
// flag before any had finished.
SLANG_UNIT_TEST(irDeferredBodyMaterializesOnTheSupportedConcurrentPath)
{
    if (!isOnDemandIRLoadEnabled())
        return; // Nothing is deferred, so there is nothing to observe.

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    // ---- serial front end: everything up to and including link() ----
    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_HLSL;
    targetDesc.profile = globalSession->findProfile("sm_5_0");
    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;

    ComPtr<slang::ISession> session;
    SLANG_CHECK_ABORT(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    static const char* kSource = R"(
interface IScale { float apply(float v); }
struct Doubler : IScale { float apply(float v) { return v * 2.0f; } }
float scaleAll<T : IScale>(T s, float v) { return s.apply(v); }
RWStructuredBuffer<float> gOut;
[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    float4x4 m = float4x4(1.0f);
    float3 v = normalize(float3(1.0f, 2.0f, 3.0f));
    Doubler d;
    gOut[tid.x] = scaleAll(d, dot(v, mul(m, float4(v, 1.0f)).xyz)) + sqrt(abs(v.y));
}
)";

    ComPtr<slang::IBlob> diagnostics;
    ComPtr<slang::IModule> module(session->loadModuleFromSourceString(
        "supportedConcurrentPath",
        "supportedConcurrentPath.slang",
        kSource,
        diagnostics.writeRef()));
    SLANG_CHECK_ABORT(module != nullptr);

    ComPtr<slang::IEntryPoint> entryPoint;
    SLANG_CHECK_ABORT(
        module->findEntryPointByName("computeMain", entryPoint.writeRef()) == SLANG_OK);
    slang::IComponentType* components[] = {module, entryPoint};
    ComPtr<slang::IComponentType> composed;
    SLANG_CHECK_ABORT(
        session->createCompositeComponentType(components, 2, composed.writeRef()) == SLANG_OK);
    ComPtr<slang::IComponentType> linked;
    SLANG_CHECK_ABORT(composed->link(linked.writeRef(), diagnostics.writeRef()) == SLANG_OK);

    const Index afterLink = getDeferredBodyMaterializationCount();

    // ---- parallel back end: the one concurrent use the API documents as supported ----
    const int kThreadCount = 8;
    List<String> outputs;
    outputs.setCount(kThreadCount);
    List<uint8_t> succeeded;
    succeeded.setCount(kThreadCount);
    ::memset(succeeded.getBuffer(), 0, size_t(kThreadCount));

    std::atomic<bool> go{false};
    List<std::thread> threads;
    for (int i = 0; i < kThreadCount; i++)
    {
        threads.add(std::thread(
            [&, i]()
            {
                while (!go.load(std::memory_order_acquire))
                    std::this_thread::yield();
                ComPtr<slang::IBlob> code;
                ComPtr<slang::IBlob> diag;
                if (linked->getEntryPointCode(0, 0, code.writeRef(), diag.writeRef()) != SLANG_OK ||
                    !code)
                {
                    return;
                }
                outputs[i] = String((const char*)code->getBufferPointer());
                succeeded[i] = 1;
            }));
    }
    go.store(true, std::memory_order_release);
    for (auto& t : threads)
        t.join();

    const Index duringBackend = getDeferredBodyMaterializationCount() - afterLink;

    for (int i = 0; i < kThreadCount; i++)
    {
        SLANG_CHECK(succeeded[i] != 0);
        SLANG_CHECK(outputs[i] == outputs[0]);
    }
    SLANG_CHECK(outputs[0].getLength() > 0);

    // The point of the test: first touches happen on the concurrent side, so the loader's
    // lock is guarding a path that is really taken.
    SLANG_CHECK(duringBackend > 0);
}

// Checks the two paths that decline deferral, and that declining changes nothing but cost.
//
// Deferral is skipped when the caller supplies no blob, and when the blob it supplies does
// not back the flat table's spans. Neither was exercised by anything that asserted the
// outcome, and the second is the one that matters: it is what stands between a caller
// passing the wrong buffer and a use-after-free surfacing somewhere unrelated. It is also
// invisible from outside, so `getDeferralDeclinedForSpanMismatchCount()` exists to make the
// decision observable — a check that silently stopped rejecting would otherwise look
// exactly like one that had nothing to reject.
SLANG_UNIT_TEST(irDeferralDeclinesWhenTheBlobDoesNotBackTheSpans)
{
    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    struct Case
    {
        TestBlobMode blobMode;
        const char* what;
        bool expectDeferral;
        bool expectMismatchCounted;
    };
    const Case cases[] = {
        {TestBlobMode::Matching, "matching blob", true, false},
        {TestBlobMode::Null, "no blob", false, false},
        {TestBlobMode::Mismatched, "mismatched blob", false, true},
    };

    Index referenceInstCount = 0;
    for (const Case& testCase : cases)
    {
        bool deferred = false;
        Index instCount = 0;
        Index mismatchDelta = 0;
        _testDeferralFallback(globalSession, testCase.blobMode, deferred, instCount, mismatchDelta);

        SLANG_CHECK_ABORT(instCount > 0);
        if (testCase.blobMode == TestBlobMode::Matching)
        {
            referenceInstCount = instCount;
            // Guards the premise: if deferral stopped happening for the matching blob, the
            // other two cases would agree with it trivially and prove nothing.
            if (isOnDemandIRLoadEnabled())
                SLANG_CHECK(deferred == testCase.expectDeferral);
        }
        else
        {
            SLANG_CHECK(!deferred);
            // The whole point of the fallback: declining costs time, never contents.
            SLANG_CHECK(instCount == referenceInstCount);
        }

        // Only the mismatched case should trip the containment check. A null blob is
        // refused earlier, before there is anything to compare.
        if (isOnDemandIRLoadEnabled())
            SLANG_CHECK((mismatchDelta > 0) == testCase.expectMismatchCounted);
    }
}
