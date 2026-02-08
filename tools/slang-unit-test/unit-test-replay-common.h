// unit-test-replay-common.h
// Common includes and helpers for replay unit tests

#pragma once

// Include cpp files directly to access internal symbols not exported from slang DLL
#include "../../source/core/slang-file-system.h"
#include "../../source/core/slang-io.h"
#include "../../source/slang-record-replay/proxy/proxy-base.h"
#include "../../source/slang-record-replay/proxy/proxy-global-session.h"
#include "../../source/slang-record-replay/replay-context.h"
#include "unit-test/slang-unit-test.h"

#include <cstring>

using namespace Slang;
using namespace SlangRecord;

inline ReplayContext& ctx()
{
    return ReplayContext::get();
}

class ScopedReplayContext
{
public:
    ScopedReplayContext() { ctx().reset(); }

    ~ScopedReplayContext() { ctx().reset(); }
};

/*
// Replays tests can't run single threaded or they reset their own
// streams so disable for first PR
#define REPLAY_TEST                      \
    if (ReplayContext::get().isActive()) \
    {                                    \
        SLANG_IGNORE_TEST;               \
    }                                    \
    ScopedReplayContext _scopedReplayContext;
*/
#define REPLAY_TEST SLANG_IGNORE_TEST

// =============================================================================
// Helper: Round-trip test template
// Writes a value, creates a reader, reads it back, and compares
// =============================================================================

template<typename T>
static bool roundTripValue(T writeValue, T& readValue)
{
    ctx().reset();
    ctx().setMode(Mode::Record);
    ctx().record(RecordFlag::None, writeValue);

    ctx().switchToPlayback();
    ctx().record(RecordFlag::None, readValue);

    return ctx().getStream().atEnd();
}

template<typename T>
static bool roundTripCheck(T value)
{
    T readValue{};
    roundTripValue(value, readValue);
    return readValue == value;
}
