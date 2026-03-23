// unit-test-replay-shutdown-leak.cpp
//
// Regression test for https://github.com/shader-slang/slang/issues/10624
//
// The replay system registers handler strings into a Dictionary on the
// ReplayContext singleton during static initialization. slang_shutdown()
// calls resetHandlers() to free these allocations before
// _CrtDumpMemoryLeaks() runs in wmain().

#include "unit-test-replay-common.h"

// Verify that resetHandlers() clears the dictionary, so _CrtDumpMemoryLeaks()
// won't report the handler strings as leaks when called from slang_shutdown().
SLANG_UNIT_TEST(replayResetHandlersClearsDictionary)
{
    REPLAY_TEST;
    SLANG_UNUSED(unitTestContext);

    auto& replayCtx = ctx();

    SLANG_CHECK(replayCtx.getHandlerCount() > 0);

    // Save the real handlers so we can restore them after the test.
    Dictionary<String, ReplayContext::PlaybackHandler> saved;
    replayCtx.swapHandlers(saved);
    SLANG_CHECK(replayCtx.getHandlerCount() == 0);

    // Register a dummy handler so resetHandlers() has something to clear.
    replayCtx.registerHandler("__test__", [](ReplayContext&) {});
    SLANG_CHECK(replayCtx.getHandlerCount() == 1);

    // This is what slang_shutdown() calls to free handler strings
    // before _CrtDumpMemoryLeaks() runs.
    replayCtx.resetHandlers();
    SLANG_CHECK(replayCtx.getHandlerCount() == 0);

    // Restore the original handlers so subsequent tests are unaffected.
    replayCtx.swapHandlers(saved);
    SLANG_CHECK(replayCtx.getHandlerCount() > 0);
}
