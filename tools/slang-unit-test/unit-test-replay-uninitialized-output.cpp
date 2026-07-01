// unit-test-replay-uninitialized-output.cpp
// Regression test for issue #11865: the record layer must not read uninitialized caller memory
// for output parameters when the wrapped API fails without writing them.

#include "unit-test-replay-common.h"

#include <cstring>

// GlobalSessionProxy::getDownstreamCompilerVersion serializes both int* output slots
// unconditionally (the record stream has a fixed per-call schema). When called with
// SLANG_PASS_THROUGH_NONE the real IGlobalSession returns SLANG_E_NOT_FOUND WITHOUT writing
// *outMajor/*outMinor, so before the fix the proxy read (and serialized) whatever the caller
// happened to leave in that memory.
//
// We verify the fix without a sanitizer by recording the same failing call twice, back to back in
// one Record session, with DIFFERENT caller-side poison values. Each call records a fixed-schema
// segment: signature, 'this' handle, input pass-through, two int outputs, return code. The 'this'
// handle is stable (same proxy) and every field except the two output slots is identical between
// the two calls, so the two segments are byte-identical iff the output slots match. If the proxy
// serialized the caller's poison, the segments differ; with the fix both record a defined 0 and
// the segments are byte-identical.
SLANG_UNIT_TEST(replayGetDownstreamCompilerVersionFailureNoUninitializedRead)
{
    REPLAY_TEST;
    SLANG_UNUSED(unitTestContext);

    // Recording must be active BEFORE the session is created so the returned session is wrapped in
    // a GlobalSessionProxy and registered for handle tracking. (Do not reset() afterwards: that
    // would drop the proxy registration and the next call's 'this' record would fail.)
    ctx().setMode(Mode::Record);

    Slang::ComPtr<slang::IGlobalSession> globalSession;
    SlangGlobalSessionDesc desc = {};
    desc.apiVersion = 0;
    SLANG_CHECK(SLANG_SUCCEEDED(slang_createGlobalSession2(&desc, globalSession.writeRef())));
    SLANG_CHECK(globalSession != nullptr);
    SLANG_CHECK(dynamic_cast<GlobalSessionProxy*>(globalSession.get()) != nullptr);
    // SLANG_CHECK records and continues (it does not abort), so bail out cleanly if session
    // creation failed rather than dereferencing a null session below.
    if (!globalSession)
        return;

    // Two failing calls, back to back, with distinct poison in the (untouched-on-failure) outputs.
    const size_t off0 = ctx().getStream().getSize();

    int majorA = 0x11111111;
    int minorA = 0x22222222;
    SLANG_CHECK(
        globalSession->getDownstreamCompilerVersion(SLANG_PASS_THROUGH_NONE, &majorA, &minorA) ==
        SLANG_E_NOT_FOUND);
    const size_t off1 = ctx().getStream().getSize();

    int majorB = 0x33333333;
    int minorB = 0x44444444;
    SLANG_CHECK(
        globalSession->getDownstreamCompilerVersion(SLANG_PASS_THROUGH_NONE, &majorB, &minorB) ==
        SLANG_E_NOT_FOUND);
    const size_t off2 = ctx().getStream().getSize();

    const size_t len1 = off1 - off0;
    const size_t len2 = off2 - off1;
    SLANG_CHECK(len1 > 0);
    SLANG_CHECK(len1 == len2);

    // With the fix the differing caller-side poison never reaches the stream, so the two recorded
    // call segments are byte-identical. Before the fix the poison was serialized into the output
    // slots and the segments diverged.
    const uint8_t* data = ctx().getStream().getData();
    const bool identical =
        (len1 == len2) && (len1 == 0 || memcmp(data + off0, data + off1, len1) == 0);
    SLANG_CHECK(identical);

    // Return to Idle before the session ComPtr releases during teardown.
    ctx().reset();
}
