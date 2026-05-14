// unit-test-replay-api-surface.cpp
//
// Direct coverage of ReplayContext public APIs not exercised elsewhere.

#include "unit-test-replay-common.h"

// =============================================================================
// Release-to-zero through the proxy (issue #10479)
// =============================================================================

// Driving a proxy's refcount to zero through release() must not crash.
SLANG_UNIT_TEST(replayContextProxyReleaseToZero)
{
    // PROXY_RELEASE_IMPL deliberately reads _ctx (global) and result (stack
    // local) AFTER ProxyBase::releaseImpl() has run, and that call may have
    // done `delete this`. The test cycles refcount up and back a few times
    // (exercising the macro on non-final releases) then drops the final
    // reference. The recorded release-return is appended after releaseImpl()
    // returns, so the stream must keep growing past the deletion. Any future
    // change that introduced a `this->...` access after release would crash
    // here, especially under ASan.
    REPLAY_TEST;

    ctx().setMode(Mode::Record);

    // Hold the raw proxy pointer (not a ComPtr) so this test can drive
    // refcount changes by hand and directly observe the release-to-zero
    // moment. Refcount starts at 1 (slang_createGlobalSession2 returns an
    // addref'd reference).
    SlangGlobalSessionDesc globalDesc = {};
    globalDesc.apiVersion = 0;

    slang::IGlobalSession* proxy = nullptr;
    SLANG_CHECK(SLANG_SUCCEEDED(slang_createGlobalSession2(&globalDesc, &proxy)));
    SLANG_CHECK(proxy != nullptr);

    // Cycle refcount up and back. None of these releases reach zero, so
    // each one runs PROXY_RELEASE_IMPL's "after release" tail with `this`
    // still alive. Catches a regression where the macro's tail accidentally
    // depends on something that's only valid when `this` survives.
    for (int i = 0; i < 4; ++i)
    {
        uint32_t up = proxy->addRef();
        uint32_t down = proxy->release();
        SLANG_CHECK(down == up - 1);
    }

    // Sample the stream size now so we can verify the final release still
    // appended a record after `delete this`.
    size_t streamSizeBeforeFinal = ctx().getStream().getSize();
    SLANG_CHECK(streamSizeBeforeFinal > 0);

    // Last reference: refcount 1 -> 0 runs `delete this` inside releaseImpl().
    // The macro must continue to run after that point, reading only _ctx
    // (global) and result (stack local). A regression that touched
    // `this->...` here would crash, especially under ASan.
    uint32_t finalCount = proxy->release();
    SLANG_CHECK(finalCount == 0);

    // The "release returned" record is appended after releaseImpl() in the
    // macro tail, so the stream must have grown past the deletion.
    SLANG_CHECK(ctx().getStream().getSize() > streamSizeBeforeFinal);
}
