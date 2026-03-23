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

    replayCtx.resetHandlers();
    SLANG_CHECK(replayCtx.getHandlerCount() == 0);
}
