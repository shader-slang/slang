// unit-test-replay-uninitialized-output.cpp
// Regression tests for issue #11865: the record layer must not read uninitialized caller memory for
// output parameters when the wrapped API fails without writing them.
//
// Several proxy methods share this defect shape and are covered here:
//   - GlobalSessionProxy::getDownstreamCompilerPath      (ISlangBlob** outPath)
//   - SessionProxy::getTypeConformanceWitnessSequentialID (uint32_t* outId)
//
// Each proxy serializes its output slot(s) unconditionally, because the record stream has a fixed
// per-call schema. When the wrapped API fails without writing the output, the proxy previously read
// (and serialized) whatever the caller left in that memory. The fix redirects the output pointer(s)
// to the zero-initialized PREPARE_POINTER_OUTPUT temporaries on the failure path.
//
// We verify the fix without a sanitizer by recording the same failing call TWICE, back to back in
// one Record session, with DIFFERENT caller-side poison values, and byte-comparing the two recorded
// segments. Each call records a fixed-schema segment: signature, 'this' handle, inputs, output
// slot(s), return code. The 'this' handle is stable across the two calls -- the proxy is registered
// exactly once (at creation) and beginCall records its already-assigned handle rather than
// allocating a new one -- and every field except the output slot(s) is identical between the two
// calls, so the two segments are byte-identical iff the output slots match. If the proxy serialized
// the caller's poison the segments differ; with the fix both record a defined 0 and the segments
// are byte-identical.

#include "slang-record-replay/proxy/proxy-session.h"
#include "unit-test-replay-common.h"

// Assert that the two back-to-back recorded call segments [off0,off1) and [off1,off2) are
// byte-identical. Uses SLANG_CHECK_ABORT for the length preconditions so the memcmp only runs on a
// valid recorded state (a degenerate 0-length or mismatched-length segment aborts the test rather
// than short-circuiting the compare to a false pass), and reads the stream data pointer only after
// all writes are done, since the underlying buffer may reallocate as the stream grows.
static bool twoRecordedSegmentsIdentical(size_t off0, size_t off1, size_t off2)
{
    const size_t len1 = off1 - off0;
    const size_t len2 = off2 - off1;
    SLANG_CHECK_ABORT(len1 > 0);
    SLANG_CHECK_ABORT(len1 == len2);
    const uint8_t* data = ctx().getStream().getData();
    return memcmp(data + off0, data + off1, len1) == 0;
}

// GlobalSessionProxy::getDownstreamCompilerPath serializes its ISlangBlob* output slot
// unconditionally. Called with SLANG_PASS_THROUGH_NONE the real IGlobalSession returns
// SLANG_E_NOT_FOUND WITHOUT writing *outPath (see Session::getDownstreamCompilerPath in
// slang-global-session.cpp), so the same uninitialized-read defect applies. Here the output is a
// COM pointer, so a regression would serialize an uninitialized ISlangBlob* rather than a stray
// int; recording twice with distinct poison pointers and byte-comparing confirms the redirect.
SLANG_UNIT_TEST(replayGetDownstreamCompilerPathFailureNoUninitializedRead)
{
    REPLAY_TEST;
    SLANG_UNUSED(unitTestContext);

    // Recording must be active BEFORE the session is created so the returned session is wrapped in
    // a GlobalSessionProxy and registered for handle tracking. We set the mode directly via
    // ctx().setMode(Mode::Record) rather than the public slang_enableRecordLayer(true) used by the
    // sibling wrapping test; both leave the context in Mode::Record, which is what makes
    // slang_createGlobalSession2 wrap the returned session in a proxy. (Do not reset() after
    // creation: that drops the proxy registration and the next call's 'this' record would fail.)
    ctx().setMode(Mode::Record);

    Slang::ComPtr<slang::IGlobalSession> globalSession;
    SlangGlobalSessionDesc desc = {};
    desc.apiVersion = 0;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(slang_createGlobalSession2(&desc, globalSession.writeRef())));
    SLANG_CHECK_ABORT(dynamic_cast<GlobalSessionProxy*>(globalSession.get()) != nullptr);

    // Two failing calls, back to back, with distinct poison in the (untouched-on-failure) outPath.
    // outPath is a raw ISlangBlob* out-param, so we poison it directly rather than via a ComPtr.
    const size_t off0 = ctx().getStream().getSize();
    ISlangBlob* pathA =
        reinterpret_cast<ISlangBlob*>(static_cast<uintptr_t>(0x1111111111111111ull));
    SLANG_CHECK(
        globalSession->getDownstreamCompilerPath(SLANG_PASS_THROUGH_NONE, &pathA) ==
        SLANG_E_NOT_FOUND);
    const size_t off1 = ctx().getStream().getSize();

    ISlangBlob* pathB =
        reinterpret_cast<ISlangBlob*>(static_cast<uintptr_t>(0x2222222222222222ull));
    SLANG_CHECK(
        globalSession->getDownstreamCompilerPath(SLANG_PASS_THROUGH_NONE, &pathB) ==
        SLANG_E_NOT_FOUND);
    const size_t off2 = ctx().getStream().getSize();

    // On failure the proxy redirects outPath to a zero-initialized temporary before recording, so
    // the caller's poison pointer never reaches the stream: both calls serialize the same defined
    // null and the two recorded segments are byte-identical. A proxy that recorded *outPath
    // directly would serialize the two distinct poison pointers and the segments would diverge.
    SLANG_CHECK(twoRecordedSegmentsIdentical(off0, off1, off2));

    // Return to Idle before the session ComPtr releases during teardown.
    ctx().reset();
}

// SessionProxy::getTypeConformanceWitnessSequentialID serializes its uint32_t* output slot
// unconditionally. Called with null type arguments the real ISession returns SLANG_FAIL WITHOUT
// writing *outId (Linkage::getTypeConformanceWitnessSequentialID, slang-session.cpp:758-759), so
// the same uninitialized-read defect applies. The null-argument failure path is reached without a
// compiled module or real TypeReflection objects, so this mirrors the test above for the sibling
// fix.
SLANG_UNIT_TEST(replayGetTypeConformanceWitnessSequentialIDFailureNoUninitializedRead)
{
    REPLAY_TEST;
    SLANG_UNUSED(unitTestContext);

    // Same requirement as above: recording active before session creation so the proxies are
    // wrapped and registered.
    ctx().setMode(Mode::Record);

    Slang::ComPtr<slang::IGlobalSession> globalSession;
    SlangGlobalSessionDesc desc = {};
    desc.apiVersion = 0;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(slang_createGlobalSession2(&desc, globalSession.writeRef())));

    Slang::ComPtr<slang::ISession> session;
    slang::SessionDesc sessionDesc = {};
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(globalSession->createSession(sessionDesc, session.writeRef())));
    SLANG_CHECK_ABORT(dynamic_cast<SessionProxy*>(session.get()) != nullptr);

    // Two failing calls (null type args -> SLANG_FAIL) with distinct poison in the untouched outId.
    const size_t off0 = ctx().getStream().getSize();
    uint32_t idA = 0x11111111;
    SLANG_CHECK(
        session->getTypeConformanceWitnessSequentialID(nullptr, nullptr, &idA) == SLANG_FAIL);
    const size_t off1 = ctx().getStream().getSize();

    uint32_t idB = 0x22222222;
    SLANG_CHECK(
        session->getTypeConformanceWitnessSequentialID(nullptr, nullptr, &idB) == SLANG_FAIL);
    const size_t off2 = ctx().getStream().getSize();

    SLANG_CHECK(twoRecordedSegmentsIdentical(off0, off1, off2));

    // Return to Idle before the session/global-session ComPtrs release during teardown.
    ctx().reset();
}
