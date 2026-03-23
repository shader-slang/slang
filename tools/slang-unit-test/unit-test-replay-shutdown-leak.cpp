// unit-test-replay-shutdown-leak.cpp
//
// Regression test for https://github.com/shader-slang/slang/issues/10624
//
// The replay system's handlers are registered lazily (on first playback use)
// rather than at static initialization time. This avoids heap allocations
// that would be reported as leaks by _CrtDumpMemoryLeaks in Debug MSVC builds,
// since the singleton is only destroyed during static destruction which runs
// after the CRT leak check in wmain().

#include "unit-test-replay-common.h"

// Verify that handlers are not registered at startup (lazy registration).
// Before the fix, a static HandlerRegistrar would populate the dictionary
// at library load time, causing CRT leak reports.
SLANG_UNIT_TEST(replayHandlersNotRegisteredAtStartup)
{
    REPLAY_TEST;
    SLANG_UNUSED(unitTestContext);

    auto& replayCtx = ctx();

    // With lazy registration, no handlers should be present until
    // a playback operation triggers ensureHandlersRegistered().
    SLANG_CHECK(replayCtx.getHandlerCount() == 0);
}

// Verify that ensureHandlersRegistered() populates the dictionary on demand.
SLANG_UNIT_TEST(replayHandlersRegisteredLazilyOnDemand)
{
    REPLAY_TEST;
    SLANG_UNUSED(unitTestContext);

    auto& replayCtx = ctx();

    SLANG_CHECK(replayCtx.getHandlerCount() == 0);

    replayCtx.ensureHandlersRegistered();

    SLANG_CHECK(replayCtx.getHandlerCount() > 0);
}
