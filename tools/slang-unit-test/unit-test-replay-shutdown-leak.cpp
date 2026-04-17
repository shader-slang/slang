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

    size_t initialCount = replayCtx.getHandlerCount();
    SLANG_CHECK(initialCount > 0);

    // This is what slang_shutdown() calls to free handler strings
    // before _CrtDumpMemoryLeaks() runs.
    replayCtx.resetHandlers();
    SLANG_CHECK(replayCtx.getHandlerCount() == 0);

    // Restore the handlers so subsequent tests are unaffected.
    replayCtx.registerDefaultHandlers();
    size_t restoredCount = replayCtx.getHandlerCount();
    SLANG_CHECK(restoredCount > 0);
    SLANG_CHECK(restoredCount <= initialCount);
}

// Verify that tryGet() returns the singleton when it has already been
// constructed, so slang_shutdown() can safely skip construction when
// the singleton was never needed (e.g. slang-bootstrap with static linking).
//
// The nullptr path (singleton never constructed) is not reachable from
// slang-test: ReplayContext::get() is a function-local static with no
// reset, and REPLAY_TEST / earlier tests construct it before any test
// body runs.
//
// Regression test for https://github.com/shader-slang/slang/issues/10791
SLANG_UNIT_TEST(replayTryGetSkipsConstruction)
{
    REPLAY_TEST;
    SLANG_UNUSED(unitTestContext);

    // The singleton already exists in this process, so tryGet() must return it.
    auto* ptr = ReplayContext::tryGet();
    SLANG_CHECK(ptr != nullptr);
    SLANG_CHECK(ptr == &ReplayContext::get());
}
