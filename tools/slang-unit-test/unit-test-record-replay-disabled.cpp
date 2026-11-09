// Referencing every exported record/replay C API symbol makes a dropped or renamed OFF stub fail to
// link rather than silently vanish from the ABI.

#ifndef SLANG_ENABLE_RECORD_REPLAY
#define SLANG_ENABLE_RECORD_REPLAY 1
#endif

#if !SLANG_ENABLE_RECORD_REPLAY

#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <cstring>

SLANG_UNIT_TEST(recordReplayDisabledStubContract)
{
    (void)unitTestContext;

    // Setters, marker, and enable must resolve (stay exported) and be inert.
    slang_enableRecordLayer(true);
    slang_enableRecordLayer(false);
    slang_setReplayDirectory("ignored");
    slang_replayMarker("ignored");

    SLANG_CHECK(slang_isRecordLayerEnabled() == false);

    const char* dir = slang_getReplayDirectory();
    SLANG_CHECK(dir != nullptr);
    SLANG_CHECK(dir != nullptr && strcmp(dir, ".slang-replays") == 0);

    SLANG_CHECK(slang_getCurrentReplayPath() == nullptr);

    SLANG_CHECK(slang_loadReplay("ignored") == SLANG_E_NOT_AVAILABLE);
    SLANG_CHECK(slang_loadLatestReplay() == SLANG_E_NOT_AVAILABLE);
}

#endif // !SLANG_ENABLE_RECORD_REPLAY
